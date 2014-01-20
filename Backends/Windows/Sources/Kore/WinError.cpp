#include "pch.h"
#include "WinError.h"
#include <Kore/ErrorArgs.h>

#define S_OK    ((HRESULT)0L)

void Kore::affirm(HRESULT result) {
	affirm(result == S_OK);
}

void Kore::affirm(HRESULT result, const char* format, ...) {
	va_list args;
	va_start(args, format);
	affirmArgs(result == S_OK, format, args);
	va_end(args);
}
