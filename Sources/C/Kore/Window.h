#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Kore_FramebufferOptions {
	int frequency;
	bool vertical_sync;
	int color_bits;
	int depth_bits;
	int stencil_bits;
	int samples_per_pixel;
} Kore_FramebufferOptions;

typedef enum {
	WINDOW_MODE_WINDOW,
	WINDOW_MODE_FULLSCREEN,
	WINDOW_MODE_EXCLUSIVE_FULLSCREEN // Only relevant for Windows
} Kore_WindowMode;

#define KORE_WINDOW_FEATURE_RESIZEABLE 1
#define KORE_WINDOW_FEATURE_MINIMIZABLE 2
#define KORE_WINDOW_FEATURE_MAXIMIZABLE 4
#define KORE_WINDOW_FEATURE_BORDERLESS 8
#define KORE_WINDOW_FEATURE_ON_TOP 16

typedef struct {
	const char *title;

	int x;
	int y;
	int width;
	int height;
	int display_index;

	bool visible;
	int window_features;
	Kore_WindowMode mode;
} Kore_WindowOptions;

int Kore_WindowCreate(Kore_WindowOptions *win, Kore_FramebufferOptions *frame);
void Kore_WindowDestroy(int window_index);
int Kore_CountWindows();
void Kore_WindowResize(int window_index, int width, int height);
void Kore_WindowMove(int window_index, int x, int y);
void Kore_WindowChangeMode(int window_index, Kore_WindowMode mode);
void Kore_WindowChangeFeatures(int window_index, int features);
void Kore_WindowChangeFramebuffer(int window_index, Kore_FramebufferOptions *frame);
int Kore_WindowX(int window_index);
int Kore_WindowY(int window_index);
int Kore_WindowWidth(int window_index);
int Kore_WindowHeight(int window_index);
int Kore_WindowDisplay(int window_index);
Kore_WindowMode Kore_WindowGetMode(int window_index);
void Kore_WindowShow(int window_index);
void Kore_WindowHide(int window_index);
void Kore_WindowSetTitle(int window_index, const char *title);
void Kore_WindowSetResizeCallback(int window_index, void (*callback)(int x, int y, void *data), void *data);
void Kore_WindowSetPpiChangedCallback(int window_index, void (*callback)(int ppi, void *data), void *data);
bool Kore_WindowVSynced(int window_index);

void Kore_Internal_InitWindowOptions(Kore_WindowOptions *win);
void Kore_Internal_InitFramebufferOptions(Kore_FramebufferOptions *frame);
void Kore_Internal_CallResizeCallback(int window_index, int width, int height);
void Kore_Internal_CallPpiChangedCallback(int window_index, int ppi);

#ifdef __cplusplus
}
#endif
