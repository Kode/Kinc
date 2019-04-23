#include "pch.h"

#include "Error.h"
#include "ErrorArgs.h"
#include "Log.h"
#include "LogArgs.h"

#include <Kinc/Error.h>

#include <stdlib.h>

using namespace Kore;

void Kore::affirm(bool b) {
#ifndef NDEBUG
	if (!b) error();
#endif
}

void Kore::affirm(bool b, const char* format, ...) {
#ifndef NDEBUG
	if (!b) {
		va_list args;
		va_start(args, format);
		errorArgs(format, args);
		va_end(args);
	}
#endif
}

void Kore::affirmArgs(bool b, const char* format, va_list args) {
	if (!b) {
		errorArgs(format, args);
	}
}

void Kore::error() {
	kinc_error();
}

void Kore::error(const char* format, ...) {
	va_list args;
	va_start(args, format);
	kinc_error_args(format, args);
	va_end(args);
}

void Kore::errorArgs(const char* format, va_list args) {
	kinc_error_args(format, args);
}
