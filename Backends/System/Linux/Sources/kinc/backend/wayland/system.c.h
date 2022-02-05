#include "kinc/backend/wayland/xdg-shell.h"
#include "wayland.h"

#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>

#ifdef KINC_EGL
#include <EGL/egl.h>
#endif
#include <dlfcn.h>
#include <stdlib.h>

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
	LOAD_FUN(xkb_state_mod_name_is_active)
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
	if (!name) return;
	struct wl_cursor *cursor = wl_cursor_theme_get_cursor(wl_ctx.cursor_theme, name);
	if (!cursor) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Wayland: No cursor found '%'.", name);
		return;
	}
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

	const char *cursor_name = "default";

	switch (window->decorations.focus) {
	case KINC_WL_DECORATION_FOCUS_MAIN:
		mouse->x = x;
		mouse->y = y;
		kinc_internal_mouse_trigger_move(mouse->current_window, x, y);
		break;
	case KINC_WL_DECORATION_FOCUS_TOP:
		if (y < KINC_WL_DECORATION_WIDTH)
			cursor_name = "n-resize";
		else
			cursor_name = "move";
		break;
	case KINC_WL_DECORATION_FOCUS_LEFT:
		if (y < KINC_WL_DECORATION_WIDTH)
			cursor_name = "nw-resize";
		else if (mouse->y > KINC_WL_DECORATION_TOP_HEIGHT - KINC_WL_DECORATION_WIDTH)
			cursor_name = "sw-resize";
		else
			cursor_name = "w-resize";
		break;
	case KINC_WL_DECORATION_FOCUS_RIGHT:
		if (y < KINC_WL_DECORATION_WIDTH)
			cursor_name = "ne-resize";
		else if (mouse->y > KINC_WL_DECORATION_RIGHT_HEIGHT - KINC_WL_DECORATION_WIDTH)
			cursor_name = "se-resize";
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

	if (mouse->previous_cursor_name != cursor_name) {
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
			if (mouse->y > KINC_WL_DECORATION_WIDTH)
				edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
			else {
				xdg_toplevel_move(window->toplevel, wl_ctx.seat.seat, serial);
			}
			break;
		case KINC_WL_DECORATION_FOCUS_LEFT:
			if (mouse->y < KINC_WL_DECORATION_WIDTH)
				edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
			else if (mouse->y > KINC_WL_DECORATION_TOP_HEIGHT - KINC_WL_DECORATION_WIDTH)
				edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
			else
				edges = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
			break;
		case KINC_WL_DECORATION_FOCUS_RIGHT:
			if (mouse->y < KINC_WL_DECORATION_WIDTH)
				edges = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
			else if (mouse->y > KINC_WL_DECORATION_RIGHT_HEIGHT - KINC_WL_DECORATION_WIDTH)
				edges = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
			else
				edges = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
			break;
		case KINC_WL_DECORATION_FOCUS_BOTTOM:
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
	struct kinc_wl_seat *seat = data;
	struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
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
		keyboard->ctrlDown = false;
		break;
	}
	default:
		close(fd);
		kinc_log(KINC_LOG_LEVEL_WARNING, "Unsupported wayland keymap format %i", format);
	}
}

void wl_keyboard_handle_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
	struct kinc_wl_seat *seat = data;
	struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
}

void wl_keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {
	struct kinc_wl_seat *seat = data;
	struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
}

int xkb_to_kinc(xkb_keysym_t symbol);

void handle_paste(void *data, size_t data_size, void *user_data) {
	kinc_internal_paste_callback(data);
}

void wl_keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
	if (keyboard->keymap && keyboard->state) {
		xkb_keysym_t symbol = wl_xkb.xkb_state_key_get_one_sym(keyboard->state, key + 8);
		uint32_t character = wl_xkb.xkb_state_key_get_utf32(keyboard->state, key + 8);
		int kinc_key = xkb_to_kinc(symbol);
		if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
			if (keyboard->ctrlDown && (symbol == XKB_KEY_c || symbol == XKB_KEY_C)) {
				char *text = kinc_internal_copy_callback();
				if (text != NULL) {
					kinc_wayland_set_selection(keyboard->seat, text, serial);
				}
			}
			else if (keyboard->ctrlDown && (symbol == XKB_KEY_x || symbol == XKB_KEY_X)) {
				char *text = kinc_internal_copy_callback();
				if (text != NULL) {
					kinc_wayland_set_selection(keyboard->seat, text, serial);
				}
			}
			else if (keyboard->ctrlDown && (symbol == XKB_KEY_v || symbol == XKB_KEY_V)) {
				if (keyboard->seat->current_selection_offer != NULL) {
					kinc_wl_data_offer_accept(keyboard->seat->current_selection_offer, handle_paste, NULL);
				}
			}
			kinc_internal_keyboard_trigger_key_down(kinc_key);
			if (character != 0) {
				kinc_internal_keyboard_trigger_key_press(character);
			}
		}
		if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
			kinc_internal_keyboard_trigger_key_up(kinc_key);
		}
	}
}

void wl_keyboard_handle_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
                                  uint32_t mods_locked, uint32_t group) {
	struct kinc_wl_keyboard *keyboard = wl_keyboard_get_user_data(wl_keyboard);
	if (keyboard->keymap && keyboard->state) {
		wl_xkb.xkb_state_update_mask(keyboard->state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
		int mask = wl_xkb.xkb_state_serialize_mods(keyboard->state,
		                                           XKB_STATE_MODS_DEPRESSED | XKB_STATE_LAYOUT_DEPRESSED | XKB_STATE_MODS_LATCHED | XKB_STATE_LAYOUT_LATCHED);
		keyboard->ctrlDown = wl_xkb.xkb_state_mod_name_is_active(keyboard->state, XKB_MOD_NAME_CTRL, XKB_STATE_MODS_EFFECTIVE) > 0;
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
		seat->keyboard.seat = seat;
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

/**
 * a target accepts an offered mime type
 *
 * Sent when a target accepts pointer_focus or motion events. If
 * a target does not accept any of the offered types, type is NULL.
 *
 * Used for feedback during drag-and-drop.
 * @param mime_type mime type accepted by the target
 */
void wl_data_source_handle_target(void *data, struct wl_data_source *wl_data_source, const char *mime_type) {
	struct kinc_wl_data_source *data_source = wl_data_source_get_user_data(wl_data_source);
}
/**
 * send the data
 *
 * Request for data from the client. Send the data as the
 * specified mime type over the passed file descriptor, then close
 * it.
 * @param mime_type mime type for the data
 * @param fd file descriptor for the data
 */
void wl_data_source_handle_send(void *data, struct wl_data_source *wl_data_source, const char *mime_type, int32_t fd) {
	struct kinc_wl_data_source *data_source = wl_data_source_get_user_data(wl_data_source);
	write(fd, data_source->data, data_source->data_size);
	close(fd);
}
/**
 * selection was cancelled
 *
 * This data source is no longer valid. There are several reasons
 * why this could happen:
 *
 * - The data source has been replaced by another data source. -
 * The drag-and-drop operation was performed, but the drop
 * destination did not accept any of the mime types offered through
 * wl_data_source.target. - The drag-and-drop operation was
 * performed, but the drop destination did not select any of the
 * actions present in the mask offered through
 * wl_data_source.action. - The drag-and-drop operation was
 * performed but didn't happen over a surface. - The compositor
 * cancelled the drag-and-drop operation (e.g. compositor dependent
 * timeouts to avoid stale drag-and-drop transfers).
 *
 * The client should clean up and destroy this data source.
 *
 * For objects of version 2 or older, wl_data_source.cancelled will
 * only be emitted if the data source was replaced by another data
 * source.
 */
void wl_data_source_handle_cancelled(void *data, struct wl_data_source *wl_data_source) {
	struct kinc_wl_data_source *data_source = wl_data_source_get_user_data(wl_data_source);
}
/**
 * the drag-and-drop operation physically finished
 *
 * The user performed the drop action. This event does not
 * indicate acceptance, wl_data_source.cancelled may still be
 * emitted afterwards if the drop destination does not accept any
 * mime type.
 *
 * However, this event might however not be received if the
 * compositor cancelled the drag-and-drop operation before this
 * event could happen.
 *
 * Note that the data_source may still be used in the future and
 * should not be destroyed here.
 * @since 3
 */
void wl_data_source_handle_dnd_drop_performed(void *data, struct wl_data_source *wl_data_source) {
	struct kinc_wl_data_source *data_source = wl_data_source_get_user_data(wl_data_source);
}
/**
 * the drag-and-drop operation concluded
 *
 * The drop destination finished interoperating with this data
 * source, so the client is now free to destroy this data source
 * and free all associated data.
 *
 * If the action used to perform the operation was "move", the
 * source can now delete the transferred data.
 * @since 3
 */
void wl_data_source_handle_dnd_finished(void *data, struct wl_data_source *wl_data_source) {
	struct kinc_wl_data_source *data_source = wl_data_source_get_user_data(wl_data_source);
}
/**
 * notify the selected action
 *
 * This event indicates the action selected by the compositor
 * after matching the source/destination side actions. Only one
 * action (or none) will be offered here.
 *
 * This event can be emitted multiple times during the
 * drag-and-drop operation, mainly in response to destination side
 * changes through wl_data_offer.set_actions, and as the data
 * device enters/leaves surfaces.
 *
 * It is only possible to receive this event after
 * wl_data_source.dnd_drop_performed if the drag-and-drop operation
 * ended in an "ask" action, in which case the final
 * wl_data_source.action event will happen immediately before
 * wl_data_source.dnd_finished.
 *
 * Compositors may also change the selected action on the fly,
 * mainly in response to keyboard modifier changes during the
 * drag-and-drop operation.
 *
 * The most recent action received is always the valid one. The
 * chosen action may change alongside negotiation (e.g. an "ask"
 * action can turn into a "move" operation), so the effects of the
 * final action must always be applied in
 * wl_data_offer.dnd_finished.
 *
 * Clients can trigger cursor surface changes from this point, so
 * they reflect the current action.
 * @param dnd_action action selected by the compositor
 * @since 3
 */
void wl_data_source_handle_action(void *data, struct wl_data_source *wl_data_source, uint32_t dnd_action) {
	struct kinc_wl_data_source *data_source = wl_data_source_get_user_data(wl_data_source);
}

static const struct wl_data_source_listener wl_data_source_listener = {
    wl_data_source_handle_target,       wl_data_source_handle_send,   wl_data_source_handle_cancelled, wl_data_source_handle_dnd_drop_performed,
    wl_data_source_handle_dnd_finished, wl_data_source_handle_action,
};

struct kinc_wl_data_source *kinc_wl_create_data_source(struct kinc_wl_seat *seat, const char *mime_types[], int num_mime_types, void *data, size_t data_size) {
	struct kinc_wl_data_source *data_source = kinc_allocate(sizeof *data_source);
	data_source->source = wl_data_device_manager_create_data_source(wl_ctx.data_device_manager);
	data_source->data = data;
	data_source->data_size = data_size;
	data_source->mime_types = mime_types;
	data_source->num_mime_types = num_mime_types;

	for (int i = 0; i < num_mime_types; i++) {
		wl_data_source_offer(data_source->source, mime_types[i]);
	}
	// wl_data_source_set_actions(data_source->source, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
	wl_data_source_set_user_data(data_source->source, data_source);
	wl_data_source_add_listener(data_source->source, &wl_data_source_listener, data_source);
	return data_source;
}

void kinc_wl_data_source_destroy(struct kinc_wl_data_source *data_source) {}

void wl_data_offer_handle_offer(void *data, struct wl_data_offer *wl_data_offer, const char *mime_type) {
	struct kinc_wl_data_offer *offer = wl_data_offer_get_user_data(wl_data_offer);
	if (offer != NULL) {
		offer->mime_type_count++;
		offer->mime_types = kinc_reallocate(offer->mime_types, offer->mime_type_count * sizeof(const char *));
		offer->mime_types[offer->mime_type_count - 1] = kinc_string_duplicate(mime_type);
	}
}

void wl_data_offer_handle_source_actions(void *data, struct wl_data_offer *wl_data_offer, uint32_t source_actions) {
	struct kinc_wl_data_offer *offer = wl_data_offer_get_user_data(wl_data_offer);
	offer->source_actions = source_actions;
}

void wl_data_offer_handle_action(void *data, struct wl_data_offer *wl_data_offer, uint32_t dnd_action) {
	struct kinc_wl_data_offer *offer = wl_data_offer_get_user_data(wl_data_offer);
	offer->dnd_action = dnd_action;
}

static const struct wl_data_offer_listener wl_data_offer_listener = {
    wl_data_offer_handle_offer,
    wl_data_offer_handle_source_actions,
    wl_data_offer_handle_action,
};

void kinc_wl_init_data_offer(struct wl_data_offer *id) {
	struct kinc_wl_data_offer *offer = kinc_allocate(sizeof *offer);
	kinc_memset(offer, 0, sizeof *offer);
	offer->id = id;
	offer->mime_type_count = 0;
	offer->mime_types = NULL;

	wl_data_offer_set_user_data(id, offer);
	wl_data_offer_add_listener(id, &wl_data_offer_listener, offer);
}

void kinc_wl_data_offer_accept(struct kinc_wl_data_offer *offer, void (*callback)(void *data, size_t data_size, void *user_data), void *user_data) {
	offer->callback = callback;
	offer->user_data = user_data;

	int fds[2];
	pipe(fds);
	wl_data_offer_receive(offer->id, "text/plain", fds[1]);
	close(fds[1]);

	wl_display_roundtrip(wl_ctx.display);

	offer->read_fd = fds[0];

	struct kinc_wl_data_offer **queue = &wl_ctx.data_offer_queue;

	while (*queue != NULL) queue = &(*queue)->next;
	*queue = offer;
}

void kinc_wl_destroy_data_offer(struct kinc_wl_data_offer *offer) {
	wl_data_offer_destroy(offer->id);
	if (offer->buffer != NULL) {
		kinc_free(offer->buffer);
	}
	for (int i = 0; i < offer->mime_type_count; i++) {
		kinc_free(offer->mime_types[i]);
	}
	kinc_free(offer->mime_types);
	kinc_free(offer);
}

/**
 * introduce a new wl_data_offer
 *
 * The data_offer event introduces a new wl_data_offer object,
 * which will subsequently be used in either the data_device.enter
 * event (for drag-and-drop) or the data_device.selection event
 * (for selections). Immediately following the
 * data_device_data_offer event, the new data_offer object will
 * send out data_offer.offer events to describe the mime types it
 * offers.
 * @param id the new data_offer object
 */
void wl_data_device_handle_data_offer(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *id) {
	struct kinc_wl_seat *seat = data;
	kinc_wl_init_data_offer(id);
}

/**
 * initiate drag-and-drop session
 *
 * This event is sent when an active drag-and-drop pointer enters
 * a surface owned by the client. The position of the pointer at
 * enter time is provided by the x and y arguments, in
 * surface-local coordinates.
 * @param serial serial number of the enter event
 * @param surface client surface entered
 * @param x surface-local x coordinate
 * @param y surface-local y coordinate
 * @param id source data_offer object
 */
void wl_data_device_handle_enter(void *data, struct wl_data_device *wl_data_device, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y,
                                 struct wl_data_offer *id) {
	struct kinc_wl_seat *seat = data;
	seat->current_dnd_offer = wl_data_offer_get_user_data(id);
	wl_data_offer_set_actions(id, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY | WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
}

/**
 * end drag-and-drop session
 *
 * This event is sent when the drag-and-drop pointer leaves the
 * surface and the session ends. The client must destroy the
 * wl_data_offer introduced at enter time at this point.
 */
void wl_data_device_handle_leave(void *data, struct wl_data_device *wl_data_device) {
	struct kinc_wl_seat *seat = data;
	kinc_wl_destroy_data_offer(seat->current_dnd_offer);
	seat->current_dnd_offer = NULL;
}

/**
 * drag-and-drop session motion
 *
 * This event is sent when the drag-and-drop pointer moves within
 * the currently focused surface. The new position of the pointer
 * is provided by the x and y arguments, in surface-local
 * coordinates.
 * @param time timestamp with millisecond granularity
 * @param x surface-local x coordinate
 * @param y surface-local y coordinate
 */
void wl_data_device_handle_motion(void *data, struct wl_data_device *wl_data_device, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
	struct kinc_wl_seat *seat = data;
}

/**
 * end drag-and-drop session successfully
 *
 * The event is sent when a drag-and-drop operation is ended
 * because the implicit grab is removed.
 *
 * The drag-and-drop destination is expected to honor the last
 * action received through wl_data_offer.action, if the resulting
 * action is "copy" or "move", the destination can still perform
 * wl_data_offer.receive requests, and is expected to end all
 * transfers with a wl_data_offer.finish request.
 *
 * If the resulting action is "ask", the action will not be
 * considered final. The drag-and-drop destination is expected to
 * perform one last wl_data_offer.set_actions request, or
 * wl_data_offer.destroy in order to cancel the operation.
 */

static void dnd_callback(void *data, size_t data_size, void *user_data) {
	char *str = data;
	if (kinc_string_compare_limited(data, "file://", kinc_string_length("file://")) == 0) {
		str += kinc_string_length("file://");
	}
	size_t wide_size = mbstowcs(NULL, str, 0) + 1;
	wchar_t *dest = kinc_allocate(wide_size * sizeof(wchar_t));
	mbstowcs(dest, str, wide_size);
	kinc_internal_drop_files_callback(dest);
	kinc_free(dest);
}

void wl_data_device_handle_drop(void *data, struct wl_data_device *wl_data_device) {
	struct kinc_wl_seat *seat = data;
	if (seat->current_dnd_offer != NULL) {
		kinc_wl_data_offer_accept(seat->current_dnd_offer, dnd_callback, NULL);
	}
}

void wl_data_device_handle_selection(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *id) {
	struct kinc_wl_seat *seat = data;
	if (seat->current_selection_offer != NULL && seat->current_selection_offer->id != id) {
		kinc_wl_destroy_data_offer(seat->current_selection_offer);
		seat->current_selection_offer = NULL;
	}

	if (id != NULL) {
		seat->current_selection_offer = wl_data_offer_get_user_data(id);
	}
}

static const struct wl_data_device_listener wl_data_device_listener = {
    wl_data_device_handle_data_offer, wl_data_device_handle_enter, wl_data_device_handle_leave,
    wl_data_device_handle_motion,     wl_data_device_handle_drop,  wl_data_device_handle_selection,
};

static void wl_registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
	if (kinc_string_compare(interface, wl_compositor_interface.name) == 0) {
		wl_ctx.compositor = wl_registry_bind(wl_ctx.registry, name, &wl_compositor_interface, 4);
	}
	else if (kinc_string_compare(interface, wl_shm_interface.name) == 0) {
		wl_ctx.shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	}
	else if (kinc_string_compare(interface, wl_subcompositor_interface.name) == 0) {
		wl_ctx.subcompositor = wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);
	}
	else if (kinc_string_compare(interface, wp_viewporter_interface.name) == 0) {
		wl_ctx.viewporter = wl_registry_bind(registry, name, &wp_viewporter_interface, 1);
	}
	else if (kinc_string_compare(interface, xdg_wm_base_interface.name) == 0) {
		wl_ctx.xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(wl_ctx.xdg_wm_base, &xdg_wm_base_listener, NULL);
	}
	else if (kinc_string_compare(interface, wl_seat_interface.name) == 0) {
		if (wl_ctx.seat.seat) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Multi-seat configurations not supported");
			return;
		}
		wl_ctx.seat.seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);

		wl_seat_add_listener(wl_ctx.seat.seat, &wl_seat_listener, &wl_ctx.seat);
		if (wl_ctx.data_device_manager != NULL) {
			wl_ctx.seat.data_device = wl_data_device_manager_get_data_device(wl_ctx.data_device_manager, wl_ctx.seat.seat);
			wl_data_device_add_listener(wl_ctx.seat.data_device, &wl_data_device_listener, &wl_ctx.seat);
		}
	}
	else if (kinc_string_compare(interface, wl_output_interface.name) == 0) {
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
	else if (kinc_string_compare(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
		wl_ctx.decoration_manager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
	}
	else if (kinc_string_compare(interface, wl_data_device_manager_interface.name) == 0) {
		wl_ctx.data_device_manager = wl_registry_bind(registry, name, &wl_data_device_manager_interface, 3);
		if (wl_ctx.seat.seat != NULL) {
			wl_ctx.seat.data_device = wl_data_device_manager_get_data_device(wl_ctx.data_device_manager, wl_ctx.seat.seat);
			wl_data_device_add_listener(wl_ctx.seat.data_device, &wl_data_device_listener, &wl_ctx.seat);
		}
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

void kinc_wayland_set_selection(struct kinc_wl_seat *seat, const char *text, int serial) {
	static const char *mime_types[] = {"text/plain"};
	struct kinc_wl_data_source *data_source =
	    kinc_wl_create_data_source(seat, mime_types, sizeof mime_types / sizeof mime_types[0], kinc_string_duplicate(text), kinc_string_length(text));
	wl_data_device_set_selection(seat->data_device, data_source->source, serial);
}

void kinc_wayland_copy_to_clipboard(const char *text) {}

#define READ_SIZE 64

bool kinc_wayland_handle_messages() {
	wl_display_dispatch(wl_ctx.display);
	while (wl_display_prepare_read(wl_ctx.display) != 0) wl_display_dispatch_pending(wl_ctx.display);
	wl_display_flush(wl_ctx.display);
	wl_display_read_events(wl_ctx.display);
	wl_display_dispatch_pending(wl_ctx.display);
	wl_display_roundtrip(wl_ctx.display);

	struct kinc_wl_data_offer **offer = &wl_ctx.data_offer_queue;
	while (*offer != NULL) {
		struct kinc_wl_data_offer *current = *offer;
		struct kinc_wl_data_offer **next = &current->next;
		if (current->buf_pos + READ_SIZE > current->buf_size) {
			current->buffer = kinc_reallocate(current->buffer, current->buf_size + READ_SIZE);
			current->buf_size += READ_SIZE;
		}

		ssize_t n = read(current->read_fd, current->buffer + current->buf_pos, READ_SIZE);
		if (n <= 0) {
			*offer = *next;
			close(current->read_fd);

			current->callback(current->buffer, current->buf_pos, current->user_data);

			kinc_free(current->buffer);
			current->buffer = NULL;
			current->buf_pos = 0;
			current->buf_size = 0;
			current->read_fd = 0;
			current->next = NULL;
		}
		else {
			current->buf_pos += n;
			offer = next;
		}
	}
	return false;
}

#undef READ_SIZE

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