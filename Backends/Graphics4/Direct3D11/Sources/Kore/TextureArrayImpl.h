#pragma once

#include <Kore/Graphics4/Graphics.h>

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

class TextureArrayImpl {
public:
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* view;
	void set(Kore::Graphics4::TextureUnit unit);
};
