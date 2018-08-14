#pragma once

/**
 * @file Error.h
 * @brief File containing functionality to stop the program in
 * case of an error and create a user-visible error message.
 *
 * The affirm and error functions print an error message and
 * then exit the program. Error messages can be made visible to
 * the user (unless a console window is active this is only
 * implemented for Windows).
 */

#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Exits the program when a condition is untrue and shows
 * a generic error message.
 *
 * This is an alternative to assert which also persists in release
 * builds. Use this instead of assert in situations where you want
 * your users to see what's going wrong.
 * This uses Kore's log and error functionality to make errors
 * visible.
 * @param condition Exits the program if condition is false,
 * otherwise does nothing.
 */
void Kore_affirm(bool condition);

/**
 * @brief Exits the program when a condition is untrue and shows
 * a provided error message.
 *
 * This is equivalent to Kore_affirm() but uses the provided message
 * instead of a generic one.
 * @param condition Exits the program if condition is false,
 * otherwise does nothing.
 * @param format The parameter is equivalent to the first printf parameter.
 * @param ... The parameter is equivalent to the second printf parameter.
 */
void Kore_affirmMessage(bool condition, const char* format, ...);

/**
 * @brief Equivalent to Kore_affirmMessage but uses a va_list parameter.
 *
 * You will need this if you want to provide the parameters using va_start/va_end.
 * @param condition Exits the program if condition is false,
 * otherwise does nothing.
 * @param format The parameter is equivalent to the first vprintf parameter.
 * @param ... The parameter is equivalent to the second vprintf parameter.
 */
void Kore_affirmArgs(bool condition, const char* format, va_list args);

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
