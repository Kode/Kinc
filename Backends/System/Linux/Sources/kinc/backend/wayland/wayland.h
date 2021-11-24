#pragma once

#include <wayland-client-core.h>

struct kinc_wl_procs {
	void (*_wl_event_queue_destroy)(struct wl_event_queue *queue);
	void (*_wl_proxy_marshal)(struct wl_proxy *p, uint32_t opcode, ...);
	void (*_wl_proxy_marshal_array)(struct wl_proxy *p, uint32_t opcode, union wl_argument *args);
	struct wl_proxy *(*_wl_proxy_create)(struct wl_proxy *factory, const struct wl_interface *interface);
	void *(*_wl_proxy_create_wrapper)(void *proxy);
	void (*_wl_proxy_wrapper_destroy)(void *proxy_wrapper);
	struct wl_proxy *(*_wl_proxy_marshal_constructor)(struct wl_proxy *proxy, uint32_t opcode, const struct wl_interface *interface, ...);
	struct wl_proxy *(*_wl_proxy_marshal_constructor_versioned)(struct wl_proxy *proxy, uint32_t opcode, const struct wl_interface *interface, uint32_t version,
	                                                            ...);
	struct wl_proxy *(*_wl_proxy_marshal_array_constructor)(struct wl_proxy *proxy, uint32_t opcode, union wl_argument *args,
	                                                        const struct wl_interface *interface);
	struct wl_proxy *(*_wl_proxy_marshal_array_constructor_versioned)(struct wl_proxy *proxy, uint32_t opcode, union wl_argument *args,
	                                                                  const struct wl_interface *interface, uint32_t version);
	void (*_wl_proxy_destroy)(struct wl_proxy *proxy);
	int (*_wl_proxy_add_listener)(struct wl_proxy *proxy, void (**implementation)(void), void *data);
	const void *(*_wl_proxy_get_listener)(struct wl_proxy *proxy);
	int (*_wl_proxy_add_dispatcher)(struct wl_proxy *proxy, wl_dispatcher_func_t dispatcher_func, const void *dispatcher_data, void *data);
	void (*_wl_proxy_set_user_data)(struct wl_proxy *proxy, void *user_data);
	void *(*_wl_proxy_get_user_data)(struct wl_proxy *proxy);
	uint32_t (*_wl_proxy_get_version)(struct wl_proxy *proxy);
	uint32_t (*_wl_proxy_get_id)(struct wl_proxy *proxy);
	void (*_wl_proxy_set_tag)(struct wl_proxy *proxy, const char *const *tag);
	const char *const *(*_wl_proxy_get_tag)(struct wl_proxy *proxy);
	const char *(*_wl_proxy_get_class)(struct wl_proxy *proxy);
	void (*_wl_proxy_set_queue)(struct wl_proxy *proxy, struct wl_event_queue *queue);
	struct wl_display *(*_wl_display_connect)(const char *name);
	struct wl_display *(*_wl_display_connect_to_fd)(int fd);
	void (*_wl_display_disconnect)(struct wl_display *display);
	int (*_wl_display_get_fd)(struct wl_display *display);
	int (*_wl_display_dispatch)(struct wl_display *display);
	int (*_wl_display_dispatch_queue)(struct wl_display *display, struct wl_event_queue *queue);
	int (*_wl_display_dispatch_queue_pending)(struct wl_display *display, struct wl_event_queue *queue);
	int (*_wl_display_dispatch_pending)(struct wl_display *display);
	int (*_wl_display_get_error)(struct wl_display *display);
	uint32_t (*_wl_display_get_protocol_error)(struct wl_display *display, const struct wl_interface **interface, uint32_t *id);
	int (*_wl_display_flush)(struct wl_display *display);
	int (*_wl_display_roundtrip_queue)(struct wl_display *display, struct wl_event_queue *queue);
	int (*_wl_display_roundtrip)(struct wl_display *display);
	struct wl_event_queue *(*_wl_display_create_queue)(struct wl_display *display);
	int (*_wl_display_prepare_read_queue)(struct wl_display *display, struct wl_event_queue *queue);
	int (*_wl_display_prepare_read)(struct wl_display *display);
	void (*_wl_display_cancel_read)(struct wl_display *display);
	int (*_wl_display_read_events)(struct wl_display *display);
	void (*_wl_log_set_handler_client)(wl_log_func_t handler);
};

#define wl_event_queue_destroy wl._wl_event_queue_destroy
#define wl_proxy_marshal wl._wl_proxy_marshal
#define wl_proxy_marshal_array wl._wl_proxy_marshal_array
#define wl_proxy_create wl._wl_proxy_create
#define wl_proxy_create_wrapper wl._wl_proxy_create_wrapper
#define wl_proxy_wrapper_destroy wl._wl_proxy_wrapper_destroy
#define wl_proxy_marshal_constructor wl._wl_proxy_marshal_constructor
#define wl_proxy_marshal_constructor_versioned wl._wl_proxy_marshal_constructor_versioned
#define wl_proxy_marshal_array_constructor wl._wl_proxy_marshal_array_constructor
#define wl_proxy_marshal_array_constructor_versioned wl._wl_proxy_marshal_array_constructor_versioned
#define wl_proxy_destroy wl._wl_proxy_destroy
#define wl_proxy_add_listener wl._wl_proxy_add_listener
#define wl_proxy_get_listener wl._wl_proxy_get_listener
#define wl_proxy_add_dispatcher wl._wl_proxy_add_dispatcher
#define wl_proxy_set_user_data wl._wl_proxy_set_user_data
#define wl_proxy_get_user_data wl._wl_proxy_get_user_data
#define wl_proxy_get_version wl._wl_proxy_get_version
#define wl_proxy_get_id wl._wl_proxy_get_id
#define wl_proxy_set_tag wl._wl_proxy_set_tag
#define wl_proxy_get_tag wl._wl_proxy_get_tag
#define wl_proxy_get_class wl._wl_proxy_get_class
#define wl_proxy_set_queue wl._wl_proxy_set_queue
#define wl_display_connect wl._wl_display_connect
#define wl_display_connect_to_fd wl._wl_display_connect_to_fd
#define wl_display_disconnect wl._wl_display_disconnect
#define wl_display_get_fd wl._wl_display_get_fd
#define wl_display_dispatch wl._wl_display_dispatch
#define wl_display_dispatch_queue wl._wl_display_dispatch_queue
#define wl_display_dispatch_queue_pending wl._wl_display_dispatch_queue_pending
#define wl_display_dispatch_pending wl._wl_display_dispatch_pending
#define wl_display_get_error wl._wl_display_get_error
#define wl_display_get_protocol_error wl._wl_display_get_protocol_error
#define wl_display_flush wl._wl_display_flush
#define wl_display_roundtrip_queue wl._wl_display_roundtrip_queue
#define wl_display_roundtrip wl._wl_display_roundtrip
#define wl_display_create_queue wl._wl_display_create_queue
#define wl_display_prepare_read_queue wl._wl_display_prepare_read_queue
#define wl_display_prepare_read wl._wl_display_prepare_read
#define wl_display_cancel_read wl._wl_display_cancel_read
#define wl_display_read_events wl._wl_display_read_events
#define wl_log_set_handler_client wl._wl_log_set_handler_client

extern struct kinc_wl_procs wl;

#include "wayland-client-protocol.h"
#include "xdg-decoration.h"
#include "xdg-shell.h"

#ifdef KINC_EGL
struct kinc_wl_egl_procs {
	struct wl_egl_window *(*_wl_egl_window_create)(struct wl_surface *surface, int width, int height);
	void (*_wl_egl_window_destroy)(struct wl_egl_window *egl_window);
	void (*_wl_egl_window_resize)(struct wl_egl_window *egl_window, int width, int height, int dx, int dy);
	void (*_wl_egl_window_get_attached_size)(struct wl_egl_window *egl_window, int *width, int *height);
};

#define wl_egl_window_create wl_egl._wl_egl_window_create
#define wl_egl_window_destroy wl_egl._wl_egl_window_destroy
#define wl_egl_window_resize wl_egl._wl_egl_window_resize

extern struct kinc_wl_egl_procs wl_egl;
#endif

#include <xkbcommon/xkbcommon.h>

struct kinc_xkb_procs {
	struct xkb_context *(*xkb_context_new)(enum xkb_context_flags flags);
	void (*xkb_context_unref)(struct xkb_context *context);
	struct xkb_keymap *(*xkb_keymap_new_from_string)(struct xkb_context *context, const char *string, enum xkb_keymap_format format,
	                                                 enum xkb_keymap_compile_flags flags);
	struct xkb_state *(*xkb_state_new)(struct xkb_keymap *keymap);
	xkb_keysym_t (*xkb_state_key_get_one_sym)(struct xkb_state *state, xkb_keycode_t key);
	uint32_t (*xkb_state_key_get_utf32)(struct xkb_state *state, xkb_keycode_t key);
	xkb_mod_mask_t (*xkb_state_serialize_mods)(struct xkb_state *state, enum xkb_state_component components);
	enum xkb_state_component (*xkb_state_update_mask)(struct xkb_state *state, xkb_mod_mask_t depressed_mods, xkb_mod_mask_t latched_mods,
	                                               xkb_mod_mask_t locked_mods, xkb_layout_index_t depressed_layout, xkb_layout_index_t latched_layout,
	                                               xkb_layout_index_t locked_layout);
};

extern struct kinc_xkb_procs wl_xkb;

#include <kinc/display.h>
#include <kinc/global.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include <xkbcommon/xkbcommon.h>

#define MAXIMUM_WINDOWS 16
#define MAXIMUM_DISPLAYS 16
#define MAXIMUM_DISPLAY_MODES 16

struct kinc_wl_window {
	int display_index;
	int window_id;
	int width;
	int height;
	kinc_window_mode_t mode;
	struct wl_surface *surface;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *toplevel;
	struct zxdg_toplevel_decoration_v1 *decoration;
#ifdef KINC_EGL
	struct wl_egl_window *egl_window;
#endif
};

struct kinc_wl_display {
	struct wl_output *output;
	int index;
	int current_mode;
	int num_modes;
	char name[64];
	int x;
	int y;
	int physical_width;
	int physical_height;
	int subpixel;
	enum wl_output_transform transform;
	enum wl_output_subpixel scale;
	kinc_display_mode_t modes[MAXIMUM_DISPLAY_MODES];
};

struct kinc_wl_mouse {
	int current_window;
	int x;
	int y;
	struct wl_pointer *pointer;
};

struct kinc_wl_keyboard {
	struct wl_keyboard *keyboard;
	struct xkb_keymap *keymap;
	struct xkb_state *state;
};

struct kinc_wl_seat {
	struct wl_seat *seat;
	struct kinc_wl_keyboard keyboard;
	struct kinc_wl_mouse mouse;
	struct wl_touch *touch;
	uint32_t capabilities;
	char name[64];
};

struct wayland_context {
	struct xkb_context *xkb_context;

	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct kinc_wl_seat seat;
	struct xdg_wm_base *xdg_wm_base;
	struct zxdg_decoration_manager_v1 *decoration_manager;
	int num_windows;
	struct kinc_wl_window windows[MAXIMUM_WINDOWS];
	int num_displays;
	struct kinc_wl_display displays[MAXIMUM_DISPLAYS];
};

extern struct wayland_context wl_ctx;
