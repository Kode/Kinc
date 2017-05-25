#include "pch.h"

#include "Texture5Impl.h"

#include <Kore/Graphics5/Graphics.h>
#include <Kore/Graphics1/Image.h>
#include <Kore/Log.h>

#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();

void Graphics5::Texture::_init(const char* format, bool readable) {
	texWidth = width;
	texHeight = height;

	create(width, height);
	lock();
	unlock();
}

Graphics5::Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {
	texWidth = width;
	texHeight = height;
	create(width, height);
}

Graphics5::Texture::Texture(int width, int height, int depth, Format format, bool readable) : Image(width, height, format, readable) {
	texWidth = width;
	texHeight = height;
	create(width, height);
}

Texture5Impl::~Texture5Impl() {}

void Texture5Impl::create(int width, int height) {
	id<MTLDevice> device = getMetalDevice();

	MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
	descriptor.textureType = MTLTextureType2D;
	descriptor.width = width;
	descriptor.height = height;
	descriptor.depth = 1;
	descriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;
	descriptor.arrayLength = 1;
	descriptor.mipmapLevelCount = 1;

	tex = [device newTextureWithDescriptor:descriptor];
}

id getMetalDevice();
id getMetalEncoder();

void Graphics5::Texture::_set(TextureUnit unit) {
	id<MTLDevice> device = getMetalDevice();
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
	id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:desc];

	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setFragmentSamplerState:sampler atIndex:unit.index];
	[encoder setFragmentTexture:tex atIndex:unit.index];
}

int Graphics5::Texture::stride() {
	return width * 4;
}

u8* Graphics5::Texture::lock() {
	return (u8*)data;
}

void Graphics5::Texture::unlock() {
	id<MTLTexture> texture = tex;
	[texture replaceRegion:MTLRegionMake2D(0, 0, width, height) mipmapLevel:0 slice:0 withBytes:data bytesPerRow:stride() bytesPerImage:stride() * height];
}

void Graphics5::Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {

}

#ifdef SYS_IOS
void Graphics5::Texture::upload(u8* data) {}
#endif

void Graphics5::Texture::generateMipmaps(int levels) {}

void Graphics5::Texture::setMipmap(Texture* mipmap, int level) {}
