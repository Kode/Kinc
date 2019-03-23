#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Kinc_FramebufferOptions {
	int frequency;
	bool vertical_sync;
	int color_bits;
	int depth_bits;
	int stencil_bits;
	int samples_per_pixel;
} Kinc_FramebufferOptions;

typedef enum {
	KINC_WINDOW_MODE_WINDOW,
	KINC_WINDOW_MODE_FULLSCREEN,
	KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN // Only relevant for Windows
} Kinc_WindowMode;

#define KINC_WINDOW_FEATURE_RESIZEABLE 1
#define KINC_WINDOW_FEATURE_MINIMIZABLE 2
#define KINC_WINDOW_FEATURE_MAXIMIZABLE 4
#define KINC_WINDOW_FEATURE_BORDERLESS 8
#define KINC_WINDOW_FEATURE_ON_TOP 16

typedef struct _Kinc_WindowOptions {
	const char *title;

	int x;
	int y;
	int width;
	int height;
	int display_index;

	bool visible;
	int window_features;
	Kinc_WindowMode mode;
} Kinc_WindowOptions;

int Kinc_WindowCreate(Kinc_WindowOptions *win, Kinc_FramebufferOptions *frame);
void Kinc_WindowDestroy(int window_index);
int Kinc_CountWindows(void);
void Kinc_WindowResize(int window_index, int width, int height);
void Kinc_WindowMove(int window_index, int x, int y);
void Kinc_WindowChangeMode(int window_index, Kinc_WindowMode mode);
void Kinc_WindowChangeFeatures(int window_index, int features);
void Kinc_WindowChangeFramebuffer(int window_index, Kinc_FramebufferOptions *frame);
int Kinc_WindowX(int window_index);
int Kinc_WindowY(int window_index);
int Kinc_WindowWidth(int window_index);
int Kinc_WindowHeight(int window_index);
int Kinc_WindowDisplay(int window_index);
Kinc_WindowMode Kinc_WindowGetMode(int window_index);
void Kinc_WindowShow(int window_index);
void Kinc_WindowHide(int window_index);
void Kinc_WindowSetTitle(int window_index, const char *title);
void Kinc_WindowSetResizeCallback(int window_index, void (*callback)(int x, int y, void *data), void *data);
void Kinc_WindowSetPpiChangedCallback(int window_index, void (*callback)(int ppi, void *data), void *data);
bool Kinc_WindowVSynced(int window_index);

void Kinc_Internal_InitWindowOptions(Kinc_WindowOptions *win);
void Kinc_Internal_InitFramebufferOptions(Kinc_FramebufferOptions *frame);
void Kinc_Internal_CallResizeCallback(int window_index, int width, int height);
void Kinc_Internal_CallPpiChangedCallback(int window_index, int ppi);

#ifdef __cplusplus
}
#endif
