#include "textureformat.h"

#include <assert.h>

uint32_t kope_g5_texture_format_byte_size(kope_g5_texture_format format) {
	switch (format) {
	case KOPE_G5_TEXTURE_FORMAT_R8_UNORM:
		return 1;
	case KOPE_G5_TEXTURE_FORMAT_R8_SNORM:
		return 1;
	case KOPE_G5_TEXTURE_FORMAT_R8_UINT:
		return 1;
	case KOPE_G5_TEXTURE_FORMAT_R8_SINT:
		return 1;
	case KOPE_G5_TEXTURE_FORMAT_R16_UINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_R16_SINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_R16_FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG8_UNORM:
		return 2;
	case KOPE_G5_TEXTURE_FORMAT_RG8_SNORM:
		return 2;
	case KOPE_G5_TEXTURE_FORMAT_RG8_UINT:
		return 2;
	case KOPE_G5_TEXTURE_FORMAT_RG8_SINT:
		return 2;
	case KOPE_G5_TEXTURE_FORMAT_R32_UINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_R32_SINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_R32_FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG16_UINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG16_SINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG16_FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_UNORM_SRGB:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_SNORM:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_UINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGBA8_SINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_BGRA8_UNORM_SRGB:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGB9E5U_FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGB10A2_UINT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RGB10A2_UNORM:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG11B10U_FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_RG32_UINT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RG32_SINT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RG32_FLOAT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RGBA16_UINT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RGBA16_SINT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RGBA16_FLOAT:
		return 8;
	case KOPE_G5_TEXTURE_FORMAT_RGBA32_UINT:
		return 16;
	case KOPE_G5_TEXTURE_FORMAT_RGBA32_SINT:
		return 16;
	case KOPE_G5_TEXTURE_FORMAT_RGBA32_FLOAT:
		return 16;
	// case KOPE_G5_TEXTURE_FORMAT_STENCIL8:
	//	return 1;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH16_UNORM:
		return 2;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_NOTHING8:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_STENCIL8:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT:
		return 4;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT_STENCIL8_NOTHING24:
		return 8;
	}

	assert(false);
	return 4;
}

bool kope_g5_texture_format_is_depth(kope_g5_texture_format format) {
	switch (format) {
	// case KOPE_G5_TEXTURE_FORMAT_STENCIL8:
	//	return 1;
	case KOPE_G5_TEXTURE_FORMAT_DEPTH16_UNORM:
	case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_NOTHING8:
	case KOPE_G5_TEXTURE_FORMAT_DEPTH24PLUS_STENCIL8:
	case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT:
	case KOPE_G5_TEXTURE_FORMAT_DEPTH32FLOAT_STENCIL8_NOTHING24:
		return true;
	default:
		return false;
	}
	return false;
}
