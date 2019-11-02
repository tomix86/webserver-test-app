#pragma once
#include <cpprest/http_listener.h>
#include <functional>
#include <string>

class RequestListener {
public:
	RequestListener(utility::string_t baseURI);

	void addListener(utility::string_t resource, std::function<web::http::http_response(web::http::http_request)> handler);

	bool start();
	void stop();

private:
	using http_listener = web::http::experimental::listener::http_listener;

	utility::string_t listenerBaseURI;
	std::vector<http_listener> listeners;
};
