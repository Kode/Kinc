#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE // put this here too, to make clangd happy
#endif
#include <kinc/display.h>
#include <kinc/log.h>
#include <kinc/memory.h>
#include <kinc/string.h>
#include <kinc/system.h>

#include <wayland-client-core.h>
#include <wayland-cursor.h>

struct wl_surface;

struct kinc_wl_procs {
	void (*_wl_event_queue_destroy)(struct wl_event_queue *queue);
	struct wl_proxy *(*_wl_proxy_marshal_flags)(struct wl_proxy *proxy, uint32_t opcode, const struct wl_interface *interface, uint32_t version, uint32_t flags,
	                                            ...);
	struct wl_proxy *(*_wl_proxy_marshal_array_flags)(struct wl_proxy *proxy, uint32_t opcode, const struct wl_interface *interface, uint32_t version,
	                                                  uint32_t flags, union wl_argument *args);
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

	struct wl_cursor_theme *(*_wl_cursor_theme_load)(const char *name, int size, struct wl_shm *shm);
	void (*_wl_cursor_theme_destroy)(struct wl_cursor_theme *theme);
	struct wl_cursor *(*_wl_cursor_theme_get_cursor)(struct wl_cursor_theme *theme, const char *name);
	struct wl_buffer *(*_wl_cursor_image_get_buffer)(struct wl_cursor_image *image);
	int (*_wl_cursor_frame)(struct wl_cursor *cursor, uint32_t time);
	int (*_wl_cursor_frame_and_duration)(struct wl_cursor *cursor, uint32_t time, uint32_t *duration);

#ifdef KINC_EGL
	struct wl_egl_window *(*_wl_egl_window_create)(struct wl_surface *surface, int width, int height);
	void (*_wl_egl_window_destroy)(struct wl_egl_window *egl_window);
	void (*_wl_egl_window_resize)(struct wl_egl_window *egl_window, int width, int height, int dx, int dy);
	void (*_wl_egl_window_get_attached_size)(struct wl_egl_window *egl_window, int *width, int *height);
#endif
};

#define wl_event_queue_destroy wl._wl_event_queue_destroy
#define wl_proxy_marshal_flags wl._wl_proxy_marshal_flags
#define wl_proxy_marshal_array_flags wl._wl_proxy_marshal_array_flags
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

#define wl_cursor_theme_load wl._wl_cursor_theme_load
#define wl_cursor_theme_destroy wl._wl_cursor_theme_destroy
#define wl_cursor_theme_get_cursor wl._wl_cursor_theme_get_cursor
#define wl_cursor_image_get_buffer wl._wl_cursor_image_get_buffer
#define wl_cursor_frame wl._wl_cursor_frame
#define wl_cursor_frame_and_duration wl._wl_cursor_frame_and_duration

#define wl_egl_window_create wl._wl_egl_window_create
#define wl_egl_window_destroy wl._wl_egl_window_destroy
#define wl_egl_window_resize wl._wl_egl_window_resize

extern struct kinc_wl_procs wl;

#include "wayland-viewporter.h"
#include "xdg-decoration.h"
#include "xdg-shell.h"
#include <wayland-client-protocol.h>

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
	int (*xkb_state_mod_name_is_active)(struct xkb_state *state, const char *name, enum xkb_state_component type);
};

extern struct kinc_xkb_procs wl_xkb;

#include <kinc/display.h>
#include <kinc/global.h>
#include <kinc/system.h>
#include <kinc/window.h>

#define MAXIMUM_WINDOWS 16
#define MAXIMUM_DISPLAYS 16
#define MAXIMUM_DISPLAY_MODES 16

struct kinc_wl_decoration {
	struct wl_surface *surface;
	struct wl_subsurface *subsurface;
	struct wp_viewport *viewport;
};

enum kinc_wl_decoration_focus {
	KINC_WL_DECORATION_FOCUS_MAIN,
	KINC_WL_DECORATION_FOCUS_TOP,
	KINC_WL_DECORATION_FOCUS_LEFT,
	KINC_WL_DECORATION_FOCUS_RIGHT,
	KINC_WL_DECORATION_FOCUS_BOTTOM,
	KINC_WL_DECORATION_FOCUS_CLOSE_BUTTON,
	KINC_WL_DECORATION_FOCUS_MAX_BUTTON,
	KINC_WL_DECORATION_FOCUS_MIN_BUTTON
};

#define KINC_WL_DECORATION_WIDTH 10

#define KINC_WL_DECORATION_TOP_X 0
#define KINC_WL_DECORATION_TOP_Y -(KINC_WL_DECORATION_TOP_HEIGHT)
#define KINC_WL_DECORATION_TOP_WIDTH window->width
#define KINC_WL_DECORATION_TOP_HEIGHT KINC_WL_DECORATION_WIDTH * 3

#define KINC_WL_DECORATION_LEFT_X -10
#define KINC_WL_DECORATION_LEFT_Y -(KINC_WL_DECORATION_TOP_HEIGHT)
#define KINC_WL_DECORATION_LEFT_WIDTH KINC_WL_DECORATION_WIDTH
#define KINC_WL_DECORATION_LEFT_HEIGHT window->height + KINC_WL_DECORATION_TOP_HEIGHT + KINC_WL_DECORATION_BOTTOM_HEIGHT

#define KINC_WL_DECORATION_RIGHT_X window->width
#define KINC_WL_DECORATION_RIGHT_Y -(KINC_WL_DECORATION_TOP_HEIGHT)
#define KINC_WL_DECORATION_RIGHT_WIDTH 10
#define KINC_WL_DECORATION_RIGHT_HEIGHT window->height + KINC_WL_DECORATION_TOP_HEIGHT + KINC_WL_DECORATION_BOTTOM_HEIGHT

#define KINC_WL_DECORATION_BOTTOM_X 0
#define KINC_WL_DECORATION_BOTTOM_Y window->height
#define KINC_WL_DECORATION_BOTTOM_WIDTH window->width
#define KINC_WL_DECORATION_BOTTOM_HEIGHT KINC_WL_DECORATION_WIDTH

#define KINC_WL_DECORATION_CLOSE_X window->width - 10
#define KINC_WL_DECORATION_CLOSE_Y -20
#define KINC_WL_DECORATION_CLOSE_WIDTH 9
#define KINC_WL_DECORATION_CLOSE_HEIGHT 9

struct kinc_wl_window {
	int display_index;
	int window_id;
	int width;
	int height;
	kinc_window_mode_t mode;
	struct wl_surface *surface;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *toplevel;
	struct zxdg_toplevel_decoration_v1 *xdg_decoration;

	struct {
		bool server_side;
		enum kinc_wl_decoration_focus focus;
		struct kinc_wl_decoration top;
		struct kinc_wl_decoration left;
		struct kinc_wl_decoration right;
		struct kinc_wl_decoration bottom;

		struct kinc_wl_decoration close;
		struct kinc_wl_decoration max;
		struct kinc_wl_decoration min;

		struct wl_buffer *dec_buffer;
		struct wl_buffer *close_buffer;
		struct wl_buffer *max_buffer;
		struct wl_buffer *min_buffer;
	} decorations;
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
	struct kinc_wl_seat *seat;
	int current_window;
	int x;
	int y;
	int enter_serial;
	const char *previous_cursor_name;
	struct wl_pointer *pointer;
	struct wl_surface *surface;
};

struct kinc_wl_keyboard {
	struct kinc_wl_seat *seat;
	struct wl_keyboard *keyboard;
	struct xkb_keymap *keymap;
	struct xkb_state *state;
	bool ctrlDown;
};

struct kinc_wl_data_offer {
	struct wl_data_offer *id;

	int mime_type_count;
	char **mime_types;

	uint32_t source_actions;
	uint32_t dnd_action;

	void (*callback)(void *data, size_t data_size, void *user_data);
	void *user_data;
	int read_fd;

	void *buffer;
	size_t buf_size;
	size_t buf_pos;

	struct kinc_wl_data_offer *next;
};

struct kinc_wl_data_source {
	struct wl_data_source *source;
	const char **mime_types;
	int num_mime_types;
	void *data;
	size_t data_size;
};

struct kinc_wl_seat {
	struct wl_seat *seat;
	struct kinc_wl_keyboard keyboard;
	struct kinc_wl_mouse mouse;
	struct wl_touch *touch;
	struct wl_data_device *data_device;
	struct kinc_wl_data_offer *current_selection_offer;
	struct kinc_wl_data_offer *current_dnd_offer;
	int current_serial;
	uint32_t capabilities;
	char name[64];
};

struct wayland_context {
	struct xkb_context *xkb_context;

	struct wl_display *display;
	struct wl_shm *shm;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_subcompositor *subcompositor;
	struct wp_viewporter *viewporter;
	struct kinc_wl_seat seat;
	struct xdg_wm_base *xdg_wm_base;
	struct zxdg_decoration_manager_v1 *decoration_manager;
	struct wl_data_device_manager *data_device_manager;
	struct wl_cursor_theme *cursor_theme;
	int cursor_size;
	int num_windows;
	struct kinc_wl_window windows[MAXIMUM_WINDOWS];
	int num_displays;
	struct kinc_wl_display displays[MAXIMUM_DISPLAYS];

	struct kinc_wl_data_offer *data_offer_queue;
};

extern struct wayland_context wl_ctx;

struct kinc_wl_data_source *kinc_wl_create_data_source(struct kinc_wl_seat *seat, const char *mime_types[], int num_mime_types, void *data, size_t data_size);
void kinc_wl_data_source_destroy(struct kinc_wl_data_source *data_source);
void kinc_wl_data_offer_accept(struct kinc_wl_data_offer *offer, void (*callback)(void *data, size_t data_size, void *user_data), void *user_data);
void kinc_wl_destroy_data_offer(struct kinc_wl_data_offer *offer);
void kinc_wayland_set_selection(struct kinc_wl_seat *seat, const char *text, int serial);
void kinc_wayland_window_destroy(int window_index);