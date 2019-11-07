#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_window_options;
struct kinc_framebuffer_options;

int kinc_init(const char *name, int width, int height, struct kinc_window_options *win, struct kinc_framebuffer_options *frame);

const char *kinc_application_name(void);
void kinc_set_application_name(const char *name);
int kinc_width(void);
int kinc_height(void);

bool kinc_internal_handle_messages(void);

void kinc_load_url(const char *url);

const char *kinc_system_id();

const char *kinc_internal_save_path();

const char **kinc_video_formats();

const char *kinc_language();

void kinc_vibrate(int milliseconds);

float kinc_safe_zone();
bool kinc_automatic_safe_zone();
void kinc_set_safe_zone(float value);

typedef uint64_t kinc_ticks_t;

double kinc_frequency();
kinc_ticks_t kinc_timestamp();
double kinc_time();

void kinc_start();
bool kinc_internal_frame();
void kinc_stop();

void kinc_login();
void kinc_unlock_achievement(int id);
void kinc_disallow_user_change();
void kinc_allow_user_change();

void kinc_set_keep_screen_on(bool on);

void kinc_set_update_callback(void (*value)());
void kinc_set_foreground_callback(void (*value)());
void kinc_set_resume_callback(void (*value)());
void kinc_set_pause_callback(void (*value)());
void kinc_set_background_callback(void (*value)());
void kinc_set_shutdown_callback(void (*value)());
void kinc_set_drop_files_callback(void (*value)(wchar_t *));
void kinc_set_cut_callback(char *(*value)());
void kinc_set_copy_callback(char *(*value)());
void kinc_set_paste_callback(void (*value)(char *));
void kinc_set_login_callback(void (*value)());
void kinc_set_logout_callback(void (*value)());

void kinc_internal_shutdown();
void kinc_internal_update_callback();
void kinc_internal_foreground_callback();
void kinc_internal_resume_callback();
void kinc_internal_pause_callback();
void kinc_internal_background_callback();
void kinc_internal_shutdown_callback();
void kinc_internal_drop_files_callback(wchar_t *);
char *kinc_internal_cut_callback();
char *kinc_internal_copy_callback();
void kinc_internal_paste_callback(char *);
void kinc_internal_login_callback();
void kinc_internal_logout_callback();

#ifdef __cplusplus
}
#endif
