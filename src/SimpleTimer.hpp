#pragma once
#include <string>

#ifndef _WIN32
#include <time.h>
#else
#include <chrono>
#endif

class SimpleTimer {
public:
#ifdef _WIN32
	typedef std::chrono::high_resolution_clock clock;
#endif

	explicit SimpleTimer(std::string name, bool debug = false);
	~SimpleTimer(void);

private:
	const std::string name;
	const bool debug;
#ifndef _WIN32
	timespec begin;
#else
	clock::time_point begin;
#endif
};