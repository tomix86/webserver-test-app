#include "SimpleTimer.hpp"

#include "spdlog/spdlog.h"

SimpleTimer::SimpleTimer(std::string name, bool debug) : name{ name }, debug{ debug } {
#ifndef _WIN32
	clock_gettime(CLOCK_MONOTONIC, &begin);
#else
	begin = clock::now();
#endif
}

SimpleTimer::~SimpleTimer(void) {
	long long timeDiff;
#ifndef _WIN32
	timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	timeDiff = (end.tv_sec - begin.tv_sec) * 1000 + (end.tv_nsec - begin.tv_nsec) / (1000 * 1000);
#else
	timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - begin).count();
#endif

	constexpr auto formatString{ "Execution of \"{}\" took {}ms" };
	if (debug) {
		spdlog::debug(formatString, name, timeDiff);
	} else {
		spdlog::info(formatString, name, timeDiff);
	}
}