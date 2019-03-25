#pragma once

#include "Texture.h"

#include <Kore/TextureArrayImpl.h>

typedef struct {
	Kinc_G4_TextureArrayImpl impl;
} Kinc_G4_TextureArray;

void Kinc_G4_TextureArray_Create(Image **textures, int count);
