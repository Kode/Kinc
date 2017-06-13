#include "pch.h"

#include "Log.h"
#include "LogArgs.h"

#include <stdio.h>
#ifdef KORE_WINDOWS
#include <Windows.h>
#endif
#ifdef KORE_ANDROID
#include <android/log.h>
#endif

using namespace Kore;

void Kore::log(LogLevel level, const char* format, ...) {
	va_list args;
	va_start(args, format);
	logArgs(level, format, args);
	va_end(args);
}

void Kore::logArgs(LogLevel level, const char* format, va_list args) {
#ifdef KORE_WINDOWS
	wchar_t buffer[4096];
	MultiByteToWideChar(CP_UTF8, 0, format, -1, buffer, 4096 - 2);
	_vsnwprintf(buffer, sizeof(buffer) - 2, buffer, args);
	wcscat(buffer, L"\r\n");
	OutputDebugStringW(buffer);
	vfwprintf(level == Info ? stdout : stderr, buffer, args);
#else
	vfprintf(level == Info ? stdout : stderr, format, args);
	fprintf(level == Info ? stdout : stderr, "\n");
#endif

#ifdef KORE_ANDROID
	switch (level) {
	case Info:
		__android_log_vprint(ANDROID_LOG_INFO, "kore", format, args);
		break;
	case Warning:
		__android_log_vprint(ANDROID_LOG_WARN, "kore", format, args);
		break;
	case Error:
		__android_log_vprint(ANDROID_LOG_ERROR, "kore", format, args);
		break;
	}
#endif
}
