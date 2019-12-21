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

	SimpleTimer(std::string name);
	~SimpleTimer(void);

private:
	const std::string name;
#ifndef _WIN32
	timespec begin;
#else
	clock::time_point begin;
#endif
};