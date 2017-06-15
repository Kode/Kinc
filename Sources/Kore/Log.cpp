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
	wchar_t formatw[4096];
	MultiByteToWideChar(CP_UTF8, 0, format, -1, formatw, 4096);

	wchar_t buffer[4096];
	int bufferIndex = 0;
	buffer[bufferIndex] = 0;

	for (int i = 0; formatw[i] != 0; ++i) {
		if (formatw[i] == L'%') {
			++i;
			if (formatw[i] == L's' || formatw[i] == L'S') {
				char* arg = va_arg(args, char*);
				wchar_t argw[1024];
				MultiByteToWideChar(CP_UTF8, 0, arg, -1, argw, 1024);
				wcscat(buffer, argw);
				bufferIndex += wcslen(argw);
			}
			else {
				void* arg = va_arg(args, void*);
				wchar_t argformat[3];
				argformat[0] = L'%';
				argformat[1] = formatw[i];
				argformat[2] = 0;
				bufferIndex += swprintf(&buffer[bufferIndex], 4096 - bufferIndex - 1, argformat, arg);
			}
		}
		else {
			buffer[bufferIndex++] = formatw[i];
			buffer[bufferIndex] = 0;
		}
	}

	wcscat(buffer, L"\r\n");
	OutputDebugString(buffer);
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
