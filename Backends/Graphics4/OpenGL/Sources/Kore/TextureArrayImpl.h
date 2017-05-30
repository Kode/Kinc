#pragma once

#include <Kore/Graphics4/Graphics.h>

class TextureArrayImpl {
public:
	unsigned texture;
	void set(Kore::Graphics4::TextureUnit unit);
};
