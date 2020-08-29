#pragma once

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC extern void (*kinc_surface_touch_start_callback)(int /*index*/, int /*x*/, int /*y*/);
KINC_FUNC extern void (*kinc_surface_move_callback)(int /*index*/, int /*x*/, int /*y*/);
KINC_FUNC extern void (*kinc_surface_touch_end_callback)(int /*index*/, int /*x*/, int /*y*/);

void kinc_internal_surface_trigger_touch_start(int index, int x, int y);
void kinc_internal_surface_trigger_move(int index, int x, int y);
void kinc_internal_surface_trigger_touch_end(int index, int x, int y);

#ifdef __cplusplus
}
#endif
