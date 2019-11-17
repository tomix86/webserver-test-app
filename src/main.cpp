#include <boost/algorithm/string.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <mutex>
#include <onesdk/onesdk.h>
#include <opencv2/imgcodecs.hpp>
#include <thread>
#include <tuple>

#include "SimpleTimer.hpp"
#include "TerminationHandler.hpp"
#include "ironbow_palette.hpp"
#include "listener.hpp"

using namespace std::chrono_literals;
namespace po = boost::program_options;
using web::http::status_codes, web::http::http_response, web::http::http_request;

onesdk_webapplicationinfo_handle_t web_application_info_handle = ONESDK_INVALID_HANDLE;

void initialize_heat_compute();
void basic_heat_compute();
std::vector<std::vector<float>> cuda_heat_compute(int blockDimX, int blockDimY, int meshSize, int steps);

static auto make_string_t(const std::string& input) {
	return utility::string_t{ input.begin(), input.end() };
}

static auto make_string(const utility::string_t& input) {
	std::string result;

	for(auto c : input) {
		result.push_back(static_cast<char>(c));
	}

	return result;
}

static std::tuple<int, int, int, int> parseParams(const utility::string_t& queryString) {
	std::vector<utility::string_t> params;
	boost::split(params, queryString, boost::is_any_of("&"));
	for (const auto& param : params) {
		ucout << U("Param: ") << param << U("\n");
	}

	if (params.size() != 4) { throw std::invalid_argument{ "Invalid number of request paramers" }; }

	return { std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]) };
}

static auto encodeImage(const std::vector<std::vector<float>>& grid) {
	SimpleTimer t("Image encoding");
	cv::Mat image(static_cast<int>(grid.front().size()), static_cast<int>(grid.size()), CV_8UC3);
	for (int y = 0; y < image.rows; ++y) {
		for (int x = 0; x < image.cols; ++x) {
			image.at<cv::Vec3b>(x, y) = tempToColor(grid[y][x]);
		}
	}

	std::vector<unsigned char> encodedImage;
	if (!cv::imencode(".png", image, encodedImage, { cv::IMWRITE_PNG_COMPRESSION, 9 })) {
		throw std::runtime_error{ "Failed to encode the image" };
	}

	return encodedImage;
}

static auto makeResponse(const std::vector<std::vector<float>>& result) {
	http_response response{ status_codes::OK };
	response.set_body(encodeImage(result));
	response.headers().set_content_type(U("image/png"));
	return response;
}

static void printRequestMetadata(const http_request& req) {
	static int count;
	ucout << U("Request #") << ++count << U(" received: ") << req.request_uri().query() << U("\n");
	ucout << U("Remote address: ") << req.remote_address() << U("\n");
	ucout << U("Headers: ") << U("\n");
	for (const auto& [header, content] : req.headers()) {
		ucout << U("   ") << header << U(" = ") << content << U("\n");
	}
}

static http_response requestHandler(const http_request& req) try {
	static std::mutex mutex;
	std::lock_guard l{ mutex };

	//TODO: Do I need to create/start it each time?
	/* create tracer */
    onesdk_tracer_handle_t const tracer = onesdk_incomingwebrequesttracer_create(
        web_application_info_handle,
        onesdk_asciistr(("/test?" + make_string(req.request_uri().query())).c_str()), //TODO: get root from request data
        onesdk_asciistr("GET")); //TODO: get from request data

    /* add information about the incoming request */
    onesdk_incomingwebrequesttracer_set_remote_address(tracer, onesdk_asciistr(make_string(req.remote_address()).c_str()));
    onesdk_incomingwebrequesttracer_add_request_header(tracer, onesdk_asciistr("Connection"), onesdk_asciistr("keep-alive"));
    onesdk_incomingwebrequesttracer_add_request_header(tracer, onesdk_asciistr("Pragma"), onesdk_asciistr("no-cache"));
    /* ... */

    /* start tracer */
    onesdk_tracer_start(tracer);

	printRequestMetadata(req);

	SimpleTimer t("Request handler");
	const auto [blockX, blockY, meshSize, steps]{ parseParams(req.request_uri().query()) };
	const auto result{ cuda_heat_compute(blockX, blockY, meshSize, steps) };
	if (result.empty()) {
		std::cerr << "Failed to compute result\n";
		return status_codes::InternalError;
	}

	const auto response{makeResponse(result)};

    /* add information about the response */
    //onesdk_incomingwebrequesttracer_add_response_header(tracer, onesdk_asciistr("Transfer-Encoding"), onesdk_asciistr("chunked"));
  	onesdk_incomingwebrequesttracer_add_response_header(tracer, onesdk_asciistr("Content-Length"), onesdk_asciistr(std::to_string(response.headers().content_length()).c_str()));
    onesdk_incomingwebrequesttracer_set_status_code(tracer, 200);

	//TODO: handle onesdk_tracer_error()
	//TODO: return proper response in case of request handling failure

	return response;
} catch (const std::invalid_argument& err) {
	std::cerr << err.what() << '\n';
	return status_codes::BadRequest;
} catch (const std::runtime_error& err) {
	std::cerr << err.what() << '\n';
	return status_codes::InternalError;
}

utility::string_t parseCommandLine(int argc, char* argv[]) {
	std::string port;
	std::string address;

	po::options_description desc("Allowed options");
	desc.add_options()("help", "produce help message")(
		"listen_address", po::value<std::string>(&address)->default_value("*"), "Address on which to listen for incoming connections")(
		"port", po::value<std::string>(&port)->default_value("40000"), "Port on which to listen for incoming connections");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << "\n";
		exit(0);
	}

	return U("http://") + make_string_t(address) + U(":") + make_string_t(port) + U("/");
}

static void mycallback(const char* message) {
    std::cout << message;
}

int main(int argc, char* argv[]) try {
	registerTerminationHandler();

	/* Initialize SDK */
    const auto onesdk_init_result{ onesdk_initialize() };

	/* optional: Set logging callbacks. */
    onesdk_agent_set_warning_callback(mycallback); /* Highly recommended. */
    onesdk_agent_set_verbose_callback(mycallback); /* Recommended for development & debugging. */


    web_application_info_handle = onesdk_webapplicationinfo_create(
        onesdk_asciistr("www.gpudev.icu"),      /* name of the web server that hosts your application */ //TODO: is this a correct name?
        onesdk_asciistr("GPUPluginTestApp"),    /* unique name for your web application               */
        onesdk_asciistr("/")         /* context root of your web application               */ );

	const auto listeningAddress{ parseCommandLine(argc, argv) };

	initialize_heat_compute();
	basic_heat_compute();

	RequestListener listener{ listeningAddress };
	listener.addListener(U("test"), requestHandler);
	if (!listener.start()) { return 1; }

	while (running) {
		std::this_thread::sleep_for(1s);
	}

	listener.stop();

    onesdk_webapplicationinfo_delete(web_application_info_handle);
    web_application_info_handle = ONESDK_INVALID_HANDLE;

	/* Shut down SDK */
    if (onesdk_init_result == ONESDK_SUCCESS) {
        onesdk_shutdown();
	}

	return 0;
} catch (const std::exception& ex) {
	std::cout << "Fatal error: " << ex.what();
	return 1;
}