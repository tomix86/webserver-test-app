#include "TerminationHandler.hpp"
#include "kernel.hpp"
#include "listener.hpp"
#include <boost/algorithm/string.hpp>

void initialize_heat_compute();
void basic_heat_compute();
bool cuda_heat_compute(int blockDimX, int blockDimY, int meshSize, int steps);

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

		const auto success{ cuda_heat_compute(std::stoi(params[0]), std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3])) };
		web::http::http_response response{ web::http::status_codes::OK };
		response.set_body(success ? "Computations succeeded" : "Computations failed");
		return response;
	});

	listener.start();

	while (running);

	listener.stop();

	return 0;
}