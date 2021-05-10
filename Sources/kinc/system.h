#pragma once

#include <kinc/global.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_window_options;
struct kinc_framebuffer_options;

KINC_FUNC int kinc_init(const char *name, int width, int height, struct kinc_window_options *win, struct kinc_framebuffer_options *frame);

KINC_FUNC const char *kinc_application_name(void);
KINC_FUNC void kinc_set_application_name(const char *name);
KINC_FUNC int kinc_width(void);
KINC_FUNC int kinc_height(void);

KINC_FUNC bool kinc_internal_handle_messages(void);

KINC_FUNC void kinc_load_url(const char *url);

KINC_FUNC const char *kinc_system_id(void);

KINC_FUNC const char *kinc_internal_save_path(void);

KINC_FUNC const char **kinc_video_formats(void);

KINC_FUNC const char *kinc_language(void);

KINC_FUNC void kinc_vibrate(int milliseconds);

KINC_FUNC float kinc_safe_zone(void);
KINC_FUNC bool kinc_automatic_safe_zone(void);
KINC_FUNC void kinc_set_safe_zone(float value);

typedef uint64_t kinc_ticks_t;

KINC_FUNC double kinc_frequency(void);
KINC_FUNC kinc_ticks_t kinc_timestamp(void);
KINC_FUNC double kinc_time(void);

KINC_FUNC void kinc_start(void);
bool kinc_internal_frame(void);
KINC_FUNC void kinc_stop(void);

KINC_FUNC void kinc_login(void);
KINC_FUNC bool kinc_waiting_for_login(void);
KINC_FUNC void kinc_unlock_achievement(int id);
KINC_FUNC void kinc_disallow_user_change(void);
KINC_FUNC void kinc_allow_user_change(void);

KINC_FUNC void kinc_set_keep_screen_on(bool on);

KINC_FUNC void kinc_set_update_callback(void (*value)(void));
KINC_FUNC void kinc_set_foreground_callback(void (*value)(void));
KINC_FUNC void kinc_set_resume_callback(void (*value)(void));
KINC_FUNC void kinc_set_pause_callback(void (*value)(void));
KINC_FUNC void kinc_set_background_callback(void (*value)(void));
KINC_FUNC void kinc_set_shutdown_callback(void (*value)(void));
KINC_FUNC void kinc_set_drop_files_callback(void (*value)(wchar_t *));
KINC_FUNC void kinc_set_cut_callback(char *(*value)(void));
KINC_FUNC void kinc_set_copy_callback(char *(*value)(void));
KINC_FUNC void kinc_set_paste_callback(void (*value)(char *));
KINC_FUNC void kinc_set_login_callback(void (*value)(void));
KINC_FUNC void kinc_set_logout_callback(void (*value)(void));

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

KINC_FUNC void kinc_copy_to_clipboard(const char *);

#ifdef __cplusplus
}
#endif
