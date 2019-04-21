#pragma once

#include <Kinc/Math/Matrix.h>

#include "ConstantLocation.h"
#include "TextureUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _Kinc_G4_PipelineState;
struct _Kinc_G4_RenderTarget;
struct _Kinc_G4_Texture;
struct _Kinc_G4_TextureArray;

typedef enum {
	KINC_G4_TEXTURE_ADDRESSING_REPEAT,
	KINC_G4_TEXTURE_ADDRESSING_MIRROR,
	KINC_G4_TEXTURE_ADDRESSING_CLAMP,
	KINC_G4_TEXTURE_ADDRESSING_BORDER
} Kinc_G4_TextureAddressing;

typedef enum {
	KINC_G4_TEXTURE_DIRECTION_U,
	KINC_G4_TEXTURE_DIRECTION_V,
	KINC_G4_TEXTURE_DIRECTION_W
} Kinc_G4_TextureDirection;

typedef enum {
	KINC_G4_TEXTURE_OPERATION_MODULATE,
	KINC_G4_TEXTURE_OPERATION_SELECT_FIRST,
	KINC_G4_TEXTURE_OPERATION_SELECT_SECOND
} Kinc_G4_TextureOperation;

typedef enum {
	KINC_G4_TEXTURE_ARGUMENT_CURRENT_COLOR,
	KINC_G4_TEXTURE_ARGUMENT_TEXTURE_COLOR
} Kinc_G4_TextureArgument;

typedef enum {
	KINC_G4_TEXTURE_FILTER_POINT,
	KINC_G4_TEXTURE_FILTER_LINEAR,
	KINC_G4_TEXTURE_FILTER_ANISOTROPIC
} Kinc_G4_TextureFilter;

typedef enum {
	KINC_G4_MIPMAP_FILTER_NONE,
	KINC_G4_MIPMAP_FILTER_POINT,
	KINC_G4_MIPMAP_FILTER_LINEAR // linear texture filter + linear mip filter -> trilinear filter
} Kinc_G4_MipmapFilter;

void Kinc_G4_Init(int window, int depthBufferBits, int stencilBufferBits, bool vSync);

void Kinc_G4_Destroy(int window);

void Kinc_G4_Flush();

void Kinc_G4_Begin(int window);

void Kinc_G4_End(int window);

bool Kinc_G4_SwapBuffers();

#define KINC_G4_CLEAR_COLOR   1
#define KINC_G4_CLEAR_DEPTH   2
#define KINC_G4_CLEAR_STENCIL 4

void Kinc_G4_Clear(unsigned flags, unsigned color, float depth, int stencil);

void Kinc_G4_Viewport(int x, int y, int width, int height);

void Kinc_G4_Scissor(int x, int y, int width, int height);

void Kinc_G4_DisableScissor();

void Kinc_G4_DrawIndexedVertices();

void Kinc_G4_DrawIndexedVerticesFromTo(int start, int count);

void Kinc_G4_DrawIndexedVerticesInstanced(int instanceCount);

void Kinc_G4_DrawIndexedVerticesInstancedFromTo(int instanceCount, int start, int count);

void Kinc_G4_SetTextureAddressing(Kinc_G4_TextureUnit unit, Kinc_G4_TextureDirection dir, Kinc_G4_TextureAddressing addressing);

void Kinc_G4_SetTexture3DAddressing(Kinc_G4_TextureUnit unit, Kinc_G4_TextureDirection dir, Kinc_G4_TextureAddressing addressing);

void Kinc_G4_SetPipeline(_Kinc_G4_PipelineState *pipeline);

void Kinc_G4_SetStencilReferenceValue(int value);

void Kinc_G4_SetTextureOperation(Kinc_G4_TextureOperation operation, Kinc_G4_TextureArgument arg1, Kinc_G4_TextureArgument arg2);

void Kinc_G4_SetInt(Kinc_G4_ConstantLocation location, int value);
void Kinc_G4_SetFloat(Kinc_G4_ConstantLocation location, float value);
void Kinc_G4_SetFloat2(Kinc_G4_ConstantLocation location, float value1, float value2);
void Kinc_G4_SetFloat3(Kinc_G4_ConstantLocation location, float value1, float value2, float value3);
void Kinc_G4_SetFloat4(Kinc_G4_ConstantLocation location, float value1, float value2, float value3, float value4);
void Kinc_G4_SetFloats(Kinc_G4_ConstantLocation location, float *values, int count);
void Kinc_G4_SetBool(Kinc_G4_ConstantLocation location, bool value);
void Kinc_G4_SetMatrix3(Kinc_G4_ConstantLocation location, Kinc_Matrix3x3 *value);
void Kinc_G4_SetMatrix4(Kinc_G4_ConstantLocation location, Kinc_Matrix4x4 *value);

void Kinc_G4_SetTextureMagnificationFilter(Kinc_G4_TextureUnit unit, Kinc_G4_TextureFilter filter);

void Kinc_G4_SetTexture3DMagnificationFilter(Kinc_G4_TextureUnit texunit, Kinc_G4_TextureFilter filter);

void Kinc_G4_SetTextureMinificationFilter(Kinc_G4_TextureUnit unit, Kinc_G4_TextureFilter filter);

void Kinc_G4_SetTexture3DMinificationFilter(Kinc_G4_TextureUnit texunit, Kinc_G4_TextureFilter filter);

void Kinc_G4_SetTextureMipmapFilter(Kinc_G4_TextureUnit unit, Kinc_G4_MipmapFilter filter);

void Kinc_G4_SetTexture3DMipmapFilter(Kinc_G4_TextureUnit texunit, Kinc_G4_MipmapFilter filter);

void Kinc_G4_SetTextureCompareMode(Kinc_G4_TextureUnit unit, bool enabled);

void Kinc_G4_SetCubeMapCompareMode(Kinc_G4_TextureUnit unit, bool enabled);

bool Kinc_G4_RenderTargetsInvertedY();

bool Kinc_G4_NonPow2TexturesSupported();

void Kinc_G4_RestoreRenderTarget();

void Kinc_G4_SetRenderTargets(_Kinc_G4_RenderTarget **targets, int count);

void Kinc_G4_SetRenderTargetFace(_Kinc_G4_RenderTarget *texture, int face);

void Kinc_G4_SetTexture(Kinc_G4_TextureUnit unit, _Kinc_G4_Texture *texture);

void Kinc_G4_SetImageTexture(Kinc_G4_TextureUnit unit, _Kinc_G4_Texture *texture);

bool Kinc_G4_InitOcclusionQuery(unsigned *occlusionQuery);

void Kinc_G4_DeleteOcclusionQuery(unsigned occlusionQuery);

void Kinc_G4_StartOcclusionQuery(unsigned occlusionQuery);

void Kinc_G4_EndOcclusionQuery(unsigned occlusionQuery);

bool Kinc_G4_AreQueryResultsAvailable(unsigned occlusionQuery);

void Kinc_G4_GetQueryResults(unsigned occlusionQuery, unsigned *pixelCount);

void Kinc_G4_SetTextureArray(Kinc_G4_TextureUnit unit, _Kinc_G4_TextureArray *array);

int Kinc_G4_AntialiasingSamples();

void Kinc_G4_SetAntialiasingSamples(int samples);

#ifdef __cplusplus
}
#endif
