#include "pch.h"

#include "Error.h"
#include "Log.h"

#include <stdlib.h>

#ifdef KORE_WINDOWS
#include <Windows.h>

#include <Kore/SystemMicrosoft.h>
#endif

void Kore_Affirm(bool condition) {
	if (!condition) {
		Kore_Error();
	}
}

void Kore_AffirmMessage(bool condition, const char* format, ...) {
	if (!condition) {
		va_list args;
		va_start(args, format);
		Kore_ErrorArgs(format, args);
		va_end(args);
	}
}

void Kore_AffirmArgs(bool condition, const char* format, va_list args) {
	if (!condition) {
		Kore_ErrorArgs(format, args);
	}
}

void Kore_Error() {
	Kore_ErrorMessage("Unknown error");
}

void Kore_ErrorMessage(const char* format, ...) {
	{
		va_list args;
		va_start(args, format);
		Kore_LogArgs(KORE_LOG_LEVEL_ERROR, format, args);
		va_end(args);
	}

#ifdef KORE_WINDOWS
	{
		va_list args;
		va_start(args, format);
		wchar_t buffer[4096];
		Kore_Microsoft_Format(format, args, buffer);
		MessageBox(NULL, buffer, L"Error", 0);
		va_end(args);
	}
#endif

	exit(EXIT_FAILURE);
}

void Kore_ErrorArgs(const char* format, va_list args) {
	Kore_LogArgs(KORE_LOG_LEVEL_ERROR, format, args);

#ifdef KORE_WINDOWS
	wchar_t buffer[4096];
	Kore_Microsoft_Format(format, args, buffer);
	MessageBox(NULL, buffer, L"Error", 0);
#endif

	exit(EXIT_FAILURE);
}
