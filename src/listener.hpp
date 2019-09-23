#pragma once
#include <string>
#include <functional>

#include <cpprest/http_listener.h>

class RequestListener {
public:
	RequestListener(utility::string_t baseURI);

	void addListener(utility::string_t resource, std::function< web::http::http_response(web::http::http_request)> handler);

	void start();
	void stop();

private:
	using http_listener = web::http::experimental::listener::http_listener;

	utility::string_t listenerBaseURI;
	std::vector<http_listener> listeners;
};
