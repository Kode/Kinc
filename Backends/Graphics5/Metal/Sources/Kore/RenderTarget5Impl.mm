#include "pch.h"

#include "RenderTarget5Impl.h"

#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/rendertarget.h>

#import <Metal/Metal.h>

id getMetalDevice();
id getMetalEncoder();

void kinc_g5_render_target_init(kinc_g5_render_target_t *target, int width, int height, int depthBufferBits, bool antialiasing,
								kinc_g5_render_target_format_t format, int stencilBufferBits, int contextId) {
	memset(target, 0, sizeof(kinc_g5_render_target_t));
	
	target->texWidth = width;
	target->texHeight = height;
	
	target->contextId = contextId;
		
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
	
	target->impl._tex = [device newTextureWithDescriptor:descriptor];
	
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
	target->impl._sampler = [device newSamplerStateWithDescriptor:desc];
	
	MTLTextureDescriptor* depthDescriptor = [MTLTextureDescriptor new];
	depthDescriptor.textureType = MTLTextureType2D;
	depthDescriptor.width = width;
	depthDescriptor.height = height;
	depthDescriptor.depth = 1;
	depthDescriptor.pixelFormat = MTLPixelFormatDepth32Float_Stencil8;
	depthDescriptor.arrayLength = 1;
	depthDescriptor.mipmapLevelCount = 1;
	depthDescriptor.usage = MTLTextureUsageRenderTarget;
	depthDescriptor.resourceOptions = MTLResourceStorageModePrivate;
	
	target->impl._depthTex = [device newTextureWithDescriptor:depthDescriptor];
}

void kinc_g5_render_target_init_cube(kinc_g5_render_target_t *target, int cubeMapSize, int depthBufferBits, bool antialiasing,
									 kinc_g5_render_target_format_t format, int stencilBufferBits, int contextId) {
	target->impl._tex = 0;
	target->impl._sampler = 0;
	target->impl._depthTex = 0;
}

void kinc_g5_render_target_destroy(kinc_g5_render_target_t *target) {
	target->impl._tex = 0;
	target->impl._sampler = 0;
	target->impl._depthTex = 0;
}

void kinc_g5_render_target_use_color_as_texture(kinc_g5_render_target_t *target, kinc_g5_texture_unit_t unit) {
	id<MTLRenderCommandEncoder> encoder = getMetalEncoder();
	[encoder setFragmentSamplerState:target->impl._sampler atIndex:unit.impl.index];
	[encoder setFragmentTexture:target->impl._tex atIndex:unit.impl.index];
}

void kinc_g5_render_target_use_depth_as_texture(kinc_g5_render_target_t *target, kinc_g5_texture_unit_t unit) {}

void kinc_g5_render_target_set_depth_stencil_from(kinc_g5_render_target_t *target, kinc_g5_render_target_t *source) {}
