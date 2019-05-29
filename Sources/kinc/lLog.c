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

void kinc_log(kinc_log_level_t level, const char *format, ...) {
	va_list args;
	va_start(args, format);
	kinc_log_args(level, format, args);
	va_end(args);
}

void kinc_log_args(kinc_log_level_t level, const char *format, va_list args) {
#ifdef KORE_MICROSOFT
	wchar_t buffer[4096];
	Kinc_Microsoft_Format(format, args, buffer);
	wcscat(buffer, L"\r\n");
	OutputDebugString(buffer);
#ifdef KORE_WINDOWS
	DWORD written;
	WriteConsole(GetStdHandle(level == KINC_LOG_LEVEL_INFO ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), buffer, (DWORD)wcslen(buffer), &written, NULL);
#endif

#else
	vfprintf(level == KINC_LOG_LEVEL_INFO ? stdout : stderr, format, args);
	fprintf(level == KINC_LOG_LEVEL_INFO ? stdout : stderr, "\n");
#endif

#ifdef KORE_ANDROID
	switch (level) {
	case KORE_LOG_LEVEL_INFO:
		__android_log_vprint(ANDROID_LOG_INFO, "kore", format, args);
		break;
	case KORE_LOG_LEVEL_WARNING:
		__android_log_vprint(ANDROID_LOG_WARN, "kore", format, args);
		break;
	case KORE_LOG_LEVEL_ERROR:
		__android_log_vprint(ANDROID_LOG_ERROR, "kore", format, args);
		break;
	}
#endif
}
