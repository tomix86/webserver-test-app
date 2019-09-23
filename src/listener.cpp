#include "listener.hpp"
#include <iostream>

using namespace std::string_literals;

RequestListener::RequestListener(utility::string_t baseURI) :
	listenerBaseURI{ baseURI } {
}

auto methodNameToString(web::http::method method) {
	if (method == web::http::methods::GET) {
		return U("GET");
	}
	else if (method == web::http::methods::PUT) {
		return U("PUT");
	}
	else if (method == web::http::methods::DEL) {
		return U("DELETE");
	}
	else {
		return U("Unknown");
	}
}

void RequestListener::addListener(utility::string_t resource, std::function< web::http::http_response(web::http::http_request)> handler) {
	const auto method = web::http::methods::GET;

	listeners.emplace_back(listenerBaseURI + resource).support(method, [handler](web::http::http_request request) {
		auto response = handler(request);
		request.reply(response);
	});

	ucout << U("Configured (method:path): ") << methodNameToString(method) << U(":") << listeners.back().uri().path() << std::endl;
}

void RequestListener::start() {
	ucout << U("Starting listener, base url: ") << listenerBaseURI << std::endl;

	for (auto& listener : listeners) {
		ucout << U("...starting ") << listener.uri().path() << std::endl;

		try {
			listener.open().wait();
		}
		catch (const std::exception& ex) {
			std::cerr << "RequestListener::start: " << ex.what() << '\n';
		}
	}

	std::cout << "All listeners successfully started\n";
}

void RequestListener::stop() {
	std::cout << "Stopping listeners\n";

	for (auto& listener : listeners) {
		ucout << U("...stopping ") << listener.uri().path() << '\n';
		listener.close();
	}

	std::cout << "All listeners successfully stopped\n";
}
