#ifdef _WINDOWS
#include <Windows.h>
#else
 #include <signal.h>
#endif //_WINDOWS

#include <atomic>
#include <functional>

std::atomic_bool running{ true };

bool registerTerminationHandler() {
#ifdef _WINDOWS
	return SetConsoleCtrlHandler(
		[](auto) {
			running = false;
			return TRUE;
		},
		TRUE) != 0;
#else
	struct sigaction handler{};
	handler.sa_handler = [](auto){ running = false; };
	handler.sa_flags = 0;
	return sigaction(SIGTERM, &handler, nullptr) == 0 && 
		sigaction(SIGINT, &handler, nullptr) == 0;
#endif //_WINDOWS
}