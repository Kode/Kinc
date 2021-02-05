#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct IDirect3DSurface9;
struct IDirect3DTexture9;

typedef struct {
	struct IDirect3DSurface9 *colorSurface;
	struct IDirect3DSurface9 *depthSurface;
	struct IDirect3DTexture9 *colorTexture;
	struct IDirect3DTexture9 *depthTexture;
	bool antialiasing;
} kinc_g4_render_target_impl_t;

#ifdef __cplusplus
}
#endif
