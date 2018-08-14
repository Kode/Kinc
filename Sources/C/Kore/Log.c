#include "pch.h"

#include "Log.h"

#include <stdio.h>

#ifdef KORE_MICROSOFT
#include <Kore/SystemMicrosoft.h>
#include <Windows.h>
#endif

#ifdef KORE_ANDROID
#include <android/log.h>
#endif

void Kore_log(Kore_LogLevel level, const char* format, ...) {
	va_list args;
	va_start(args, format);
	Kore_logArgs(level, format, args);
	va_end(args);
}

void Kore_logArgs(Kore_LogLevel level, const char* format, va_list args) {
#ifdef KORE_MICROSOFT
	wchar_t buffer[4096];
	Kore_Microsoft_format(format, args, buffer);
	wcscat(buffer, L"\r\n");
	OutputDebugString(buffer);
#ifdef KORE_WINDOWS
	DWORD written;
	WriteConsole(GetStdHandle(level == KORE_LOG_LEVEL_INFO ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), buffer, wcslen(buffer), &written, NULL);
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
