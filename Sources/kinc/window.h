#pragma once

#include <kinc/global.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_framebuffer_options {
	int frequency;
	bool vertical_sync;
	int color_bits;
	int depth_bits;
	int stencil_bits;
	int samples_per_pixel;
} kinc_framebuffer_options_t;

typedef enum {
	KINC_WINDOW_MODE_WINDOW,
	KINC_WINDOW_MODE_FULLSCREEN,
	KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN // Only relevant for Windows
} kinc_window_mode_t;

#define KINC_WINDOW_FEATURE_RESIZEABLE 1
#define KINC_WINDOW_FEATURE_MINIMIZABLE 2
#define KINC_WINDOW_FEATURE_MAXIMIZABLE 4
#define KINC_WINDOW_FEATURE_BORDERLESS 8
#define KINC_WINDOW_FEATURE_ON_TOP 16

typedef struct kinc_window_options {
	const char *title;

	int x;
	int y;
	int width;
	int height;
	int display_index;

	bool visible;
	int window_features;
	kinc_window_mode_t mode;
} kinc_window_options_t;

KINC_FUNC int kinc_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame);
KINC_FUNC void kinc_window_destroy(int window_index);
KINC_FUNC int kinc_count_windows(void);
KINC_FUNC void kinc_window_resize(int window_index, int width, int height);
KINC_FUNC void kinc_window_move(int window_index, int x, int y);
KINC_FUNC void kinc_window_change_mode(int window_index, kinc_window_mode_t mode);
KINC_FUNC void kinc_window_change_features(int window_index, int features);
KINC_FUNC void kinc_window_change_framebuffer(int window_index, kinc_framebuffer_options_t *frame);
KINC_FUNC int kinc_window_x(int window_index);
KINC_FUNC int kinc_window_y(int window_index);
KINC_FUNC int kinc_window_width(int window_index);
KINC_FUNC int kinc_window_height(int window_index);
KINC_FUNC int kinc_window_display(int window_index);
KINC_FUNC kinc_window_mode_t kinc_window_get_mode(int window_index);
KINC_FUNC void kinc_window_show(int window_index);
KINC_FUNC void kinc_window_hide(int window_index);
KINC_FUNC void kinc_window_set_title(int window_index, const char *title);
KINC_FUNC void kinc_window_set_resize_callback(int window_index, void (*callback)(int x, int y, void *data), void *data);
KINC_FUNC void kinc_window_set_ppi_changed_callback(int window_index, void (*callback)(int ppi, void *data), void *data);
KINC_FUNC bool kinc_window_vsynced(int window_index);

void kinc_internal_init_window_options(kinc_window_options_t *win);
void kinc_internal_init_framebuffer_options(kinc_framebuffer_options_t *frame);
void kinc_internal_call_resize_callback(int window_index, int width, int height);
void kinc_internal_call_ppi_changed_callback(int window_index, int ppi);

#ifdef __cplusplus
}
#endif
