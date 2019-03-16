#include "pch.h"

#include "Log.h"
#include "LogArgs.h"

#include <Kinc/Log.h>

#include <stdio.h>

void Kore::log(LogLevel level, const char* format, ...) {
	va_list args;
	va_start(args, format);
	Kore_LogArgs((Kore_LogLevel)level, format, args);
	va_end(args);
}

void Kore::logArgs(LogLevel level, const char* format, va_list args) {
	Kore_LogArgs((Kore_LogLevel)level, format, args);
}
