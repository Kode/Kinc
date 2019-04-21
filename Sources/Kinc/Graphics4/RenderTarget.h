#pragma once

#include <Kore/RenderTargetImpl.h>

#include "TextureUnit.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	KINC_G4_RENDER_TARGET_FORMAT_32BIT,
	KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT,
	KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT,
	KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT,
	KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH,
	KINC_G4_RENDER_TARGET_FORMAT_8BIT_RED,
	KINC_G4_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT
} Kinc_G4_RenderTargetFormat;

typedef struct {
	int width;
	int height;
	int texWidth;
	int texHeight;
	int contextId;
	bool isCubeMap;
	bool isDepthAttachment;

	Kinc_G4_RenderTargetImpl impl;
} Kinc_G4_RenderTarget;

void Kinc_G4_RenderTarget_Create(Kinc_G4_RenderTarget *renderTarget, int width, int height, int depthBufferBits, bool antialiasing,
                                 Kinc_G4_RenderTargetFormat format, int stencilBufferBits, int contextId);

void Kinc_G4_RenderTarget_CreateCube(Kinc_G4_RenderTarget *renderTarget, int cubeMapSize, int depthBufferBits, bool antialiasing,
                                     Kinc_G4_RenderTargetFormat format, int stencilBufferBits, int contextId);

void Kinc_G4_RenderTarget_Destroy(Kinc_G4_RenderTarget *renderTarget);

void Kinc_G4_RenderTarget_UseColorAsTexture(Kinc_G4_RenderTarget *renderTarget, Kinc_G4_TextureUnit unit);
void Kinc_G4_RenderTarget_UseDepthAsTexture(Kinc_G4_RenderTarget *renderTarget, Kinc_G4_TextureUnit unit);
void Kinc_G4_RenderTarget_SetDepthStencilFrom(Kinc_G4_RenderTarget *renderTarget, Kinc_G4_RenderTarget *source);
void Kinc_G4_RenderTarget_GetPixels(Kinc_G4_RenderTarget *renderTarget, uint8_t *data);
void Kinc_G4_RenderTarget_GenerateMipmaps(Kinc_G4_RenderTarget *renderTarget, int levels);

#ifdef __cplusplus
}
#endif
