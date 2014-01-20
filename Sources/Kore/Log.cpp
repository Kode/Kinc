#include "pch.h"
#include "Log.h"
#include "LogArgs.h"
#include <stdio.h>
#ifdef SYS_WINDOWS
#include <Windows.h>
#endif

using namespace Kore;

void Kore::log(LogLevel level, const char* format, ...) {
	va_list args;
	va_start(args, format);
	logArgs(level, format, args);
	va_end(args);
}

void Kore::logArgs(LogLevel level, const char* format, va_list args) {
	vfprintf(level == Info ? stdout : stderr, format, args);
	fprintf(level == Info ? stdout : stderr, "\n");
#ifdef SYS_WINDOWS
	char buffer[4096];
	vsprintf(buffer, format, args);
	strcat(buffer, "\r\n");
	OutputDebugStringA(buffer);
#endif
}
