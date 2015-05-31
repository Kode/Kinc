#include "pch.h"
#include "TextureImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Image.h>
#include <Kore/Log.h>
#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();

Texture::Texture(const char* filename, bool readable) : Image(filename, readable) {
	texWidth = width;
	texHeight = height;

	create(width, height);
	lock();
	unlock();
}

Texture::Texture(int width, int height, Image::Format format, bool readable) : Image(width, height, format, readable) {
	texWidth = width;
	texHeight = height;
	create(width, height);
}

TextureImpl::~TextureImpl() {

}

void TextureImpl::create(int width, int height) {
	id <MTLDevice> device = getMetalDevice();
	
	MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
	descriptor.textureType = MTLTextureType2D;
	descriptor.width = width;
	descriptor.height = height;
	descriptor.depth = 1;
	descriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
	descriptor.arrayLength = 1;
	descriptor.mipmapLevelCount = 1;
	
	tex = [device newTextureWithDescriptor:descriptor];
}

id getMetalDevice();
id getMetalEncoder();

void Texture::set(TextureUnit unit) {
	id <MTLDevice> device = getMetalDevice();
	MTLSamplerDescriptor* desc = [[MTLSamplerDescriptor alloc] init];
	desc.minFilter = MTLSamplerMinMagFilterLinear;
	desc.magFilter = MTLSamplerMinMagFilterLinear;
	desc.sAddressMode = MTLSamplerAddressModeRepeat;
	desc.tAddressMode = MTLSamplerAddressModeRepeat;
	desc.mipFilter = MTLSamplerMipFilterNotMipmapped;
	desc.maxAnisotropy = 1U;
	desc.normalizedCoordinates = YES;
	desc.lodMinClamp = 0.0f;
	desc.lodMaxClamp = FLT_MAX;
	id <MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:desc];
	
	id <MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setFragmentSamplerState:sampler atIndex:unit.index];
	[encoder setFragmentTexture:tex atIndex:unit.index];
}

int Texture::stride() {
	return width * 4;
}

u8* Texture::lock() {
	return (u8*)data;
}

void Texture::unlock() {
	id <MTLTexture> texture = tex;
	[texture replaceRegion:MTLRegionMake2D(0, 0, width, height) mipmapLevel:0 slice:0 withBytes:data bytesPerRow:stride() bytesPerImage:stride() * height];
}

#ifdef SYS_IOS
void Texture::upload(u8* data) {

}
#endif
