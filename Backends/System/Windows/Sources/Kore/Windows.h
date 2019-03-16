#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HMONITOR__;
struct HWND__;

void Kore_Windows_InitDisplays();
int Kore_Windows_GetDisplayForMonitor(struct HMONITOR__ *monitor);
bool Kore_Windows_SetDisplayMode(int display_index, int width, int height, int bpp, int frequency);
void Kore_Windows_RestoreDisplay(int display_index);
void Kore_Windows_RestoreDisplays();
void Kore_Windows_HideWindows();
void Kore_Windows_DestroyWindows();
struct HWND__ *Kore_Windows_WindowHandle(int window_index);
int Kore_Windows_WindowIndexFromHWND(struct HWND__ *handle);

#ifdef __cplusplus
}
#endif
