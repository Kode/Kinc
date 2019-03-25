#pragma once

#include <Kore/Graphics4/Graphics.h>

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

typedef struct {
	struct ID3D11Texture2D *texture;
	struct ID3D11ShaderResourceView *view;
	//void set(Kore::Graphics4::TextureUnit unit);
} Kinc_G4_TextureArrayImpl;
