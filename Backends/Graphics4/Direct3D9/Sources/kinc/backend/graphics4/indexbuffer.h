#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DIndexBuffer9;

typedef struct {
	struct IDirect3DIndexBuffer9 *ib;
	int myCount;
} kinc_g4_index_buffer_impl_t;

struct kinc_g4_index_buffer;

extern struct kinc_g4_index_buffer *kinc_internal_current_index_buffer;

#ifdef __cplusplus
}
#endif
