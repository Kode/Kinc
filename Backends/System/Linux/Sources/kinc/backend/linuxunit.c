#ifndef _GNU_SOURCE
#define _GNU_SOURCE // memfd_create and mkostemp
#endif
#include "funcs.h"
#include <dlfcn.h>
// #include <kinc/log.h>

#ifndef KINC_NO_WAYLAND
#include "wayland/display.c.h"
#include "wayland/system.c.h"
#include "wayland/wayland-protocol.c.h"
#include "wayland/wayland-viewporter.c.h"
#include "wayland/window.c.h"
#include "wayland/xdg-decoration.c.h"
#include "wayland/xdg-shell.c.h"
#endif

#ifndef KINC_NO_X11
#include "x11/display.c.h"
#include "x11/system.c.h"
#include "x11/window.c.h"
#endif

struct linux_procs procs = {0};

void kinc_linux_init_procs() {
	if (procs.window_create != NULL) {
		return;
	}
#ifndef KINC_NO_WAYLAND
	if (kinc_wayland_init()) {
		procs.handle_messages = kinc_wayland_handle_messages;
		procs.shutdown = kinc_wayland_shutdown;

		procs.window_create = kinc_wayland_window_create;
		procs.window_width = kinc_wayland_window_width;
		procs.window_height = kinc_wayland_window_height;
		procs.window_x = kinc_wayland_window_x;
		procs.window_y = kinc_wayland_window_y;
		procs.window_destroy = kinc_wayland_window_destroy;
		procs.window_change_mode = kinc_wayland_window_change_mode;
		procs.window_get_mode = kinc_wayland_window_get_mode;
		procs.window_set_title = kinc_wayland_window_set_title;
		procs.window_display = kinc_wayland_window_display;
		procs.window_move = kinc_wayland_window_move;
		procs.window_resize = kinc_wayland_window_resize;
		procs.window_show = kinc_wayland_window_show;
		procs.window_hide = kinc_wayland_window_hide;
		procs.count_windows = kinc_wayland_count_windows;

		procs.display_init = kinc_wayland_display_init;
		procs.display_available = kinc_wayland_display_available;
		procs.display_available_mode = kinc_wayland_display_available_mode;
		procs.display_current_mode = kinc_wayland_display_current_mode;
		procs.display_name = kinc_wayland_display_name;
		procs.display_primary = kinc_wayland_display_primary;
		procs.count_displays = kinc_wayland_count_displays;

		procs.copy_to_clipboard = kinc_wayland_copy_to_clipboard;
#ifdef KINC_EGL
		procs.egl_get_display = kinc_wayland_egl_get_display;
		procs.egl_get_native_window = kinc_wayland_egl_get_native_window;
#endif
#ifdef KORE_VULKAN
		procs.vulkan_create_surface = kinc_wayland_vulkan_create_surface;
		procs.vulkan_get_instance_extensions = kinc_wayland_vulkan_get_instance_extensions;
#endif
	}
	else
#endif
#ifndef KINC_NO_X11
	if (kinc_x11_init()) {
		procs.handle_messages = kinc_x11_handle_messages;
		procs.shutdown = kinc_x11_shutdown;

		procs.window_create = kinc_x11_window_create;
		procs.window_width = kinc_x11_window_width;
		procs.window_height = kinc_x11_window_height;
		procs.window_x = kinc_x11_window_x;
		procs.window_y = kinc_x11_window_y;
		procs.window_destroy = kinc_x11_window_destroy;
		procs.window_change_mode = kinc_x11_window_change_mode;
		procs.window_get_mode = kinc_x11_window_get_mode;
		procs.window_set_title = kinc_x11_window_set_title;
		procs.window_display = kinc_x11_window_display;
		procs.window_move = kinc_x11_window_move;
		procs.window_resize = kinc_x11_window_resize;
		procs.window_show = kinc_x11_window_show;
		procs.window_hide = kinc_x11_window_hide;
		procs.count_windows = kinc_x11_count_windows;

		procs.display_init = kinc_x11_display_init;
		procs.display_available = kinc_x11_display_available;
		procs.display_available_mode = kinc_x11_display_available_mode;
		procs.display_current_mode = kinc_x11_display_current_mode;
		procs.display_name = kinc_x11_display_name;
		procs.display_primary = kinc_x11_display_primary;
		procs.count_displays = kinc_x11_count_displays;

		procs.mouse_can_lock = kinc_x11_mouse_can_lock;
		procs.mouse_lock = kinc_x11_mouse_lock;
		procs.mouse_unlock = kinc_x11_mouse_unlock;
		procs.mouse_show = kinc_x11_mouse_show;
		procs.mouse_hide = kinc_x11_mouse_hide;
		procs.mouse_set_position = kinc_x11_mouse_set_position;
		procs.mouse_get_position = kinc_x11_mouse_get_position;

		procs.copy_to_clipboard = kinc_x11_copy_to_clipboard;
#ifdef KINC_EGL
		procs.egl_get_display = kinc_x11_egl_get_display;
		procs.egl_get_native_window = kinc_x11_egl_get_native_window;
#endif
#ifdef KORE_VULKAN
		procs.vulkan_create_surface = kinc_x11_vulkan_create_surface;
		procs.vulkan_get_instance_extensions = kinc_x11_vulkan_get_instance_extensions;
#endif
	}
	else
#endif
	{
		kinc_log(KINC_LOG_LEVEL_ERROR, "Neither wayland nor X11 found.");
		exit(1);
	}
}

#include "display.c.h"
#ifndef __FreeBSD__
#include "gamepad.c.h"
#endif
#include "mouse.c.h"
#include "sound.c.h"
#include "system.c.h"
#include "video.c.h"
#include "window.c.h"
