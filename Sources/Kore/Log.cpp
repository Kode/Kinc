#include "pch.h"

#include "Log.h"
#include "LogArgs.h"

#include <stdio.h>

#ifdef KORE_MICROSOFT
#include <Kore/SystemMicrosoft.h>
#include <Windows.h>
#endif

#ifdef KORE_ANDROID
#include <android/log.h>
#endif

using namespace Kore;

void Kore::log(LogLevel level, const char* format, ...) {
	va_list args;
	va_start(args, format);
	// char* arg = va_arg(args, char*);
	// float arg2 = va_arg(args, double);
	logArgs(level, format, args);
	va_end(args);
}

void Kore::logArgs(LogLevel level, const char* format, va_list args) {
#ifdef KORE_MICROSOFT
	wchar_t buffer[4096];
	Microsoft::format(format, args, buffer);
	wcscat(buffer, L"\r\n");
	OutputDebugString(buffer);
#ifdef KORE_WINDOWS
	DWORD written;
	WriteConsole(GetStdHandle(level == Info ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), buffer, wcslen(buffer), &written, nullptr);
#endif

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
