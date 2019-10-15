#pragma once
#include <iostream>
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

	SimpleTimer(std::string name) : name(name) {
#ifndef _WIN32
		clock_gettime(CLOCK_MONOTONIC, &begin);
#else
		begin = clock::now();
#endif
	}

	~SimpleTimer(void) {
		long long timeDiff;
#ifndef _WIN32
		timespec end;
		clock_gettime(CLOCK_MONOTONIC, &end);
		timeDiff = (end.tv_sec - begin.tv_sec) * 1000 + (end.tv_nsec - begin.tv_nsec) / (1000 * 1000);
#else
		timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - begin).count();
#endif

		std::cout << "Execution of \"" << name << "\" took " << timeDiff << "ms" << std::endl;
		//	std::cout << timeDiff << " ";
	}

private:
	std::string name;
#ifndef _WIN32
	timespec begin;
#else
	clock::time_point begin;
#endif
};