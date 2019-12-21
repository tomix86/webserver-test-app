#include "listener.hpp"

#include <iostream>

#include "spdlog/spdlog.h"
#include "StringUtil.h"

using namespace std::string_literals;

RequestListener::RequestListener(utility::string_t baseURI) : listenerBaseURI{ baseURI } {}

auto methodNameToString(web::http::method method) {
	if (method == web::http::methods::GET) {
		return "GET";
	} else if (method == web::http::methods::PUT) {
		return "PUT";
	} else if (method == web::http::methods::DEL) {
		return "DELETE";
	} else {
		return "Unknown";
	}
}

void RequestListener::addListener(utility::string_t resource, std::function<web::http::http_response(web::http::http_request)> handler) {
	const auto method = web::http::methods::GET;

	listeners.emplace_back(listenerBaseURI + resource).support(method, [handler](web::http::http_request request) {
		auto response = handler(request);
		request.reply(response);
	});

	spdlog::info("Configured (method:path): {}:{}", methodNameToString(method), make_string(listeners.back().uri().path()));
}

bool RequestListener::start() {
	spdlog::info("Starting listener, base url: {}", make_string(listenerBaseURI));

	for (auto& listener : listeners) {
		spdlog::info("...starting {}", make_string(listener.uri().path()));

		try {
			listener.open().wait();
		} catch (const std::exception& ex) {
			spdlog::error("RequestListener::start: {}", ex.what());
			return false;
		}
	}

	spdlog::info("All listeners successfully started");
	return true;
}

void RequestListener::stop() {
	spdlog::info("Stopping listeners");

	for (auto& listener : listeners) {
		spdlog::info("...stopping {}", make_string(listener.uri().path()));
		listener.close();
	}

	spdlog::info("All listeners successfully stopped");
}
