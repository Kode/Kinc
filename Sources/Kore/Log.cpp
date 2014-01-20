#include "pch.h"
#include "Log.h"
#include "LogArgs.h"
#include <stdio.h>

using namespace Kore;

void Kore::log(LogLevel level, const char* format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(level == Info ? stdout : stderr, format, args);
	va_end(args);
}

void Kore::logArgs(LogLevel level, const char* format, va_list args) {
	vfprintf(level == Info ? stdout : stderr, format, args);
}
