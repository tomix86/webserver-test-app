#include <boost/algorithm/string.hpp>
#include <opencv2/imgcodecs.hpp>
#include <thread>
#include <tuple>

#include "TerminationHandler.hpp"
#include "ironbow_palette.hpp"
#include "listener.hpp"

using namespace std::chrono_literals;

void initialize_heat_compute();
void basic_heat_compute();
std::vector<std::vector<float>> cuda_heat_compute(int blockDimX, int blockDimY, int meshSize, int steps);

std::tuple<int, int, int, int> parseParams(const utility::string_t& queryString) {
	std::vector<utility::string_t> params;
	boost::split(params, queryString, boost::is_any_of("&"));
	for (const auto& param : params) {
		ucout << U("Param: ") << param << U("\n");
	}

	if (params.size() != 4) { throw std::invalid_argument{ "Invalid number of request paramers" }; }

	return { std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]) };
}

auto encodeImage(const std::vector<std::vector<float>>& grid) {
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

int main(int argc, char* argv[]) {
	registerTerminationHandler();

	initialize_heat_compute();
	basic_heat_compute();

	RequestListener listener{ L"http://127.0.0.1:40000/" };
	listener.addListener(L"test", [](auto req) -> web::http::http_response {
		const auto queryString{ req.request_uri().query() };
		ucout << U("Request received: ") << queryString << std::endl;
		ucout << U("Remote address: ") << req.remote_address() << std::endl;
		ucout << U("Headers: ") << std::endl;
		for (const auto& [header, content] : req.headers()) {
			ucout << U("   ") << header << U(" = ") << content << std::endl;
		}

		try {
			const auto [blockX, blockY, meshSize, steps]{ parseParams(queryString) };
			std::cout << "Computing result...\n";
			const auto result{ cuda_heat_compute(blockX, blockY, meshSize, steps) };
			if (result.empty()) {
				std::cerr << "Failed to compute result\n";
				return web::http::status_codes::InternalError;
			}

			web::http::http_response response{ web::http::status_codes::OK };
			response.set_body(encodeImage(result));
			response.headers().set_content_type(U("image/png"));
			return response;
		} catch (const std::invalid_argument& err) {
			std::cerr << err.what() << '\n';
			return web::http::status_codes::BadRequest;
		} catch (const std::runtime_error& err) {
			std::cerr << err.what() << '\n';
			return web::http::status_codes::InternalError;
		}
	});

	listener.start();

	while (running) {
		std::this_thread::sleep_for(1s);
	}

	listener.stop();

	return 0;
}