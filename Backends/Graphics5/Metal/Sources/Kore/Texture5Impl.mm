#include "pch.h"

#include "Texture5Impl.h"

#include <Kore/Graphics1/Image.h>
#include <Kore/Graphics5/Graphics.h>
#include <Kore/Log.h>

#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();

namespace {
	MTLPixelFormat convert(Graphics1::Image::Format format) {
		switch (format) {
			case Graphics1::Image::RGBA32:
				return MTLPixelFormatRGBA8Unorm;
			case Graphics1::Image::Grey8:
				return MTLPixelFormatR8Unorm;
			case Graphics1::Image::RGB24:
			case Graphics1::Image::RGBA128:
			case Graphics1::Image::RGBA64:
			case Graphics1::Image::A32:
			case Graphics1::Image::BGRA32:
			case Graphics1::Image::A16:
				return MTLPixelFormatRGBA8Unorm;
		}
	}
}

void Graphics5::Texture::_init(const char* format, bool readable) {
	texWidth = width;
	texHeight = height;

	create(width, height, Image::RGBA32, false);
	lock();
	unlock();
}

Graphics5::Texture::Texture(int width, int height, Format format, bool readable) : Image(width, height, format, readable) {
	texWidth = width;
	texHeight = height;
	create(width, height, format, true);
}

Graphics5::Texture::Texture(int width, int height, int depth, Format format, bool readable) : Image(width, height, format, readable) {
	texWidth = width;
	texHeight = height;
	create(width, height, format, true);
}

Texture5Impl::Texture5Impl() : _tex(0), _sampler(0) {

}

Texture5Impl::~Texture5Impl() {
	_tex = 0;
	_sampler = 0;
}

void Texture5Impl::create(int width, int height, int format, bool writable) {
	id<MTLDevice> device = getMetalDevice();

	MTLTextureDescriptor* descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:convert((Graphics1::Image::Format)format) width:width height:height mipmapped:NO];
	descriptor.textureType = MTLTextureType2D;
	descriptor.width = width;
	descriptor.height = height;
	descriptor.depth = 1;
	descriptor.pixelFormat = convert((Graphics1::Image::Format)format);
	descriptor.arrayLength = 1;
	descriptor.mipmapLevelCount = 1;
	//TODO: Make less textures writable
	if (writable) {
		descriptor.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
	}
	
	_tex = [device newTextureWithDescriptor:descriptor];
	
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
	_sampler = [device newSamplerStateWithDescriptor:desc];
}

id getMetalDevice();
id getMetalEncoder();

void Graphics5::Texture::_set(TextureUnit unit) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setFragmentSamplerState:_sampler atIndex:unit.index];
	[encoder setFragmentTexture:_tex atIndex:unit.index];
}

int Graphics5::Texture::stride() {
	if (format == Graphics1::Image::Grey8) {
		return width;
	}
	else {
		return width * 4;
	}
}

u8* Graphics5::Texture::lock() {
	return (u8*)data;
}

void Graphics5::Texture::unlock() {
	id<MTLTexture> texture = _tex;
	[texture replaceRegion:MTLRegionMake2D(0, 0, width, height) mipmapLevel:0 slice:0 withBytes:data bytesPerRow:stride() bytesPerImage:stride() * height];
}

void Graphics5::Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {}

#ifdef SYS_IOS
void Graphics5::Texture::upload(u8* data) {}
#endif

void Graphics5::Texture::generateMipmaps(int levels) {}

void Graphics5::Texture::setMipmap(Texture* mipmap, int level) {}
