#include "TerminationHandler.hpp"
#include "listener.hpp"
#include <boost/algorithm/string.hpp>
#include <opencv2/imgcodecs.hpp>

void initialize_heat_compute();
void basic_heat_compute();
std::vector<std::vector<float>> cuda_heat_compute(int blockDimX, int blockDimY, int meshSize, int steps);

int main(int argc, char* argv[])
{
	registerTerminationHandler();

	initialize_heat_compute();
	basic_heat_compute();

	RequestListener listener{ L"http://127.0.0.1:40000/" };
	listener.addListener(L"test", [](auto req) -> web::http::http_response {
		const auto queryString{ req.request_uri().query() };
		ucout << U("Request received: ") << queryString << std::endl;

		//TODO: Validate query string properly, number of arguments, args type

		std::vector<utility::string_t> params;
		boost::split(params, queryString, boost::is_any_of("&"));
		for (const auto& param : params) {
			ucout << U("Param: ") << param << U("\n");
		}

		if (params.size() != 4) {
			std::cerr << "Invalid number of request paramers\n";
			return web::http::status_codes::BadRequest;
		}

		std::cout << "Computing result...\n";
		const auto result{ cuda_heat_compute(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3])) };
		if (result.empty()) {
			std::cerr << "Failed to compute result\n";
			return web::http::status_codes::InternalError;
		}

		cv::Mat image( static_cast<int>(result.front().size()), static_cast<int>(result.size()), CV_8UC3 );
		for (int y = 0; y < image.rows; ++y) {
			for (int x = 0; x < image.cols; ++x) {
				auto& bgra = image.at<cv::Vec3b>(x, y);
				//bgra[0] = UCHAR_MAX; // Blue
				//bgra[1] = cv::saturate_cast<uchar>((float(image.cols - j)) / ((float)image.cols) * UCHAR_MAX); // Green
				//bgra[2] = cv::saturate_cast<uchar>((float(image.rows - i)) / ((float)image.rows) * UCHAR_MAX); // Red
				bgra[0] = 0; // Blue
				bgra[1] = 0; // Green
				bgra[2] = static_cast<int>(result[y][x] * 4) % UCHAR_MAX; // Red
			}
		}

		std::vector<unsigned char> encodedImage;
		if (!cv::imencode(".png", image, encodedImage, { cv::IMWRITE_PNG_COMPRESSION, 9 })) {
			std::cerr << "Failed to encode the image\n";
			return web::http::status_codes::InternalError;
		}

		web::http::http_response response{ web::http::status_codes::OK };
		response.set_body(encodedImage);
		response.headers().set_content_type(U("image/png"));
		return response;
	});

	listener.start();

	while (running);

	listener.stop();

	return 0;
}