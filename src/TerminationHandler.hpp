#ifdef _WINDOWS
#include <Windows.h>
#else
#endif //_WINDOWS

#include <atomic>
#include <functional>

std::atomic_bool running{ true };

void registerTerminationHandler() {
#ifdef _WINDOWS
	SetConsoleCtrlHandler(
		[](auto) {
			running = false;
			return TRUE;
		},
		TRUE);
#else
#endif //_WINDOWS
}