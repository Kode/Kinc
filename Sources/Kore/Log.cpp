#include "pch.h"

#include "Log.h"
#include "LogArgs.h"

#include <stdio.h>

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_XBOX_ONE)
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
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_XBOX_ONE)
	wchar_t formatw[4096];
	MultiByteToWideChar(CP_UTF8, 0, format, -1, formatw, 4096);

	wchar_t buffer[4096];
	int bufferIndex = 0;
	buffer[bufferIndex] = 0;
	printf("");
	for (int i = 0; formatw[i] != 0; ++i) {
		if (formatw[i] == L'%') {
			++i;
			switch (formatw[i]) {
			case L's':
			case L'S': {
				char* arg = va_arg(args, char*);
				wchar_t argw[1024];
				MultiByteToWideChar(CP_UTF8, 0, arg, -1, argw, 1024);
				wcscat(buffer, argw);
				bufferIndex += wcslen(argw);
				break;
			}
			case L'd':
			case L'i':
			case L'u':
			case L'o':
			case L'x': {
				int arg = va_arg(args, int);
				wchar_t argformat[3];
				argformat[0] = L'%';
				argformat[1] = formatw[i];
				argformat[2] = 0;
				bufferIndex += swprintf(&buffer[bufferIndex], 4096 - bufferIndex - 1, argformat, arg);
				break;
			}
			case 'f':
			case 'e':
			case 'g':
			case 'a': {
				double arg = va_arg(args, double);
				wchar_t argformat[3];
				argformat[0] = L'%';
				argformat[1] = formatw[i];
				argformat[2] = 0;
				bufferIndex += swprintf(&buffer[bufferIndex], 4096 - bufferIndex - 1, argformat, arg);
				break;
			}
			case 'c': {
				char arg = va_arg(args, char);
				wchar_t argformat[3];
				argformat[0] = L'%';
				argformat[1] = formatw[i];
				argformat[2] = 0;
				bufferIndex += swprintf(&buffer[bufferIndex], 4096 - bufferIndex - 1, argformat, arg);
				break;
			}
			case 'p':
			case 'n': {
				void* arg = va_arg(args, void*);
				wchar_t argformat[3];
				argformat[0] = L'%';
				argformat[1] = formatw[i];
				argformat[2] = 0;
				bufferIndex += swprintf(&buffer[bufferIndex], 4096 - bufferIndex - 1, argformat, arg);
				break;
			}
			case '%': {
				bufferIndex += swprintf(&buffer[bufferIndex], 4096 - bufferIndex - 1, L"%%");
				break;
			}
			}
		}
		else {
			buffer[bufferIndex++] = formatw[i];
			buffer[bufferIndex] = 0;
		}
	}

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
