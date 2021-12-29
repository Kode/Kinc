#include "wayland.h"
#include "xdg-decoration.h"
#include <kinc/image.h>
#include <kinc/log.h>
#include <kinc/memory.h>
#include <kinc/string.h>
// for all that shared memory stuff later on
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

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
	wl_subsurface_set_position(window->decorations.top.subsurface, KINC_WL_DECORATION_TOP_X, KINC_WL_DECORATION_TOP_Y);
	wp_viewport_set_destination(window->decorations.top.viewport, KINC_WL_DECORATION_TOP_WIDTH, KINC_WL_DECORATION_TOP_HEIGHT);
	wl_surface_commit(window->decorations.top.surface);
	wl_subsurface_set_position(window->decorations.left.subsurface, KINC_WL_DECORATION_LEFT_X, KINC_WL_DECORATION_LEFT_Y);
	wp_viewport_set_destination(window->decorations.left.viewport, KINC_WL_DECORATION_LEFT_WIDTH, KINC_WL_DECORATION_LEFT_HEIGHT);
	wl_surface_commit(window->decorations.left.surface);
	wl_subsurface_set_position(window->decorations.right.subsurface, KINC_WL_DECORATION_RIGHT_X, KINC_WL_DECORATION_RIGHT_Y);
	wp_viewport_set_destination(window->decorations.right.viewport, KINC_WL_DECORATION_RIGHT_WIDTH, KINC_WL_DECORATION_RIGHT_HEIGHT);
	wl_surface_commit(window->decorations.right.surface);
	wl_subsurface_set_position(window->decorations.bottom.subsurface, KINC_WL_DECORATION_BOTTOM_X, KINC_WL_DECORATION_BOTTOM_Y);
	wp_viewport_set_destination(window->decorations.bottom.viewport, KINC_WL_DECORATION_BOTTOM_WIDTH, KINC_WL_DECORATION_BOTTOM_HEIGHT);
	wl_surface_commit(window->decorations.bottom.surface);
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

static int create_anon_file(off_t size) {
	static const char template[] = "/kinc-shared-XXXXXX";

	const char *path = getenv("XDG_RUNTIME_DIR");
	if (!path) {
		errno = ENOENT;
		return -1;
	}

	char *name = kinc_allocate(kinc_string_length(path) + sizeof(template));
	kinc_string_copy(name, path);
	kinc_string_append(name, template);

	int fd = mkstemp(name);
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	if (fd >= 0) unlink(name);

	kinc_free(name);
	if (fd < 0) return -1;

	int ret = ftruncate(fd, size);
	if (ret != 0) {
		close(fd);
		errno = ret;
		return -1;
	}
	return fd;
}

struct wl_buffer *kinc_wayland_create_shm_buffer(const kinc_image_t *image) {
	int stride = image->width * 4;
	int length = image->width * image->height * 4;

	const int fd = create_anon_file(length);
	if (fd < 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Wayland: Creating a buffer file for %d B failed: %s", length, strerror(errno));
		return NULL;
	}

	void *data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Wayland: mmap failed: %s", strerror(errno));
		close(fd);
		return NULL;
	}

	struct wl_shm_pool *pool = wl_shm_create_pool(wl_ctx.shm, fd, length);

	close(fd);
	kinc_memcpy(data, image->data, image->width * image->height * 4);
	//  unsigned char *source = (unsigned char *)image->data;
	// unsigned char *target = data;
	// for (int i = 0; i < image->width * image->height; i++, source += 4) {
	// 	unsigned int alpha = source[3];

	// 	*target++ = (unsigned char)((source[2] * alpha) / 255);
	// 	*target++ = (unsigned char)((source[1] * alpha) / 255);
	// 	*target++ = (unsigned char)((source[0] * alpha) / 255);
	// 	*target++ = (unsigned char)alpha;
	// }

	struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, image->width, image->height, stride, WL_SHM_FORMAT_ARGB8888);
	munmap(data, length);
	wl_shm_pool_destroy(pool);

	return buffer;
}

static char grey_data[] = {0xFF, 0xFF, 0x00, 0x00};

static kinc_image_t grey_image = {1, 1, 0, KINC_IMAGE_FORMAT_RGBA32, 0, KINC_IMAGE_COMPRESSION_NONE, grey_data, sizeof(grey_data)};

void kinc_wayland_create_decoration(struct kinc_wl_decoration *decoration, struct wl_surface *parent, struct wl_buffer *buffer, bool opaque, int x, int y,
                                    int width, int height) {
	decoration->surface = wl_compositor_create_surface(wl_ctx.compositor);
	decoration->subsurface = wl_subcompositor_get_subsurface(wl_ctx.subcompositor, decoration->surface, parent);
	wl_subsurface_set_position(decoration->subsurface, x, y);
	decoration->viewport = wp_viewporter_get_viewport(wl_ctx.viewporter, decoration->surface);
	wp_viewport_set_destination(decoration->viewport, width, height);
	if (buffer) wl_surface_attach(decoration->surface, buffer, 0, 0);

	if (opaque) {
		struct wl_region *region = wl_compositor_create_region(wl_ctx.compositor);
		wl_region_add(region, 0, 0, width, height);
		wl_surface_set_opaque_region(decoration->surface, region);
		wl_surface_commit(decoration->surface);
		wl_region_destroy(region);
	}
	else
		wl_surface_commit(decoration->surface);
}

void kinc_wayland_destroy_decoration(struct kinc_wl_decoration *decoration) {
	if (decoration->subsurface) wl_subsurface_destroy(decoration->subsurface);
	if (decoration->surface) wl_surface_destroy(decoration->surface);
	if (decoration->viewport) wp_viewport_destroy(decoration->viewport);
	decoration->surface = NULL;
	decoration->subsurface = NULL;
	decoration->viewport = NULL;
}

void kinc_wayland_destroy_decorations(struct kinc_wl_window *window) {
	kinc_wayland_destroy_decoration(&window->decorations.top);
	kinc_wayland_destroy_decoration(&window->decorations.left);
	kinc_wayland_destroy_decoration(&window->decorations.right);
	kinc_wayland_destroy_decoration(&window->decorations.bottom);
}

void kinc_wayland_create_decorations(struct kinc_wl_window *window) {
	if (!window->decorations.buffer) {
		window->decorations.buffer = kinc_wayland_create_shm_buffer(&grey_image);
	}
	kinc_wayland_create_decoration(&window->decorations.top, window->surface, window->decorations.buffer, true, KINC_WL_DECORATION_TOP_X,
	                               KINC_WL_DECORATION_TOP_Y, KINC_WL_DECORATION_TOP_WIDTH, KINC_WL_DECORATION_TOP_HEIGHT);
	kinc_wayland_create_decoration(&window->decorations.left, window->surface, window->decorations.buffer, true, KINC_WL_DECORATION_LEFT_X,
	                               KINC_WL_DECORATION_LEFT_Y, KINC_WL_DECORATION_LEFT_WIDTH, KINC_WL_DECORATION_LEFT_HEIGHT);
	kinc_wayland_create_decoration(&window->decorations.right, window->surface, window->decorations.buffer, true, KINC_WL_DECORATION_RIGHT_X,
	                               KINC_WL_DECORATION_RIGHT_Y, KINC_WL_DECORATION_RIGHT_WIDTH, KINC_WL_DECORATION_RIGHT_HEIGHT);
	kinc_wayland_create_decoration(&window->decorations.bottom, window->surface, window->decorations.buffer, true, KINC_WL_DECORATION_BOTTOM_X,
	                               KINC_WL_DECORATION_BOTTOM_Y, KINC_WL_DECORATION_BOTTOM_WIDTH, KINC_WL_DECORATION_BOTTOM_HEIGHT);
}

void xdg_toplevel_decoration_configure(void *data, struct zxdg_toplevel_decoration_v1 *zxdg_toplevel_decoration_v1, uint32_t mode) {
	struct kinc_wl_window *window = data;

	if (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE) {
		window->decorations.server_side = false;
		if (window->decorations.top.surface) {
			kinc_wayland_destroy_decorations(window);
		}
		kinc_wayland_create_decorations(window);
	}
	else if (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE) {
		window->decorations.server_side = true;
		if (window->decorations.top.surface) {
			kinc_wayland_destroy_decorations(window);
		}
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
		window->xdg_decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(wl_ctx.decoration_manager, window->toplevel);
#ifdef KINC_WAYLAND_FORCE_CSD
		zxdg_toplevel_decoration_v1_set_mode(window->xdg_decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
#endif
		zxdg_toplevel_decoration_v1_add_listener(window->xdg_decoration, &xdg_toplevel_decoration_listener, window);
	}
	else {
		window->decorations.server_side = false;
		kinc_wayland_create_decorations(window);
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
	if (window->xdg_decoration) {
		zxdg_toplevel_decoration_v1_destroy(window->xdg_decoration);
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