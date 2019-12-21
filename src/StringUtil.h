#pragma once
#include <cpprest/details/basic_types.h>
#include <string>

static auto make_string_t(const std::string& input) {
	return utility::string_t{ input.begin(), input.end() };
}

static auto make_string(const utility::string_t& input) {
	std::string result;

	for (auto c : input) {
		result.push_back(static_cast<char>(c));
	}

	return result;
}