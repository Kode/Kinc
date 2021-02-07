#include "pch.h"

#include "error.h"
#include "log.h"

#include <stdlib.h>

#ifdef KORE_WINDOWS
#include <Windows.h>

#include <kinc/backend/SystemMicrosoft.h>
#endif

void kinc_affirm(bool condition) {
	if (!condition) {
		kinc_error();
	}
}

void kinc_affirm_message(bool condition, const char *format, ...) {
	if (!condition) {
		va_list args;
		va_start(args, format);
		kinc_error_args(format, args);
		va_end(args);
	}
}

void kinc_affirm_args(bool condition, const char *format, va_list args) {
	if (!condition) {
		kinc_error_args(format, args);
	}
}

void kinc_error(void) {
	kinc_error_message("Unknown error");
}

void kinc_error_message(const char *format, ...) {
	{
		va_list args;
		va_start(args, format);
		kinc_log_args(KINC_LOG_LEVEL_ERROR, format, args);
		va_end(args);
	}

#ifdef KORE_WINDOWS
	{
		va_list args;
		va_start(args, format);
		wchar_t buffer[4096];
		kinc_microsoft_format(format, args, buffer);
		MessageBox(NULL, buffer, L"Error", 0);
		va_end(args);
	}
#endif

	exit(EXIT_FAILURE);
}

void kinc_error_args(const char *format, va_list args) {
	kinc_log_args(KINC_LOG_LEVEL_ERROR, format, args);

#ifdef KORE_WINDOWS
	wchar_t buffer[4096];
	kinc_microsoft_format(format, args, buffer);
	MessageBox(NULL, buffer, L"Error", 0);
#endif

	exit(EXIT_FAILURE);
}
