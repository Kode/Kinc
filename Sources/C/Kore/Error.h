#pragma once

/**
 * @file Error.h
 * @brief File containing functionality to stop the program in
 * case of an error and create a user-visible error message.
 *
 * The error functions print an error message and then exit the
 * program. Error messages can be made visible to the user
 * (unless a console window is active this is only implemented
 * for Windows).
 */

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Exits the program and shows a generic error message.
 *
 * Mainly this just calls exit(EXIT_FAILURE) but will also use
 * Kore's log function and on Windows show an error message box.
 */
KORE_FUNC void Kore_error();

/**
 * @brief Exits the program and shows a provided error message.
 *
 * This is equivalent to Kore_error() but uses the provided message
 * instead of a generic one.
 * @param format The parameter is equivalent to the first printf parameter.
 * @param ... The parameter is equivalent to the second printf parameter.
 */
KORE_FUNC void Kore_errorMessage(const char *format, ...);

/**
 * @brief Equivalent to Kore_errorMessage but uses a va_list parameter.
 *
 * You will need this if you want to provide the parameters using va_start/va_end.
 * @param format The parameter is equivalent to the first vprintf parameter.
 * @param ... The parameter is equivalent to the second vprintf parameter.
 */
KORE_FUNC void Kore_errorArgs(const char* format, va_list args);

#ifdef __cplusplus
}
#endif
