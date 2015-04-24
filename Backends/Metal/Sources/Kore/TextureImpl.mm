#include "pch.h"
#include "TextureImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Image.h>
#include <Kore/Log.h>
#import <Metal/Metal.h>

using namespace Kore;

void* getMetalDevice();

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
	id <MTLDevice> device = (__bridge_transfer id <MTLDevice>)getMetalDevice();
	
	MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
	descriptor.textureType = MTLTextureType2D;
	descriptor.height = width;
	descriptor.width = height;
	descriptor.depth = 1;
	descriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
	descriptor.arrayLength = 1;
	descriptor.mipmapLevelCount = 1;
	
	id <MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
	tex = (__bridge_retained void*)texture;
}

void Texture::set(TextureUnit unit) {

}

int Texture::stride() {
	return width * 4;
}

u8* Texture::lock() {
	return (u8*)data;
}

void Texture::unlock() {
	id <MTLTexture> texture = (__bridge_transfer id <MTLTexture>)tex;
	[texture replaceRegion:MTLRegionMake2D(0, 0, width, height) mipmapLevel:0 slice:0 withBytes:data bytesPerRow:stride() bytesPerImage:stride() * height];
}

#ifdef SYS_IOS
void Texture::upload(u8* data) {

}
#endif
