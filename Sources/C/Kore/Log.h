#pragma once

/**
 * @file Log.h
 * @brief File containing basic logging functionality.
 *
 * Logging functionality is similar to plain printf but provides
 * some system-specific bonuses.
 */

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Pass this to Kore_log or Kore_logArgs.
 *
 * When used on Android the log level is converted to the equivalent
 * Android logging level. It is currently ignored on all other targets.
 */
typedef enum {
	KORE_LOG_LEVEL_INFO,
	KORE_LOG_LEVEL_WARNING,
	KORE_LOG_LEVEL_ERROR
} Kore_LogLevel;

/**
 * @brief Logging function similar to printf including some system-specific bonuses.
 *
 * On most systems this is equivalent to printf.
 * On Windows it takes utf-8 string (like printf does on any other target)
 * and also prints to the debug console in IDEs.
 * On Android this uses the android logging functions and also passes
 * the logging level.
 * @param logLevel The logLevel is ignored on all targets but Android where
 * it is converted to the equivalent Android log level.
 * @param format The parameter is equivalent to the first printf parameter.
 * @param ... The parameter is equivalent to the second printf parameter.
 */
KORE_FUNC void Kore_log(Kore_LogLevel logLevel, const char *format, ...);

/**
 * @brief Equivalent to Kore_log but uses a va_list parameter.
 *
 * You will need this if you want to log parameters using va_start/va_end.
 * @param logLevel The logLevel is ignored on all targets but Android where
 * it is converted to the equivalent Android log level.
 * @param format The parameter is equivalent to the first vprintf parameter.
 * @param args The parameter is equivalent to the second vprintf parameter.
 */
KORE_FUNC void Kore_logArgs(Kore_LogLevel logLevel, const char *format, va_list args);

#ifdef __cplusplus
}
#endif
