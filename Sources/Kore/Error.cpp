#include "pch.h"

#include "Error.h"
#include "ErrorArgs.h"
#include "Log.h"
#include "LogArgs.h"

#include <stdlib.h>

#ifdef KORE_WINDOWS
#include <Windows.h>
#include <Kore/SystemMicrosoft.h>
#endif

using namespace Kore;

void Kore::affirm(bool b) {
	if (!b) error();
}

void Kore::affirm(bool b, const char* format, ...) {
	if (!b) {
		va_list args;
		va_start(args, format);
		errorArgs(format, args);
		va_end(args);
	}
}

void Kore::affirmArgs(bool b, const char* format, va_list args) {
	if (!b) {
		errorArgs(format, args);
	}
}

void Kore::error() {
	error("Unknown error");
}


void Kore::error(const char* format, ...) {
	{
		va_list args;
		va_start(args, format);
		logArgs(Error, format, args);
		va_end(args);
	}

#ifdef KORE_WINDOWS
	{
		va_list args;
		va_start(args, format);
		wchar_t buffer[4096];
		Microsoft::format(format, args, buffer);
		MessageBox(nullptr, buffer, L"Error", 0);
		va_end(args);
	}
#endif

	exit(EXIT_FAILURE);
}

void Kore::errorArgs(const char* format, va_list args) {
	logArgs(Error, format, args);

#ifdef KORE_WINDOWS
	wchar_t buffer[4096];
	Microsoft::format(format, args, buffer);
	MessageBox(nullptr, buffer, L"Error", 0);
#endif

	exit(EXIT_FAILURE);
}
