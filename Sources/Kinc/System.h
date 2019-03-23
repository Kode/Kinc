#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int kore(int argc, char **argv);

struct _Kinc_WindowOptions;
struct _Kinc_FramebufferOptions;

int Kinc_Init(const char *name, int width, int height, struct _Kinc_WindowOptions *win, struct _Kinc_FramebufferOptions *frame);

const char *Kinc_ApplicationName(void);
int Kinc_Width(void);
int Kinc_height(void);

bool Kinc_Internal_HandleMessages(void);

//**vec2i mousePos();
//**void showKeyboard();
//**void hideKeyboard();
//**bool showsKeyboard();

void Kinc_LoadURL(const char *title);

const char *Kinc_SystemId();

const char *Kinc_Internal_SavePath();

const char **Kinc_VideoFormats();

typedef uint64_t Kinc_ticks;

double Kinc_Frequency();
Kinc_ticks Kinc_Timestamp();
double Kinc_Time();

void Kinc_Start();
bool Kinc_Internal_Frame();
void Kinc_Stop();

void Kinc_SetUpdateCallback(void (*value)());
void Kinc_SetForegroundCallback(void (*value)());
void Kinc_SetResumeCallback(void (*value)());
void Kinc_SetPauseCallback(void (*value)());
void Kinc_SetBackgroundCallback(void (*value)());
void Kinc_SetShutdownCallback(void (*value)());
void Kinc_SetDropFilesCallback(void (*value)(wchar_t *));
void Kinc_SetCutCallback(char *(*value)());
void Kinc_SetCopyCallback(char *(*value)());
void Kinc_SetPasteCallback(void (*value)(char *));
void Kinc_SetKeepScreenOn(bool on);

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
