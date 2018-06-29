#include "pch.h"

#include "RenderTarget5Impl.h"

#include <Kore/Graphics5/Graphics.h>

#import <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();
id getMetalEncoder();

Graphics5::RenderTarget::RenderTarget(int width, int height, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) : width(width), height(height), contextId(contextId) {
	texWidth = width;
	texHeight = height;
		
	id<MTLDevice> device = getMetalDevice();
		
	MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
	descriptor.textureType = MTLTextureType2D;
	descriptor.width = width;
	descriptor.height = height;
	descriptor.depth = 1;
	descriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
	descriptor.arrayLength = 1;
	descriptor.mipmapLevelCount = 1;
	descriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
	descriptor.resourceOptions = MTLResourceStorageModePrivate;
	
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
	
	MTLTextureDescriptor* depthDescriptor = [MTLTextureDescriptor new];
	depthDescriptor.textureType = MTLTextureType2D;
	depthDescriptor.width = width;
	depthDescriptor.height = height;
	depthDescriptor.depth = 1;
#ifdef KORE_IOS
	depthDescriptor.pixelFormat = MTLPixelFormatDepth32Float_Stencil8;
#else
	depthDescriptor.pixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
#endif
	depthDescriptor.arrayLength = 1;
	depthDescriptor.mipmapLevelCount = 1;
	depthDescriptor.usage = MTLTextureUsageRenderTarget;
	depthDescriptor.resourceOptions = MTLResourceStorageModePrivate;
	
	_depthTex = [device newTextureWithDescriptor:depthDescriptor];
}

Graphics5::RenderTarget::RenderTarget(int cubeMapSize, int depthBufferBits, bool antialiasing, RenderTargetFormat format, int stencilBufferBits, int contextId) {}

Graphics5::RenderTarget::~RenderTarget() {
	
}

void Graphics5::RenderTarget::useColorAsTexture(TextureUnit unit) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setFragmentSamplerState:_sampler atIndex:unit.index];
	[encoder setFragmentTexture:_tex atIndex:unit.index];
}

void Graphics5::RenderTarget::useDepthAsTexture(Graphics5::TextureUnit unit) {}

void Graphics5::RenderTarget::setDepthStencilFrom(Graphics5::RenderTarget* source) {}
