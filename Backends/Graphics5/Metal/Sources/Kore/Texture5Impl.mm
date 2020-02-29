#include "pch.h"

#include "Texture5Impl.h"

#include <kinc/image.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/texture.h>
#include <kinc/log.h>

#import <Metal/Metal.h>

id getMetalDevice();

namespace {
	MTLPixelFormat convert(kinc_image_format_t format) {
		switch (format) {
			case KINC_IMAGE_FORMAT_RGBA32:
				return MTLPixelFormatRGBA8Unorm;
			case KINC_IMAGE_FORMAT_GREY8:
				return MTLPixelFormatR8Unorm;
			case KINC_IMAGE_FORMAT_RGB24:
			case KINC_IMAGE_FORMAT_RGBA128:
			case KINC_IMAGE_FORMAT_RGBA64:
			case KINC_IMAGE_FORMAT_A32:
			case KINC_IMAGE_FORMAT_BGRA32:
			case KINC_IMAGE_FORMAT_A16:
				return MTLPixelFormatRGBA8Unorm;
		}
	}
}

static void create(kinc_g5_texture_t *texture, int width, int height, int format, bool writable) {
	id<MTLDevice> device = getMetalDevice();

	MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:convert((kinc_image_format_t)format) width:width height:height mipmapped:NO];
	descriptor.textureType = MTLTextureType2D;
	descriptor.width = width;
	descriptor.height = height;
	descriptor.depth = 1;
	descriptor.pixelFormat = convert((kinc_image_format_t)format);
	descriptor.arrayLength = 1;
	descriptor.mipmapLevelCount = 1;
	//TODO: Make less textures writable
	if (writable) {
		descriptor.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
	}

	texture->impl._tex = [device newTextureWithDescriptor:descriptor];

	texture->impl._samplerDesc = (MTLSamplerDescriptor*)[[MTLSamplerDescriptor alloc] init];
    MTLSamplerDescriptor* desc = (MTLSamplerDescriptor*) texture->impl._samplerDesc;
	desc.minFilter = MTLSamplerMinMagFilterNearest;
	desc.magFilter = MTLSamplerMinMagFilterLinear;
	desc.sAddressMode = MTLSamplerAddressModeRepeat;
	desc.tAddressMode = MTLSamplerAddressModeRepeat;
	desc.mipFilter = MTLSamplerMipFilterNotMipmapped;
	desc.maxAnisotropy = 1U;
	desc.normalizedCoordinates = YES;
	desc.lodMinClamp = 0.0f;
	desc.lodMaxClamp = FLT_MAX;
	texture->impl._sampler = [device newSamplerStateWithDescriptor:desc];
}

/*void Graphics5::Texture::_init(const char* format, bool readable) {
	texWidth = width;
	texHeight = height;

	create(width, height, Image::RGBA32, false);
	lock();
	unlock();
}*/

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	//Image(width, height, format, readable);
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->texWidth = width;
	texture->texHeight = height;
	texture->format = format;
	texture->impl.data = malloc(width * height * (format == KINC_IMAGE_FORMAT_GREY8 ? 1 : 4));
	create(texture, width, height, format, true);
}

void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image *image) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->texWidth = image->width;
	texture->texHeight = image->height;
	texture->format = image->format;
	create(texture, image->width, image->height, image->format, true);
	id<MTLTexture> tex = texture->impl._tex;
	[tex replaceRegion:MTLRegionMake2D(0, 0, texture->texWidth, texture->texHeight) mipmapLevel:0 slice:0 withBytes:image->data bytesPerRow:kinc_g5_texture_stride(texture) bytesPerImage:kinc_g5_texture_stride(texture) * texture->texHeight];
}

void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {}

void kinc_g5_texture_destroy(kinc_g5_texture_t *texture) {
	texture->impl._sampler = nil;
	texture->impl._tex = nil;
	if (texture->impl.data != NULL) {
		free(texture->impl.data);
		texture->impl.data = NULL;
	}
}

id getMetalDevice();
id getMetalEncoder();

#if 0
void kinc_g5_internal_set_texture_descriptor(kinc_g5_texture_t *texture, kinc_g5_texture_descriptor_t descriptor) {
    MTLSamplerDescriptor* desc = (MTLSamplerDescriptor*) texture->impl._samplerDesc;
    switch(descriptor.filter_minification) {
        case KINC_G5_TEXTURE_FILTER_POINT:
            desc.minFilter = MTLSamplerMinMagFilterNearest;
            break;
        default:
            desc.minFilter = MTLSamplerMinMagFilterLinear;
    }

    switch(descriptor.filter_magnification) {
        case KINC_G5_TEXTURE_FILTER_POINT:
            desc.magFilter = MTLSamplerMinMagFilterNearest;
            break;
        default:
            desc.minFilter = MTLSamplerMinMagFilterLinear;
    }

    switch(descriptor.addressing_u) {
        case KINC_G5_TEXTURE_ADDRESSING_REPEAT:
            desc.sAddressMode = MTLSamplerAddressModeRepeat;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_MIRROR:
            desc.sAddressMode = MTLSamplerAddressModeMirrorRepeat;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_CLAMP:
            desc.sAddressMode = MTLSamplerAddressModeClampToEdge;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_BORDER:
            desc.sAddressMode = MTLSamplerAddressModeClampToBorderColor;
            break;
    }

    switch(descriptor.addressing_v) {
        case KINC_G5_TEXTURE_ADDRESSING_REPEAT:
            desc.tAddressMode = MTLSamplerAddressModeRepeat;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_MIRROR:
            desc.tAddressMode = MTLSamplerAddressModeMirrorRepeat;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_CLAMP:
            desc.tAddressMode = MTLSamplerAddressModeClampToEdge;
            break;
        case KINC_G5_TEXTURE_ADDRESSING_BORDER:
            desc.tAddressMode = MTLSamplerAddressModeClampToBorderColor;
            break;
    }
    id<MTLDevice> device = getMetalDevice();
    texture->impl._sampler = [device newSamplerStateWithDescriptor:desc];
}
#endif

void kinc_g5_internal_texture_set(kinc_g5_texture_t *texture, int unit) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setFragmentSamplerState:texture->impl._sampler atIndex:unit];
	[encoder setFragmentTexture:texture->impl._tex atIndex:unit];
}

int kinc_g5_texture_stride(kinc_g5_texture_t *texture) {
	if (texture->format == KINC_IMAGE_FORMAT_GREY8) {
		return texture->texWidth;
	}
	else {
		return texture->texWidth * 4;
	}
}
uint8_t *kinc_g5_texture_lock(kinc_g5_texture_t *texture) {
	return (uint8_t*)texture->impl.data;
}

void kinc_g5_texture_unlock(kinc_g5_texture_t *tex) {
	id<MTLTexture> texture = tex->impl._tex;
	[texture replaceRegion:MTLRegionMake2D(0, 0, tex->texWidth, tex->texHeight) mipmapLevel:0 slice:0 withBytes:tex->impl.data bytesPerRow:kinc_g5_texture_stride(tex) bytesPerImage:kinc_g5_texture_stride(tex) * tex->texHeight];
}

void kinc_g5_texture_clear(kinc_g5_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color) {}

void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels) {}

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_g5_texture_t *mipmap, int level) {}

#include <kinc/graphics4/texture.h>

#if defined(KORE_IOS) || defined(KORE_MACOS)
void kinc_g4_texture_upload(kinc_g4_texture_t *texture, uint8_t *data, int stride) {}
#endif
