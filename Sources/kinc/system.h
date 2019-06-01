#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int kore(int argc, char **argv);

struct _Kinc_WindowOptions;
struct _Kinc_FramebufferOptions;

int kinc_init(const char *name, int width, int height, struct _Kinc_WindowOptions *win, struct _Kinc_FramebufferOptions *frame);

const char *kinc_application_name(void);
int kinc_width(void);
int kinc_height(void);

bool Kinc_Internal_HandleMessages(void);

//**vec2i mousePos();
//**void showKeyboard();
//**void hideKeyboard();
//**bool showsKeyboard();

void kinc_load_url(const char *title);

const char *kinc_system_id();

const char *Kinc_Internal_SavePath();

const char **kinc_video_formats();

typedef uint64_t kinc_ticks_t;

double kinc_frequency();
kinc_ticks_t kinc_timestamp();
double kinc_time();

void kinc_start();
bool Kinc_Internal_Frame();
void kinc_stop();

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
void kinc_set_keep_screen_on(bool on);

void Kinc_Internal_Shutdown();
void Kinc_Internal_UpdateCallback();
void Kinc_Internal_ForegroundCallback();
void Kinc_Internal_ResumeCallback();
void Kinc_Internal_PauseCallback();
void Kinc_Internal_BackgroundCallback();
void Kinc_Internal_ShutdownCallback();
void Kinc_Internal_DropFilesCallback(wchar_t *);
char *Kinc_Internal_CutCallback();
char *Kinc_Internal_CopyCallback();
void Kinc_Internal_PasteCallback(char *);

#ifdef __cplusplus
}
#endif
