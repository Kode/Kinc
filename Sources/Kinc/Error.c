#include "pch.h"

#include "Error.h"
#include "Log.h"

#include <stdlib.h>

#ifdef KORE_WINDOWS
#include <Windows.h>

#include <Kore/SystemMicrosoft.h>
#endif

void Kinc_Affirm(bool condition) {
	if (!condition) {
		Kinc_Error();
	}
}

void Kinc_AffirmMessage(bool condition, const char* format, ...) {
	if (!condition) {
		va_list args;
		va_start(args, format);
		Kinc_ErrorArgs(format, args);
		va_end(args);
	}
}

void Kinc_AffirmArgs(bool condition, const char* format, va_list args) {
	if (!condition) {
		Kinc_ErrorArgs(format, args);
	}
}

void Kinc_Error() {
	Kinc_ErrorMessage("Unknown error");
}

void Kinc_ErrorMessage(const char* format, ...) {
	{
		va_list args;
		va_start(args, format);
		Kinc_LogArgs(KINC_LOG_LEVEL_ERROR, format, args);
		va_end(args);
	}

#ifdef KORE_WINDOWS
	{
		va_list args;
		va_start(args, format);
		wchar_t buffer[4096];
		Kinc_Microsoft_Format(format, args, buffer);
		MessageBox(NULL, buffer, L"Error", 0);
		va_end(args);
	}
#endif

	exit(EXIT_FAILURE);
}

void Kinc_ErrorArgs(const char* format, va_list args) {
	Kinc_LogArgs(KINC_LOG_LEVEL_ERROR, format, args);

#ifdef KORE_WINDOWS
	wchar_t buffer[4096];
	Kinc_Microsoft_Format(format, args, buffer);
	MessageBox(NULL, buffer, L"Error", 0);
#endif

	exit(EXIT_FAILURE);
}
