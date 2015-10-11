#include "pch.h"
#include "WinError.h"
#include <Kore/ErrorArgs.h>
#include <Windows.h>

#define S_OK ((HRESULT)0L)

namespace {
	void winerror() {
		LPVOID buffer = nullptr;
		DWORD dw = GetLastError();

		if (dw != 0) {
			FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, NULL);

			Kore::error("Error: %s", buffer);
		}
		else {
			Kore::error();
		}
	}
}

void Kore::affirm(HRESULT result) {
	if (result != S_OK) {
		winerror();
	}
}

void Kore::affirm(HRESULT result, const char* format, ...) {
	va_list args;
	va_start(args, format);
	affirmArgs(result == S_OK, format, args);
	va_end(args);
}
