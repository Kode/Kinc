#pragma once

#include <kinc/global.h>

#include <stdarg.h>

/// <summary>
/// Contains basic logging functionality
/// </summary>
/// <remarks>
/// Logging functionality is similar to plain printf but provides
/// some system-specific bonuses.
/// </remarks>

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Pass this to kinc_log or kinc_log_args
/// </summary>
/// <remarks>
/// When used on Android the log level is converted to the equivalent
/// Android logging level. It is currently ignored on all other targets.
/// </remarks>
typedef enum { KINC_LOG_LEVEL_INFO, KINC_LOG_LEVEL_WARNING, KINC_LOG_LEVEL_ERROR } kinc_log_level_t;

/// <summary>
/// Logging function similar to printf including some system-specific bonuses
/// </summary>
/// <remarks>
/// On most systems this is equivalent to printf.
/// On Windows it works with utf-8 strings (like printf does on any other target)
/// and also prints to the debug console in IDEs.
/// On Android this uses the android logging functions and also passes the logging level.
/// </remarks>
/// <param name="log_level">
/// The logLevel is ignored on all targets but Android where it is converted
/// to the equivalent Android log level
/// </param>
/// <param name="format">The parameter is equivalent to the first printf parameter.</param>
/// <param name="...">The parameter is equivalent to the second printf parameter.</param>
KINC_FUNC void kinc_log(kinc_log_level_t log_level, const char *format, ...);

/// <summary>
/// Equivalent to kinc_log but uses a va_list parameter
/// </summary>
/// <remarks>
/// You will need this if you want to log parameters using va_start/va_end.
/// </remarks>
/// <param name="log_level">
/// The logLevel is ignored on all targets but Android where it is converted
/// to the equivalent Android log level
/// </param>
/// <param name="format">The parameter is equivalent to the first vprintf parameter.</param>
/// <param name="args">The parameter is equivalent to the second vprintf parameter.</param>
KINC_FUNC void kinc_log_args(kinc_log_level_t log_level, const char *format, va_list args);

#ifdef __cplusplus
}
#endif
