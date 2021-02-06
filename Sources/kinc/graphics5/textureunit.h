#pragma once

#include "pch.h"

#include <kinc/backend/graphics5/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g5_texture_unit {
	TextureUnit5Impl impl;
} kinc_g5_texture_unit_t;

#ifdef __cplusplus
}
#endif
