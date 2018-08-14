#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	KORE_LOG_LEVEL_INFO,
	KORE_LOG_LEVEL_WARNING,
	KORE_LOG_LEVEL_ERROR
} Kore_LogLevel;

KORE_FUNC void Kore_log(Kore_LogLevel logLevel, const char *format, ...);
KORE_FUNC void Kore_logArgs(Kore_LogLevel logLevel, const char *format, va_list args);

#ifdef __cplusplus
}
#endif
