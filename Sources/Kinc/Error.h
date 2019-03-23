#pragma once

/// <summary>
/// File containing functionality to stop the program in
/// case of an error and create a user-visible error message
/// </summary>
/// <remarks>
///The affirm and error functions print an error message and
/// then exit the program. Error messages can be made visible to
/// the user (unless a console window is active this is only
/// implemented for Windows).
/// </remarks>

#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Exits the program when a condition is untrue and shows
/// a generic error message.
/// </summary>
/// <remarks>
/// This is an alternative to assert which also persists in release
/// builds. Use this instead of assert in situations where you want
/// your users to see what's going wrong.
/// This uses Kore's log and error functionality to make errors
/// visible.
/// </remarks>
/// <param name="condition">
/// Exits the program if condition is false,
/// otherwise does nothing.
/// </param>
KORE_FUNC void Kinc_Affirm(bool condition);

/// <summary>
/// Exits the program when a condition is untrue and shows
/// a provided error message.
/// </summary>
/// <remarks>
/// This is equivalent to Kinc_affirm() but uses the provided message
/// instead of a generic one.
/// </remarks>
/// <param name="condition">
/// Exits the program if condition is false,
/// otherwise does nothing.
/// </param>
/// <param name="format">
/// The parameter is equivalent to the first printf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second printf parameter.
/// </param>
KORE_FUNC void Kinc_AffirmMessage(bool condition, const char* format, ...);

/// <summary>
/// Equivalent to Kinc_affirmMessage but uses a va_list parameter.
/// </summary>
/// <remarks>
/// You will need this if you want to provide the parameters using va_start/va_end.
/// </remarks>
/// <param name="condition">
/// Exits the program if condition is false,
/// otherwise does nothing.
/// </param>
/// <param name="format">
/// The parameter is equivalent to the first vprintf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second vprintf parameter.
/// </param>
KORE_FUNC void Kinc_AffirmArgs(bool condition, const char* format, va_list args);

/// <summary>
/// Exits the program and shows a generic error message
/// </summary>
/// <remarks>
/// Mainly this just calls exit(EXIT_FAILURE) but will also use
/// Kore's log function and on Windows show an error message box.
/// </remarks>
KORE_FUNC void Kinc_Error();

/// <summary>
/// Exits the program and shows a provided error message.
/// </summary>
/// <remarks>
/// This is equivalent to Kinc_error() but uses the provided message
/// instead of a generic one.
/// </remarks>
/// <param name="format">
/// The parameter is equivalent to the first printf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second printf parameter.
/// </param>
KORE_FUNC void Kinc_ErrorMessage(const char *format, ...);

/// <summary>
/// Equivalent to Kinc_errorMessage but uses a va_list parameter.
/// </summary>
/// <remarks>
/// You will need this if you want to provide the parameters using va_start/va_end.
/// </remarks>
/// <param name="format">
/// The parameter is equivalent to the first vprintf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second vprintf parameter.
/// </param>
KORE_FUNC void Kinc_ErrorArgs(const char* format, va_list args);

#ifdef __cplusplus
}
#endif
