#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct HMONITOR__;
struct HWND__;

void Kinc_Windows_InitDisplays();
int Kinc_Windows_GetDisplayForMonitor(struct HMONITOR__ *monitor);
bool kinc_windows_set_display_mode(int display_index, int width, int height, int bpp, int frequency);
void kinc_windows_restore_display(int display_index);
void kinc_windows_restore_displays();
void Kinc_Windows_HideWindows();
void Kinc_Windows_DestroyWindows();
struct HWND__ *Kinc_Windows_WindowHandle(int window_index);
int Kinc_Windows_WindowIndexFromHWND(struct HWND__ *handle);
int Kinc_Windows_ManualWidth(int window);
int Kinc_Windows_ManualHeight(int window);

#ifdef __cplusplus
}
#endif
