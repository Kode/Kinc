#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

typedef struct {
	struct ID3D11Texture2D *texture;
	struct ID3D11ShaderResourceView *view;
	// void set(Kore::Graphics4::TextureUnit unit);
} kinc_g4_texture_array_impl_t;

#ifdef __cplusplus
}
#endif
