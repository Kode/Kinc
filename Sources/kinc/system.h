#pragma once

#include <kinc/global.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*! \file system.h
    \brief Provides basic system and application-management functionality which doesn't fit anywhere else.
*/

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_window_options;
struct kinc_framebuffer_options;

/// <summary>
/// Initializes a Kinc application and creates an initial window for systems which support windows (systems which do not support windows are treated as if the
/// would provide a single window which can not change). This has to be called before any other Kinc-function with the exception of the Display-API which can
/// optionally be initialized beforehand using kinc_display_init.
/// </summary>
/// <returns>The id of the initial window</returns>
KINC_FUNC int kinc_init(const char *name, int width, int height, struct kinc_window_options *win, struct kinc_framebuffer_options *frame);

/// <summary>
/// Returns the current application name as set by kinc_init or kinc_set_application_name.
/// </summary>
/// <returns>The current name of the application</returns>
KINC_FUNC const char *kinc_application_name(void);

/// <summary>
/// Changes the application-name that was initially set by kinc_init.
/// </summary>
/// <param name="name">The new application-name</param>
KINC_FUNC void kinc_set_application_name(const char *name);

/// <summary>
/// Returns the current width of the initial application-window which is equivalent to calling kinc_window_width(0).
/// </summary>
/// <returns>The width of the initial window</returns>
KINC_FUNC int kinc_width(void);

/// <summary>
/// Returns the current height of the initial application-window which is equivalent to calling kinc_window_height(0).
/// </summary>
/// <returns>The height of the initial window</returns>
KINC_FUNC int kinc_height(void);

/// <summary>
/// Instruct the system to load up the provided URL which will usually open it in the system's default browser.
/// </summary>
/// <param name="url">The URL to open</param>
KINC_FUNC void kinc_load_url(const char *url);

/// <summary>
/// Returns an ID representing the current type of target-system.
/// </summary>
/// <returns>The ID representing the target system</returns>
KINC_FUNC const char *kinc_system_id(void);

/// <summary>
/// Returns the current system-language.
/// </summary>
/// <returns>The current system-language as a two-letter language code</returns>
KINC_FUNC const char *kinc_language(void);

/// <summary>
/// Vibrates the whole system if supported. This is primarily supported on mobile phones but don't blame us if your computer falls over.
/// </summary>
KINC_FUNC void kinc_vibrate(int milliseconds);

/// <summary>
/// Returns the portion of the screen which can be safely used for important content. This is mostly relevant for TVs which often scale the image by default and
/// thefore cut off some of the content.
/// </summary>
/// <returns>The safe-zone which can be multiplied with the width or height of the display to convert it to pixels</returns>
KINC_FUNC float kinc_safe_zone(void);

/// <summary>
/// Returns whether the system itself handles configuration of the safe-zone.
/// </summary>
/// <returns>Whether the safe-zone is handlet by the syste</returns>
KINC_FUNC bool kinc_automatic_safe_zone(void);

/// <summary>
/// Sets the safe-zone for systems which return false for kinc_automatic_safe_zone.
/// </summary>
/// <param name="value">The safe-zone for width and height as a ratio of the full display-resolution.</param>
KINC_FUNC void kinc_set_safe_zone(float value);

typedef uint64_t kinc_ticks_t;

/// <summary>
/// Returns the frequency of system-timestamps.
/// </summary>
/// <returns>The frequency of the system's timestamps in 1 / second</returns>
KINC_FUNC double kinc_frequency(void);

/// <summary>
/// Returns a timestamp for right now in a system-specific unit.
/// </summary>
/// <returns>The current timestamp</returns>
KINC_FUNC kinc_ticks_t kinc_timestamp(void);

/// <summary>
/// Returns the current time. This can also be calculated ala kinc_timestamp() / kinc_frequency() but kinc_time is a little more precise on some systems.
/// </summary>
/// <returns>The current time in seconds</returns>
KINC_FUNC double kinc_time(void);

/// <summary>
/// Starts Kinc's main-loop. kinc_set_update_callback should be called before kinc_start so the main-loop actually has something to do.
/// </summary>
KINC_FUNC void kinc_start(void);

/// <summary>
/// Stops Kinc's main loop and thereby returns to the function which called kinc_start.
/// </summary>
KINC_FUNC void kinc_stop(void);

/// <summary>
/// Instructs the system to login a user if that is supported.
/// </summary>
KINC_FUNC void kinc_login(void);

/// <summary>
/// Returns true if kinc_login was called and the login-process is still ongoing.
/// </summary>
/// <returns>Whether a login-process is still in progress</returns>
KINC_FUNC bool kinc_waiting_for_login(void);

/// <summary>
/// Unlocks an achievement or trophy or however you prefer to call it.
/// </summary>
/// <param name="id">The id of the achievement/tropy</param>
KINC_FUNC void kinc_unlock_achievement(int id);

/// <summary>
/// Disallows the system to logout the current user.
/// </summary>
KINC_FUNC void kinc_disallow_user_change(void);

/// <summary>
/// Allows the system to logout the current user.
/// </summary>
KINC_FUNC void kinc_allow_user_change(void);

/// <summary>
/// Instructs the system whether it is allowed to turn of the screen while the application is running.
/// </summary>
/// <param name="on">Whether turning off the screen is allowed</param>
KINC_FUNC void kinc_set_keep_screen_on(bool on);

/// <summary>
/// Copies the provided string to the system's clipboard.
/// </summary>
/// <param name="text">The text to be copied into the clipboard</param>
KINC_FUNC void kinc_copy_to_clipboard(const char *text);

/// <summary>
/// Sets the update-callback which drives the application and is called for every frame.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_set_update_callback(void (*value)(void));

/// <summary>
/// Sets a callback which is called whenever the application is brought to the foreground.
/// </summary>
/// <param name="value">The foreground-callback</param>
KINC_FUNC void kinc_set_foreground_callback(void (*value)(void));

/// <summary>
/// Sets a callback which is called whenever the application was paused and is being resumed.
/// </summary>
/// <param name="value">The resume-callback</param>
KINC_FUNC void kinc_set_resume_callback(void (*value)(void));

/// <summary>
/// Sets a callback which is called whenever the application is paused.
/// </summary>
/// <param name="value">The pause-callback</param>
KINC_FUNC void kinc_set_pause_callback(void (*value)(void));

/// <summary>
/// Sets a callback which is called whenever the application is brought to the background.
/// </summary>
/// <param name="value">The background-callback</param>
KINC_FUNC void kinc_set_background_callback(void (*value)(void));

/// <summary>
/// Sets a callback which is called whenever the application is about to shut down.
/// </summary>
/// <param name="value">The shutdown-callback</param>
KINC_FUNC void kinc_set_shutdown_callback(void (*value)(void));

/// <summary>
/// Sets a callback which is called when files are dropped on the application-window.
/// </summary>
/// <param name="value">The drop-files-callback</param>
KINC_FUNC void kinc_set_drop_files_callback(void (*value)(wchar_t *));

/// <summary>
/// Sets a callback which is called when the application is instructed to cut, typically via ctrl+x or cmd+x.
/// </summary>
/// <param name="value">The cut-callback</param>
KINC_FUNC void kinc_set_cut_callback(char *(*value)(void));

/// <summary>
/// Sets a callback which is called when the application is instructed to copy, typically via ctrl+c or cmd+c.
/// </summary>
/// <param name="value">The copy-callback</param>
KINC_FUNC void kinc_set_copy_callback(char *(*value)(void));

/// <summary>
/// Sets a callback which is called when the application is instructed to paste, typically via ctrl+v or cmd+v.
/// </summary>
/// <param name="value">The paste-callback</param>
KINC_FUNC void kinc_set_paste_callback(void (*value)(char *));

/// <summary>
/// Sets a callback which is called when a user logs in.
/// </summary>
/// <param name="value">The login-callback</param>
KINC_FUNC void kinc_set_login_callback(void (*value)(void));

/// <summary>
/// Sets a callback which is called when a user logs out.
/// </summary>
/// <param name="value">The logout-callback</param>
KINC_FUNC void kinc_set_logout_callback(void (*value)(void));

bool kinc_internal_frame(void);
const char *kinc_internal_save_path(void);
bool kinc_internal_handle_messages(void);
void kinc_internal_shutdown(void);
void kinc_internal_update_callback(void);
void kinc_internal_foreground_callback(void);
void kinc_internal_resume_callback(void);
void kinc_internal_pause_callback(void);
void kinc_internal_background_callback(void);
void kinc_internal_shutdown_callback(void);
void kinc_internal_drop_files_callback(wchar_t *);
char *kinc_internal_cut_callback(void);
char *kinc_internal_copy_callback(void);
void kinc_internal_paste_callback(char *);
void kinc_internal_login_callback(void);
void kinc_internal_logout_callback(void);

#ifdef __cplusplus
}
#endif
