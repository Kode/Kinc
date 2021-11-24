#include "wayland.h"
#include <kinc/log.h>

#ifdef KINC_EGL
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

static void xdg_surface_handle_configure(void *data, struct xdg_surface *surface, uint32_t serial) {
	struct kinc_wl_window *window = data;

	xdg_surface_ack_configure(surface, serial);
}

void kinc_internal_resize(int, int, int);

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *toplevel, int32_t width, int32_t height, struct wl_array *states) {
	struct kinc_wl_window *window = data;
	if (width <= 0 || height <= 0) {
		return;
	}
	window->width = width;
	window->height = height;
	enum xdg_toplevel_state *state;
	wl_array_for_each(state, states) {
		switch (*state) {
		case XDG_TOPLEVEL_STATE_ACTIVATED:
			kinc_internal_foreground_callback();
			break;
		case XDG_TOPLEVEL_STATE_RESIZING:
			break;
		case XDG_TOPLEVEL_STATE_MAXIMIZED:
			break;
		default:
			break;
		}
	}
	kinc_internal_resize(window->window_id, width, height);
	xdg_surface_set_window_geometry(window->xdg_surface, 0, 0, window->width, window->height);
#ifdef KINC_EGL
	wl_egl_window_resize(window->egl_window, width, height, 0, 0);
#endif
}

void kinc_wayland_window_destroy(int window_index);

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel) {
	struct kinc_wl_window *window = data;
	if (kinc_internal_call_close_callback(window->window_id)) {
		kinc_window_destroy(window->window_id);
		if (wl_ctx.num_windows <= 0) {
			// no windows left, stop
			kinc_stop();
		}
	}
}

void xdg_toplevel_decoration_configure(void *data, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1, uint32_t mode) {
	struct kinc_wl_window *window = data;
	if (mode & ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Client side decorations requested, but Kinc has not implemented those yet.");
	}
}
void wl_surface_handle_enter(void *data, struct wl_surface *wl_surface, struct wl_output *output) {
	struct kinc_wl_window *window = wl_surface_get_user_data(wl_surface);
	struct kinc_wl_display *display = wl_output_get_user_data(output);

	if (display && window) {
		window->display_index = display->index;
	}
}

void wl_surface_handle_leave(void *data, struct wl_surface *wl_surface, struct wl_output *output) {}

static const struct wl_surface_listener wl_surface_listener = {
    wl_surface_handle_enter,
    wl_surface_handle_leave,
};

static const struct xdg_surface_listener xdg_surface_listener = {
    xdg_surface_handle_configure,
};

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    xdg_toplevel_handle_configure,
    xdg_toplevel_handle_close,
};

static const struct zxdg_toplevel_decoration_v1_listener xdg_toplevel_decoration_listener = {
    xdg_toplevel_decoration_configure,
};

void kinc_wayland_window_set_title(int window_index, const char *title);

int kinc_wayland_window_create(kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	int window_index = -1;
	for (int i = 0; i < MAXIMUM_WINDOWS; i++) {
		if (wl_ctx.windows[i].surface == NULL) {
			window_index = i;
			break;
		}
	}
	if (window_index == -1) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Too much windows (maximum is %i)", MAXIMUM_WINDOWS);
		exit(1);
	}
	struct kinc_wl_window *window = &wl_ctx.windows[window_index];
	window->window_id = window_index;
	window->width = win->width;
	window->height = win->height;
	window->mode = win->mode;
	window->surface = wl_compositor_create_surface(wl_ctx.compositor);
	wl_surface_set_user_data(window->surface, window);
	wl_surface_add_listener(window->surface, &wl_surface_listener, NULL);

	window->xdg_surface = xdg_wm_base_get_xdg_surface(wl_ctx.xdg_wm_base, window->surface);
	xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);

	window->toplevel = xdg_surface_get_toplevel(window->xdg_surface);
	xdg_toplevel_add_listener(window->toplevel, &xdg_toplevel_listener, window);

	kinc_wayland_window_set_title(window_index, win->title);

	xdg_surface_set_window_geometry(window->xdg_surface, win->x, win->y, win->width, win->height);

	if (wl_ctx.decoration_manager) {
		window->decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(wl_ctx.decoration_manager, window->toplevel);
		zxdg_toplevel_decoration_v1_add_listener(window->decoration, &xdg_toplevel_decoration_listener, window);
	}
	else {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Client side decorations requested, but Kinc has not implemented those yet.");
	}

#ifdef KINC_EGL
	window->egl_window = wl_egl_window_create(window->surface, window->width, window->height);
#endif
	wl_surface_commit(window->surface);
	wl_ctx.num_windows++;
	return window_index;
}

void kinc_wayland_window_destroy(int window_index) {
	struct kinc_wl_window *window = &wl_ctx.windows[window_index];
#ifdef KINC_EGL
	wl_egl_window_destroy(window->egl_window);
#endif
	if (window->decoration) {
		zxdg_toplevel_decoration_v1_destroy(window->decoration);
	}

	xdg_toplevel_destroy(window->toplevel);
	xdg_surface_destroy(window->xdg_surface);
	wl_surface_destroy(window->surface);
	*window = (struct kinc_wl_window){0};
	wl_ctx.num_windows--;
}

void kinc_wayland_window_set_title(int window_index, const char *title) {
	struct kinc_wl_window *window = &wl_ctx.windows[window_index];
	xdg_toplevel_set_title(window->toplevel, title == NULL ? "" : title);
}

int kinc_wayland_window_x(int window_index) {
	kinc_log(KINC_LOG_LEVEL_ERROR, "Wayland does not support getting the window position.");
	return 0;
}

int kinc_wayland_window_y(int window_index) {
	kinc_log(KINC_LOG_LEVEL_ERROR, "Wayland does not support getting the window position.");
	return 0;
}

void kinc_wayland_window_move(int window_index, int x, int y) {
	kinc_log(KINC_LOG_LEVEL_ERROR, "Wayland does not support setting the window position.");
}

int kinc_wayland_window_width(int window_index) {
	return wl_ctx.windows[window_index].width;
}

int kinc_wayland_window_height(int window_index) {
	return wl_ctx.windows[window_index].height;
}

void kinc_wayland_window_resize(int window_index, int width, int height) {
	struct kinc_wl_window *window = &wl_ctx.windows[window_index];
	kinc_log(KINC_LOG_LEVEL_WARNING, "TODO: resizing windows");
}

void kinc_wayland_window_show(int window_index) {
	// TODO: figure out what exactly this method is supposed to do
}

void kinc_wayland_window_hide(int window_index) {
	// TODO: figure out what exactly this method is supposed to do
}

kinc_window_mode_t kinc_wayland_window_get_mode(int window_index) {
	return wl_ctx.windows[window_index].mode;
}

void kinc_wayland_window_change_mode(int window_index, kinc_window_mode_t mode) {
	struct kinc_wl_window *window = &wl_ctx.windows[window_index];
	if (mode == window->mode) {
		return;
	}
	switch (mode) {
	case KINC_WINDOW_MODE_WINDOW:
		if (window->mode == KINC_WINDOW_MODE_FULLSCREEN) {
			window->mode = KINC_WINDOW_MODE_WINDOW;
			xdg_toplevel_unset_fullscreen(window->toplevel);
		}
		break;
	case KINC_WINDOW_MODE_FULLSCREEN:
	case KINC_WINDOW_MODE_EXCLUSIVE_FULLSCREEN:
		break;
	}
}

int kinc_wayland_window_display(int window_index) {
	struct kinc_wl_window *window = &wl_ctx.windows[window_index];
	return window->display_index;
}

int kinc_wayland_count_windows() {
	return wl_ctx.num_windows;
}