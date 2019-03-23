#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HMONITOR__;
struct HWND__;

void Kinc_Windows_InitDisplays();
int Kinc_Windows_GetDisplayForMonitor(struct HMONITOR__ *monitor);
bool Kinc_Windows_SetDisplayMode(int display_index, int width, int height, int bpp, int frequency);
void Kinc_Windows_RestoreDisplay(int display_index);
void Kinc_Windows_RestoreDisplays();
void Kinc_Windows_HideWindows();
void Kinc_Windows_DestroyWindows();
struct HWND__ *Kinc_Windows_WindowHandle(int window_index);
int Kinc_Windows_WindowIndexFromHWND(struct HWND__ *handle);

#ifdef __cplusplus
}
#endif
