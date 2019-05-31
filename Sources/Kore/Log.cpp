#include "pch.h"

#include "Log.h"
#include "LogArgs.h"

#include <kinc/log.h>

#include <stdio.h>

void Kore::log(LogLevel level, const char* format, ...) {
	va_list args;
	va_start(args, format);
	kinc_log_args((kinc_log_level_t)level, format, args);
	va_end(args);
}

void Kore::logArgs(LogLevel level, const char* format, va_list args) {
	kinc_log_args((kinc_log_level_t)level, format, args);
}
