#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DVertexBuffer9;

typedef struct {
	struct IDirect3DVertexBuffer9 *vb;
	int myCount;
	int myStride;
	int instanceDataStepRate;
	int _offset;
} kinc_g4_vertex_buffer_impl_t;

struct kinc_g4_vertex_buffer;

void kinc_internal_vertex_buffer_unset(struct kinc_g4_vertex_buffer *buffer);

extern struct kinc_g4_vertex_buffer *kinc_internal_current_vertex_buffer;

#ifdef __cplusplus
}
#endif
