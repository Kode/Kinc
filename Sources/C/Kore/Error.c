#include "pch.h"

#include "Error.h"
#include "Log.h"

#include <stdlib.h>

#ifdef KORE_WINDOWS
#include <Windows.h>

#include <Kore/SystemMicrosoft.h>
#endif

void Kore_error() {
	Kore_errorMessage("Unknown error");
}

void Kore_errorMessage(const char* format, ...) {
	{
		va_list args;
		va_start(args, format);
		Kore_logArgs(KORE_LOG_LEVEL_ERROR, format, args);
		va_end(args);
	}

#ifdef KORE_WINDOWS
	{
		va_list args;
		va_start(args, format);
		wchar_t buffer[4096];
		Kore_Microsoft_format(format, args, buffer);
		MessageBox(NULL, buffer, L"Error", 0);
		va_end(args);
	}
#endif

	exit(EXIT_FAILURE);
}

void Kore_errorArgs(const char* format, va_list args) {
	Kore_logArgs(KORE_LOG_LEVEL_ERROR, format, args);

#ifdef KORE_WINDOWS
	wchar_t buffer[4096];
	Kore_Microsoft_format(format, args, buffer);
	MessageBox(NULL, buffer, L"Error", 0);
#endif

	exit(EXIT_FAILURE);
}
