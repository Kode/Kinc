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
	
	MTLSamplerDescriptor* desc = [[MTLSamplerDescriptor alloc] init];
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

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format, bool readable) {
	//Image(width, height, format, readable);
	texture->impl._tex = 0;
	texture->impl._sampler = 0;
	texture->texWidth = width;
	texture->texHeight = height;
	texture->format = format;
	create(texture, width, height, format, true);
}

void kinc_g5_texture_destroy(kinc_g5_texture_t *texture) {}

id getMetalDevice();
id getMetalEncoder();

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
	return NULL; //**(u8*)data;
}

void kinc_g5_texture_unlock(kinc_g5_texture_t *tex) {
	id<MTLTexture> texture = tex->impl._tex;
	[texture replaceRegion:MTLRegionMake2D(0, 0, tex->texWidth, tex->texHeight) mipmapLevel:0 slice:0 withBytes:NULL/**data*/ bytesPerRow:kinc_g5_texture_stride(tex) bytesPerImage:kinc_g5_texture_stride(tex) * tex->texHeight];
}

void kinc_g5_texture_clear(kinc_g5_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color) {}

#ifdef SYS_IOS
void kinc_g5_texture_upload(kinc_g5_texture_t *texture, uint8_t *data) {}
#endif

void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels) {}

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_g5_texture_t *mipmap, int level) {}
