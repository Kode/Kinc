#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int kore(int argc, char **argv);

struct _Kore_WindowOptions;
struct _Kore_FramebufferOptions;

int Kore_Init(const char *name, int width, int height, struct _Kore_WindowOptions *win, struct _Kore_FramebufferOptions *frame);

const char *Kore_ApplicationName();
int Kore_Width();
int Kore_height();

bool Kore_Internal_HandleMessages();

//**vec2i mousePos();
//**void showKeyboard();
//**void hideKeyboard();
//**bool showsKeyboard();

void Kore_LoadURL(const char *title);

const char *Kore_SystemId();

const char *Kore_Internal_SavePath();

const char **Kore_VideoFormats();

typedef uint64_t Kore_ticks;

double Kore_Frequency();
Kore_ticks Kore_Timestamp();
double Kore_Time();

void Kore_Start();
bool Kore_Internal_Frame();
void Kore_Stop();

void Kore_SetUpdateCallback(void (*value)());
void Kore_SetForegroundCallback(void (*value)());
void Kore_SetResumeCallback(void (*value)());
void Kore_SetPauseCallback(void (*value)());
void Kore_SetBackgroundCallback(void (*value)());
void Kore_SetShutdownCallback(void (*value)());
void Kore_SetDropFilesCallback(void (*value)(wchar_t *));
void Kore_SetCutCallback(char *(*value)());
void Kore_SetCopyCallback(char *(*value)());
void Kore_SetPasteCallback(void (*value)(char *));
void Kore_SetKeepScreenOn(bool on);

void Kore_Internal_Shutdown();
void Kore_Internal_UpdateCallback();
void Kore_Internal_ForegroundCallback();
void Kore_Internal_ResumeCallback();
void Kore_Internal_PauseCallback();
void Kore_Internal_BackgroundCallback();
void Kore_Internal_ShutdownCallback();
void Kore_Internal_DropFilesCallback(wchar_t *);
char *Kore_Internal_CutCallback();
char *Kore_Internal_CopyCallback();
void Kore_Internal_PasteCallback(char *);

#ifdef __cplusplus
}
#endif
