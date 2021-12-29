#include "wayland.h"
#include <kinc/display.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/log.h>
#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>
#ifdef KINC_EGL
#include <EGL/egl.h>
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct kinc_wl_procs wl = {0};
struct kinc_xkb_procs wl_xkb = {0};

bool kinc_wayland_load_procs() {
	void *wayland_client = dlopen("libwayland-client.so", RTLD_LAZY);
	if (wayland_client == NULL) {
		return false;
	}
	bool has_missing_symbol = false;
#undef LOAD_FUN
#define LOAD_FUN(lib, symbol, name)                                                                                                                            \
	wl.symbol = dlsym(lib, name);                                                                                                                              \
	if (wl.symbol == NULL) {                                                                                                                                   \
		has_missing_symbol = true;                                                                                                                             \
		kinc_log(KINC_LOG_LEVEL_ERROR, "Did not find symbol %s.", name);                                                                                       \
	}
	LOAD_FUN(wayland_client, _wl_event_queue_destroy, "wl_event_queue_destroy")
	LOAD_FUN(wayland_client, _wl_proxy_marshal_flags, "wl_proxy_marshal_flags")
	LOAD_FUN(wayland_client, _wl_proxy_marshal_array_flags, "wl_proxy_marshal_array_flags")
	LOAD_FUN(wayland_client, _wl_proxy_marshal, "wl_proxy_marshal")
	LOAD_FUN(wayland_client, _wl_proxy_marshal_array, "wl_proxy_marshal_array")
	LOAD_FUN(wayland_client, _wl_proxy_create, "wl_proxy_create")
	LOAD_FUN(wayland_client, _wl_proxy_create_wrapper, "wl_proxy_create_wrapper")
	LOAD_FUN(wayland_client, _wl_proxy_wrapper_destroy, "wl_proxy_wrapper_destroy")
	LOAD_FUN(wayland_client, _wl_proxy_marshal_constructor, "wl_proxy_marshal_constructor")
	LOAD_FUN(wayland_client, _wl_proxy_marshal_constructor_versioned, "wl_proxy_marshal_constructor_versioned")
	LOAD_FUN(wayland_client, _wl_proxy_marshal_array_constructor, "wl_proxy_marshal_array_constructor")
	LOAD_FUN(wayland_client, _wl_proxy_marshal_array_constructor_versioned, "wl_proxy_marshal_array_constructor_versioned")
	LOAD_FUN(wayland_client, _wl_proxy_destroy, "wl_proxy_destroy")
	LOAD_FUN(wayland_client, _wl_proxy_add_listener, "wl_proxy_add_listener")
	LOAD_FUN(wayland_client, _wl_proxy_get_listener, "wl_proxy_get_listener")
	LOAD_FUN(wayland_client, _wl_proxy_add_dispatcher, "wl_proxy_add_dispatcher")
	LOAD_FUN(wayland_client, _wl_proxy_set_user_data, "wl_proxy_set_user_data")
	LOAD_FUN(wayland_client, _wl_proxy_get_user_data, "wl_proxy_get_user_data")
	LOAD_FUN(wayland_client, _wl_proxy_get_version, "wl_proxy_get_version")
	LOAD_FUN(wayland_client, _wl_proxy_get_id, "wl_proxy_get_id")
	LOAD_FUN(wayland_client, _wl_proxy_set_tag, "wl_proxy_set_tag")
	LOAD_FUN(wayland_client, _wl_proxy_get_tag, "wl_proxy_get_tag")
	LOAD_FUN(wayland_client, _wl_proxy_get_class, "wl_proxy_get_class")
	LOAD_FUN(wayland_client, _wl_proxy_set_queue, "wl_proxy_set_queue")
	LOAD_FUN(wayland_client, _wl_display_connect, "wl_display_connect")
	LOAD_FUN(wayland_client, _wl_display_connect_to_fd, "wl_display_connect_to_fd")
	LOAD_FUN(wayland_client, _wl_display_disconnect, "wl_display_disconnect")
	LOAD_FUN(wayland_client, _wl_display_get_fd, "wl_display_get_fd")
	LOAD_FUN(wayland_client, _wl_display_dispatch, "wl_display_dispatch")
	LOAD_FUN(wayland_client, _wl_display_dispatch_queue, "wl_display_dispatch_queue")
	LOAD_FUN(wayland_client, _wl_display_dispatch_queue_pending, "wl_display_dispatch_queue_pending")
	LOAD_FUN(wayland_client, _wl_display_dispatch_pending, "wl_display_dispatch_pending")
	LOAD_FUN(wayland_client, _wl_display_get_error, "wl_display_get_error")
	LOAD_FUN(wayland_client, _wl_display_get_protocol_error, "wl_display_get_protocol_error")
	LOAD_FUN(wayland_client, _wl_display_flush, "wl_display_flush")
	LOAD_FUN(wayland_client, _wl_display_roundtrip_queue, "wl_display_roundtrip_queue")
	LOAD_FUN(wayland_client, _wl_display_roundtrip, "wl_display_roundtrip")
	LOAD_FUN(wayland_client, _wl_display_create_queue, "wl_display_create_queue")
	LOAD_FUN(wayland_client, _wl_display_prepare_read_queue, "wl_display_prepare_read_queue")
	LOAD_FUN(wayland_client, _wl_display_prepare_read, "wl_display_prepare_read")
	LOAD_FUN(wayland_client, _wl_display_cancel_read, "wl_display_cancel_read")
	LOAD_FUN(wayland_client, _wl_display_read_events, "wl_display_read_events")
	LOAD_FUN(wayland_client, _wl_log_set_handler_client, "wl_log_set_handler_client")

	void *wayland_cursor = dlopen("libwayland-cursor.so", RTLD_LAZY);
	if (wayland_cursor == NULL) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to find libwayland-cursor.so");
		return false;
	}
	LOAD_FUN(wayland_cursor, _wl_cursor_theme_load, "wl_cursor_theme_load")
	LOAD_FUN(wayland_cursor, _wl_cursor_theme_destroy, "wl_cursor_theme_destroy")
	LOAD_FUN(wayland_cursor, _wl_cursor_theme_get_cursor, "wl_cursor_theme_get_cursor")
	LOAD_FUN(wayland_cursor, _wl_cursor_image_get_buffer, "wl_cursor_image_get_buffer")
	LOAD_FUN(wayland_cursor, _wl_cursor_frame, "wl_cursor_frame")
	LOAD_FUN(wayland_cursor, _wl_cursor_frame_and_duration, "wl_cursor_frame_and_duration")

#ifdef KINC_EGL
	void *wayland_egl = dlopen("libwayland-egl.so", RTLD_LAZY);
	if (wayland_egl == NULL) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to find libwayland-egl.so");
		return false;
	}
	LOAD_FUN(wayland_egl, _wl_egl_window_create, "wl_egl_window_create")
	LOAD_FUN(wayland_egl, _wl_egl_window_destroy, "wl_egl_window_destroy")
	LOAD_FUN(wayland_egl, _wl_egl_window_resize, "wl_egl_window_resize")
	LOAD_FUN(wayland_egl, _wl_egl_window_get_attached_size, "wl_egl_window_get_attached_size")
#endif

#undef LOAD_FUN
#define LOAD_FUN(symbol)                                                                                                                                       \
	wl_xkb.symbol = dlsym(xkb, #symbol);                                                                                                                       \
	if (wl_xkb.symbol == NULL) {                                                                                                                               \
		has_missing_symbol = true;                                                                                                                             \
		kinc_log(KINC_LOG_LEVEL_ERROR, "Did not find symbol %s.", #symbol);                                                                                    \
	}
	void *xkb = dlopen("libxkbcommon.so", RTLD_LAZY);
	if (xkb == NULL) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Failed to find libxkb_common.so");
		return false;
	}
	LOAD_FUN(xkb_context_new)
	LOAD_FUN(xkb_context_unref)
	LOAD_FUN(xkb_state_new)
	LOAD_FUN(xkb_keymap_new_from_string)
	LOAD_FUN(xkb_state_key_get_one_sym)
	LOAD_FUN(xkb_state_key_get_utf32)
	LOAD_FUN(xkb_state_serialize_mods)
	LOAD_FUN(xkb_state_update_mask)
#undef LOAD_FUN

	if (has_missing_symbol) {
		return false;
	}

	return true;
}

struct wayland_context wl_ctx = {0};

static void xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *shell, uint32_t serial) {
	xdg_wm_base_pong(shell, serial);
};

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    xdg_wm_base_handle_ping,
};

static void wl_output_handle_geometry(void *data, struct wl_output *wl_output, int x, int y, int physical_width, int physical_height, int subpixel,
                                      const char *make, const char *model, int transform) {
	struct kinc_wl_display *display = data;
	snprintf(display->name, sizeof(display->name), "%s %s", make, model);
	display->x = x;
	display->y = y;
	display->physical_width = physical_width;
	display->physical_height = physical_height;
	display->subpixel = subpixel;
	display->transform = transform;
}

static void wl_output_handle_mode(void *data, struct wl_output *wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
	struct kinc_wl_display *display = data;
	if (display->num_modes < MAXIMUM_DISPLAY_MODES) {
		int mode_index = display->num_modes++;
		kinc_display_mode_t *mode = &display->modes[mode_index];
		mode->x = 0;
		mode->y = 0;
		mode->width = width;
		mode->height = height;
		mode->bits_per_pixel = 32;
		mode->pixels_per_inch = 96;
		mode->frequency = (refresh / 1000);
		if (flags & WL_OUTPUT_MODE_CURRENT) display->current_mode = mode_index;
	}
}
static void wl_output_handle_done(void *data, struct wl_output *wl_output) {
	struct kinc_wl_display *display = data;
}
static void wl_output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	struct kinc_wl_display *display = data;
	display->scale = factor;
}

static const struct wl_output_listener wl_output_listener = {
    wl_output_handle_geometry,
    wl_output_handle_mode,
    wl_output_handle_done,
    wl_output_handle_scale,
};

struct kinc_wl_window *kinc_wayland_window_from_surface(struct wl_surface *surface, enum kinc_wl_decoration_focus *focus) {
	struct kinc_wl_window *window = wl_surface_get_user_data(surface);
	if (window == NULL) {
		for (int i = 0; i < MAXIMUM_WINDOWS; i++) {
			struct kinc_wl_window *_window = &wl_ctx.windows[i];
			if (_window->surface == surface) {
				*focus = KINC_WL_DECORATION_FOCUS_MAIN;
				window = _window;
			}
			else if (surface == _window->decorations.top.surface) {
				*focus = KINC_WL_DECORATION_FOCUS_TOP;
				window = _window;
			}
			else if (surface == _window->decorations.left.surface) {
				*focus = KINC_WL_DECORATION_FOCUS_LEFT;
				window = _window;
			}
			else if (surface == _window->decorations.right.surface) {
				*focus = KINC_WL_DECORATION_FOCUS_RIGHT;
				window = _window;
			}
			else if (surface == _window->decorations.bottom.surface) {
				*focus = KINC_WL_DECORATION_FOCUS_BOTTOM;
				window = _window;
			}
		}
	}
	return window;
}

void wl_pointer_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x,
                             wl_fixed_t surface_y) {
	enum kinc_wl_decoration_focus focus = KINC_WL_DECORATION_FOCUS_MAIN;
	struct kinc_wl_window *window = kinc_wayland_window_from_surface(surface, &focus);
	struct kinc_wl_mouse *mouse = data;
	mouse->enter_serial = serial;
	window->decorations.focus = focus;
	if (window != NULL) {
		mouse->current_window = window->window_id;
		kinc_internal_mouse_trigger_enter_window(window->window_id);
	}
}

void wl_pointer_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface) {
	enum kinc_wl_decoration_focus focus = KINC_WL_DECORATION_FOCUS_MAIN;
	struct kinc_wl_window *window = kinc_wayland_window_from_surface(surface, &focus);

	if (window != NULL) {
		kinc_internal_mouse_trigger_leave_window(window->window_id);
	}
}

#include <wayland-cursor.h>

void kinc_wayland_set_cursor(struct kinc_wl_mouse *mouse, const char *name) {
	if(!name) return;
	struct wl_cursor *cursor = wl_cursor_theme_get_cursor(wl_ctx.cursor_theme, name);
	if (!cursor) return;
	struct wl_cursor_image *image = cursor->images[0];
	if (!image) return;
	struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
	if (!buffer) return;

	wl_pointer_set_cursor(mouse->pointer, mouse->enter_serial, mouse->surface, image->hotspot_x, image->hotspot_y);
	wl_surface_attach(mouse->surface, buffer, 0, 0);
	wl_surface_damage(mouse->surface, 0, 0, image->width, image->height);
	wl_surface_commit(mouse->surface);
	mouse->previous_cursor_name = name;
}

void wl_pointer_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	struct kinc_wl_mouse *mouse = data;
	struct kinc_wl_window *window = &wl_ctx.windows[mouse->current_window];

	int x = wl_fixed_to_int(surface_x);
	int y = wl_fixed_to_int(surface_y);

	const char *cursor_name = NULL;

	switch (window->decorations.focus) {
	case KINC_WL_DECORATION_FOCUS_MAIN:
		mouse->x = x;
		mouse->y = y;
		kinc_internal_mouse_trigger_move(mouse->current_window, x, y);
		return;
	case KINC_WL_DECORATION_FOCUS_TOP:
		if (y < 10)
			cursor_name = "n-resize";
		else
			cursor_name = "left_ptr";
		break;
	case KINC_WL_DECORATION_FOCUS_LEFT:
		if (y < 10)
			cursor_name = "nw-resize";
		else
			cursor_name = "w-resize";
		break;
	case KINC_WL_DECORATION_FOCUS_RIGHT:
		if (y < 10)
			cursor_name = "ne-resize";
		else
			cursor_name = "e-resize";
		break;
	case KINC_WL_DECORATION_FOCUS_BOTTOM:
		if (x < 10)
			cursor_name = "sw-resize";
		else if (x > window->width + 10)
			cursor_name = "se-resize";
		else
			cursor_name = "s-resize";
		break;
	}

	if(mouse->previous_cursor_name != cursor_name) {
		kinc_wayland_set_cursor(mouse, cursor_name);
	}
}

#include <linux/input-event-codes.h>

void wl_pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	struct kinc_wl_mouse *mouse = data;
	struct kinc_wl_window *window = &wl_ctx.windows[mouse->current_window];
	int kinc_button = 0;
	switch (button) {
	case BTN_LEFT:
		kinc_button = 0;
		break;
	case BTN_RIGHT:
		kinc_button = 1;
		break;
	case BTN_MIDDLE:
		kinc_button = 2;
		break;
	default:
		break;
	}

	if (kinc_button == 0) {
		enum xdg_toplevel_resize_edge edges = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
		switch (window->decorations.focus) {
		case KINC_WL_DECORATION_FOCUS_MAIN:
			break;
		case KINC_WL_DECORATION_FOCUS_TOP:
			if (mouse->y < 10)
				edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
			else {
				xdg_toplevel_move(window->toplevel, wl_ctx.seat.seat, serial);
			}
			break;
		case KINC_WL_DECORATION_FOCUS_LEFT:
			if (mouse->y < 10)
				edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
			else
				edges = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
			break;
		case KINC_WL_DECORATION_FOCUS_RIGHT:
			if (mouse->y < 10)
				edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
			else
				edges = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
			break;
		case KINC_WL_DECORATION_FOCUS_BOTTOM:
			if (mouse->x < 10)
				edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
			else if (mouse->x > window->width + 10)
				edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
			else
				edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
			break;
		}
		if (edges != XDG_TOPLEVEL_RESIZE_EDGE_NONE) {
			xdg_toplevel_resize(window->toplevel, wl_ctx.seat.seat, serial, edges);
		}
	}

	if (window->decorations.focus == KINC_WL_DECORATION_FOCUS_MAIN) {
		if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
			kinc_internal_mouse_trigger_press(mouse->current_window, kinc_button, mouse->x, mouse->y);
		}
		if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
			kinc_internal_mouse_trigger_release(mouse->current_window, kinc_button, mouse->x, mouse->y);
		}
	}
}

void wl_pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
	struct kinc_wl_mouse *mouse = data;
	// FIXME: figure out what the other backends give as deltas
	int delta = wl_fixed_to_int(value);
	kinc_internal_mouse_trigger_scroll(mouse->current_window, delta);
}

static const struct wl_pointer_listener wl_pointer_listener = {
    wl_pointer_handle_enter, wl_pointer_handle_leave, wl_pointer_handle_motion, wl_pointer_handle_button, wl_pointer_handle_axis, 0, 0, 0, 0,
};

#include <sys/mman.h>
#include <unistd.h>

void wl_keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
	struct kinc_wl_keyboard *keyboard = data;
	switch (format) {
	case WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1: {
		char *mapStr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
		if (mapStr == MAP_FAILED) {
			close(fd);
			return;
		}
		keyboard->keymap = wl_xkb.xkb_keymap_new_from_string(wl_ctx.xkb_context, mapStr, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
		munmap(mapStr, size);
		close(fd);
		keyboard->state = wl_xkb.xkb_state_new(keyboard->keymap);
		break;
	}
	default:
		close(fd);
		kinc_log(KINC_LOG_LEVEL_WARNING, "Unsupported wayland keymap format %i", format);
	}
}
void wl_keyboard_handle_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {}
void wl_keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {}

int xkb_to_kinc(xkb_keysym_t symbol);

void wl_keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	struct kinc_wl_keyboard *keyboard = data;
	if (keyboard->keymap && keyboard->state) {
		xkb_keysym_t symbol = wl_xkb.xkb_state_key_get_one_sym(keyboard->state, key + 8);
		uint32_t character = wl_xkb.xkb_state_key_get_utf32(keyboard->state, key + 8);
		int kinc_key = xkb_to_kinc(symbol);
		if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
			kinc_internal_keyboard_trigger_key_down(kinc_key);
			kinc_internal_keyboard_trigger_key_press(character);
		}
		if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
			kinc_internal_keyboard_trigger_key_up(kinc_key);
		}
	}
}
void wl_keyboard_handle_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
                                  uint32_t mods_locked, uint32_t group) {
	struct kinc_wl_keyboard *keyboard = data;
	if (keyboard->keymap && keyboard->state) {
		wl_xkb.xkb_state_update_mask(keyboard->state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
		int mask = wl_xkb.xkb_state_serialize_mods(keyboard->state,
		                                           XKB_STATE_MODS_DEPRESSED | XKB_STATE_LAYOUT_DEPRESSED | XKB_STATE_MODS_LATCHED | XKB_STATE_LAYOUT_LATCHED);
	}
}
void wl_keyboard_handle_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {}

static const struct wl_keyboard_listener wl_keyboard_listener = {
    wl_keyboard_handle_keymap,      wl_keyboard_handle_enter, wl_keyboard_handle_leave, wl_keyboard_handle_key, wl_keyboard_handle_modifiers,
#ifdef WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION
    wl_keyboard_handle_repeat_info,
#endif
};

void wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
	struct kinc_wl_seat *seat = data;
	seat->capabilities = capabilities;
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
		seat->keyboard.keyboard = wl_seat_get_keyboard(wl_seat);
		wl_keyboard_add_listener(seat->keyboard.keyboard, &wl_keyboard_listener, &seat->keyboard);
	}
	if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
		seat->mouse.pointer = wl_seat_get_pointer(wl_seat);
		seat->mouse.surface = wl_compositor_create_surface(wl_ctx.compositor);
		wl_pointer_add_listener(seat->mouse.pointer, &wl_pointer_listener, &seat->mouse);
	}
	if (capabilities & WL_SEAT_CAPABILITY_TOUCH) {
		seat->touch = wl_seat_get_touch(wl_seat);
	}
}

void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name) {
	struct kinc_wl_seat *seat = data;
	snprintf(seat->name, sizeof(seat->name), "%s", name);
}

static const struct wl_seat_listener wl_seat_listener = {
    wl_seat_capabilities,
    wl_seat_name,
};

static void wl_registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		wl_ctx.compositor = wl_registry_bind(wl_ctx.registry, name, &wl_compositor_interface, 4);
	}
	else if (strcmp(interface, wl_shm_interface.name) == 0) {
		wl_ctx.shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	}
	else if (strcmp(interface, wl_subcompositor_interface.name) == 0) {
		wl_ctx.subcompositor = wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
	}
	else if (strcmp(interface, wp_viewporter_interface.name) == 0) {
		wl_ctx.viewporter = wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
	}
	else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		wl_ctx.xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(wl_ctx.xdg_wm_base, &xdg_wm_base_listener, NULL);
	}
	else if (strcmp(interface, wl_seat_interface.name) == 0) {
		if (wl_ctx.seat.seat) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Multi-seat configurations not supported");
			return;
		}
		wl_ctx.seat.seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);

		wl_seat_add_listener(wl_ctx.seat.seat, &wl_seat_listener, &wl_ctx.seat);
	}
	else if (strcmp(interface, wl_output_interface.name) == 0) {
		int display_index = -1;
		for (int i = 0; i < MAXIMUM_WINDOWS; i++) {
			if (wl_ctx.displays[i].output == NULL) {
				display_index = i;
				break;
			}
		}
		if (display_index == -1) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "Too much displays (maximum is %i)", MAXIMUM_DISPLAYS);
		}
		else {
			struct kinc_wl_display *display = &wl_ctx.displays[display_index];
			display->output = wl_registry_bind(registry, name, &wl_output_interface, 2);
			display->scale = 1;
			wl_output_set_user_data(display->output, display);
			wl_output_add_listener(display->output, &wl_output_listener, display);
			wl_ctx.num_displays++;
		}
	}
	else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
		wl_ctx.decoration_manager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
	}
}

static void wl_registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
	// TODO: handle output removal
}

static const struct wl_registry_listener registry_listener = {
    wl_registry_handle_global,
    wl_registry_handle_global_remove,
};

bool kinc_wayland_init() {
	if (!kinc_wayland_load_procs()) {
		return false;
	}

	wl_ctx.xkb_context = wl_xkb.xkb_context_new(XKB_CONTEXT_NO_FLAGS);

	wl_ctx.display = wl_display_connect(NULL);
	if (!wl_ctx.display) {
		return false;
	}
	wl_ctx.registry = wl_display_get_registry(wl_ctx.display);
	wl_registry_add_listener(wl_ctx.registry, &registry_listener, NULL);
	wl_display_dispatch(wl_ctx.display);
	wl_display_roundtrip(wl_ctx.display);
	wl_display_roundtrip(wl_ctx.display);

	if (wl_ctx.seat.mouse.pointer && wl_ctx.shm) {
		const char *cursor_theme = getenv("XCURSOR_THEME");
		const char *cursor_size_str = getenv("XCURSOR_SIZE");
		int cursor_size = 32;

		if (cursor_size_str) {
			char *end_ptr;
			long size = strtol(cursor_size_str, &end_ptr, 10);
			if (!(*end_ptr) && size > 0 && size < INT32_MAX) {
				cursor_size = (int)size;
			}
		}

		wl_ctx.cursor_theme = wl_cursor_theme_load(cursor_theme, cursor_size, wl_ctx.shm);
	}

	return true;
}

void kinc_wayland_shutdown() {
	wl_display_disconnect(wl_ctx.display);
	wl_xkb.xkb_context_unref(wl_ctx.xkb_context);
}

void kinc_wayland_copy_to_clipboard(const char *text) {}

bool kinc_wayland_handle_messages() {
	wl_display_dispatch(wl_ctx.display);
	while (wl_display_prepare_read(wl_ctx.display) != 0) wl_display_dispatch_pending(wl_ctx.display);
	wl_display_flush(wl_ctx.display);
	wl_display_read_events(wl_ctx.display);
	wl_display_dispatch_pending(wl_ctx.display);
	return false;
}

#ifdef KINC_EGL
EGLDisplay kinc_wayland_egl_get_display() {
	return eglGetDisplay(wl_ctx.display);
}

EGLNativeWindowType kinc_wayland_egl_get_native_window(int window_index) {
	return (EGLNativeWindowType)wl_ctx.windows[window_index].egl_window;
}
#endif

#ifdef KORE_VULKAN
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
VkResult kinc_wayland_vulkan_create_surface(VkInstance instance, int window_index, VkSurfaceKHR *surface) {
	VkWaylandSurfaceCreateInfoKHR info = {0};
	info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	info.pNext = NULL;
	info.flags = 0;
	info.display = wl_ctx.display;
	info.surface = wl_ctx.windows[window_index].surface;
	return vkCreateWaylandSurfaceKHR(instance, &info, NULL, surface);
}

#include <assert.h>

void kinc_wayland_vulkan_get_instance_extensions(const char **names, int *index, int max) {
	assert(*index + 1 < max);
	names[(*index)++] = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
}

VkBool32 kinc_wayland_vulkan_get_physical_device_presentation_support(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) {
	return vkGetPhysicalDeviceWaylandPresentationSupportKHR(physicalDevice, queueFamilyIndex, wl_ctx.display);
}
#undef VK_USE_PLATFORM_WAYLAND_KHR
#endif