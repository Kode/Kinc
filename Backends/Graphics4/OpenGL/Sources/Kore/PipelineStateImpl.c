#include "pch.h"

#include "ogl.h"

#include <Kinc/Graphics4/Graphics.h>
#include <Kinc/Graphics4/PipelineState.h>
#include <Kinc/Graphics4/Shader.h>
#include <Kinc/Graphics4/VertexStructure.h>
#include <Kinc/Log.h>

#include <Kore/OpenGL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef KORE_OPENGL_ES
bool Kinc_Internal_ProgramUsesTessellation = false;
#endif
extern bool Kinc_Internal_SupportsConservativeRaster;

static GLenum convertStencilAction(Kinc_G4_StencilAction action) {
	switch (action) {
	default:
	case KINC_G4_STENCIL_DECREMENT:
		return GL_DECR;
	case KINC_G4_STENCIL_DECREMENT_WRAP:
		return GL_DECR_WRAP;
	case KINC_G4_STENCIL_INCREMENT:
		return GL_INCR;
	case KINC_G4_STENCIL_INCREMENT_WRAP:
		return GL_INCR_WRAP;
	case KINC_G4_STENCIL_INVERT:
		return GL_INVERT;
	case KINC_G4_STENCIL_KEEP:
		return GL_KEEP;
	case KINC_G4_STENCIL_REPLACE:
		return GL_REPLACE;
	case KINC_G4_STENCIL_ZERO:
		return GL_ZERO;
	}
}

static GLenum convertBlendingOperation(Kinc_G4_BlendingOperation operation) {
	switch (operation) {
	case KINC_G4_BLEND_ZERO:
		return GL_ZERO;
	case KINC_G4_BLEND_ONE:
		return GL_ONE;
	case KINC_G4_BLEND_SOURCE_ALPHA:
		return GL_SRC_ALPHA;
	case KINC_G4_BLEND_DEST_ALPHA:
		return GL_DST_ALPHA;
	case KINC_G4_BLEND_INV_SOURCE_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case KINC_G4_BLEND_INV_DEST_ALPHA:
		return GL_ONE_MINUS_DST_ALPHA;
	case KINC_G4_BLEND_SOURCE_COLOR:
		return GL_SRC_COLOR;
	case KINC_G4_BLEND_DEST_COLOR:
		return GL_DST_COLOR;
	case KINC_G4_BLEND_INV_SOURCE_COLOR:
		return GL_ONE_MINUS_SRC_COLOR;
	case KINC_G4_BLEND_INV_DEST_COLOR:
		return GL_ONE_MINUS_DST_COLOR;
	default:
		return GL_ONE;
	}
}

void Kinc_G4_PipelineState_Create(Kinc_G4_PipelineState *state) {
	memset(state, 0, sizeof(Kinc_G4_PipelineState));
	
	for (int i = 0; i < 16; ++i) state->inputLayout[i] = NULL;
	state->vertexShader = NULL;
	state->fragmentShader = NULL;
	state->geometryShader = NULL;
	state->tessellationControlShader = NULL;
	state->tessellationEvaluationShader = NULL;

	state->cullMode = KINC_G4_CULL_NOTHING;

	state->depthWrite = false;
	state->depthMode = KINC_G4_COMPARE_ALWAYS;

	state->stencilMode = KINC_G4_COMPARE_ALWAYS;
	state->stencilBothPass = KINC_G4_STENCIL_KEEP;
	state->stencilDepthFail = KINC_G4_STENCIL_KEEP;
	state->stencilFail = KINC_G4_STENCIL_KEEP;
	state->stencilReferenceValue = 0;
	state->stencilReadMask = 0xff;
	state->stencilWriteMask = 0xff;

	state->blendSource = KINC_G4_BLEND_ONE;
	state->blendDestination = KINC_G4_BLEND_ZERO;
	// blendOperation = BlendingOperation.Add;
	state->alphaBlendSource = KINC_G4_BLEND_ONE;
	state->alphaBlendDestination = KINC_G4_BLEND_ZERO;
	// alphaBlendOperation = BlendingOperation.Add;

	for (int i = 0; i < 8; ++i) state->colorWriteMaskRed[i] = true;
	for (int i = 0; i < 8; ++i) state->colorWriteMaskGreen[i] = true;
	for (int i = 0; i < 8; ++i) state->colorWriteMaskBlue[i] = true;
	for (int i = 0; i < 8; ++i) state->colorWriteMaskAlpha[i] = true;

	state->conservativeRasterization = false;

	state->impl.textureCount = 0;
	// TODO: Get rid of allocations
	state->impl.textures = (char**)malloc(sizeof(char*) * 16);
	for (int i = 0; i < 16; ++i) {
		state->impl.textures[i] = (char*)malloc(sizeof(char) * 128);
		state->impl.textures[i][0] = 0;
	}
	state->impl.textureValues = (int*)malloc(sizeof(int) * 16);

	state->impl.programId = glCreateProgram();
	glCheckErrors();
}

void Kinc_G4_PipelineState_Destroy(Kinc_G4_PipelineState *state) {
	for (int i = 0; i < 16; ++i) {
		free(state->impl.textures[i]);
	}
	free(state->impl.textures);
	free(state->impl.textureValues);
	glDeleteProgram(state->impl.programId);
}

static int toGlShader(Kinc_G4_ShaderType type) {
	switch (type) {
	case KINC_SHADER_TYPE_VERTEX:
	default:
		return GL_VERTEX_SHADER;
	case KINC_SHADER_TYPE_FRAGMENT:
		return GL_FRAGMENT_SHADER;
#ifndef KORE_OPENGL_ES
	case KINC_SHADER_TYPE_GEOMETRY:
		return GL_GEOMETRY_SHADER;
	case KINC_SHADER_TYPE_TESSELLATION_CONTROL:
		return GL_TESS_CONTROL_SHADER;
	case KINC_SHADER_TYPE_TESSELLATION_EVALUATION:
		return GL_TESS_EVALUATION_SHADER;
#endif
	}
}

static void compileShader(unsigned *id, const char* source, size_t length, Kinc_G4_ShaderType type) {
	*id = glCreateShader(toGlShader(type));
	glCheckErrors();
	glShaderSource(*id, 1, (const GLchar**)&source, 0);
	glCompileShader(*id);

	int result;
	glGetShaderiv(*id, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		int length;
		glGetShaderiv(*id, GL_INFO_LOG_LENGTH, &length);
		char* errormessage = (char*)malloc(length);
		glGetShaderInfoLog(*id, length, NULL, errormessage);
		printf("GLSL compiler error: %s\n", errormessage);
		free(errormessage);
	}
}

void Kinc_G4_PipelineState_Compile(Kinc_G4_PipelineState *state) {
	compileShader(&state->vertexShader->impl._glid, state->vertexShader->impl.source, state->vertexShader->impl.length, KINC_SHADER_TYPE_VERTEX);
	compileShader(&state->fragmentShader->impl._glid, state->fragmentShader->impl.source, state->fragmentShader->impl.length, KINC_SHADER_TYPE_FRAGMENT);
#ifndef OPENGLES
	if (state->geometryShader != NULL) {
		compileShader(&state->geometryShader->impl._glid, state->geometryShader->impl.source, state->geometryShader->impl.length, KINC_SHADER_TYPE_GEOMETRY);
	}
	if (state->tessellationControlShader != NULL) {
		compileShader(&state->tessellationControlShader->impl._glid, state->tessellationControlShader->impl.source,
		              state->tessellationControlShader->impl.length, KINC_SHADER_TYPE_TESSELLATION_CONTROL);
	}
	if (state->tessellationEvaluationShader != NULL) {
		compileShader(&state->tessellationEvaluationShader->impl._glid, state->tessellationEvaluationShader->impl.source,
		              state->tessellationEvaluationShader->impl.length, KINC_SHADER_TYPE_TESSELLATION_EVALUATION);
	}
#endif
	glAttachShader(state->impl.programId, state->vertexShader->impl._glid);
	glAttachShader(state->impl.programId, state->fragmentShader->impl._glid);
#ifndef OPENGLES
	if (state->geometryShader != NULL) {
		glAttachShader(state->impl.programId, state->geometryShader->impl._glid);
	}
	if (state->tessellationControlShader != NULL) {
		glAttachShader(state->impl.programId, state->tessellationControlShader->impl._glid);
	}
	if (state->tessellationEvaluationShader != NULL) {
		glAttachShader(state->impl.programId, state->tessellationEvaluationShader->impl._glid);
	}
#endif
	glCheckErrors();

	int index = 0;
	for (int i1 = 0; state->inputLayout[i1] != NULL; ++i1) {
		for (int i2 = 0; i2 < state->inputLayout[i1]->size; ++i2) {
			Kinc_G4_VertexElement element = state->inputLayout[i1]->elements[i2];
			glBindAttribLocation(state->impl.programId, index, element.name);
			glCheckErrors();
			if (element.data == KINC_G4_VERTEX_DATA_FLOAT4X4) {
				index += 4;
			}
			else {
				++index;
			}
		}
	}

	glLinkProgram(state->impl.programId);

	int result;
	glGetProgramiv(state->impl.programId, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		int length;
		glGetProgramiv(state->impl.programId, GL_INFO_LOG_LENGTH, &length);
		char* errormessage = (char*)malloc(length);
		glGetProgramInfoLog(state->impl.programId, length, NULL, errormessage);
		printf("GLSL linker error: %s\n", errormessage);
		free(errormessage);
	}

#ifndef KORE_OPENGL_ES
#ifndef KORE_LINUX
	if (state->tessellationControlShader != NULL) {
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glCheckErrors();
	}
#endif
#endif
}

void Kinc_G4_Internal_SetPipeline(Kinc_G4_PipelineState* pipeline) {
#ifndef KORE_OPENGL_ES
	Kinc_Internal_ProgramUsesTessellation = pipeline->tessellationControlShader != NULL;
#endif
	glUseProgram(pipeline->impl.programId);
	glCheckErrors();
	for (int index = 0; index < pipeline->impl.textureCount; ++index) {
		glUniform1i(pipeline->impl.textureValues[index], index);
		glCheckErrors();
	}

	if (pipeline->stencilMode == KINC_G4_COMPARE_ALWAYS && pipeline->stencilBothPass == KINC_G4_STENCIL_KEEP &&
	    pipeline->stencilDepthFail == KINC_G4_STENCIL_KEEP && pipeline->stencilFail == KINC_G4_STENCIL_KEEP) {
		glDisable(GL_STENCIL_TEST);
	}
	else {
		glEnable(GL_STENCIL_TEST);
		int stencilFunc = Kinc_G4_Internal_StencilFunc(pipeline->stencilMode);
		glStencilMask(pipeline->stencilWriteMask);
		glStencilOp(convertStencilAction(pipeline->stencilFail), convertStencilAction(pipeline->stencilDepthFail), convertStencilAction(pipeline->stencilBothPass));
		glStencilFunc(stencilFunc, pipeline->stencilReferenceValue, pipeline->stencilReadMask);
	}

	#ifdef KORE_OPENGL_ES
	glColorMask(pipeline->colorWriteMaskRed[0], pipeline->colorWriteMaskGreen[0], pipeline->colorWriteMaskBlue[0], pipeline->colorWriteMaskAlpha[0]);
	#else
	for (int i = 0; i < 8; ++i) glColorMaski(i, pipeline->colorWriteMaskRed[i], pipeline->colorWriteMaskGreen[i], pipeline->colorWriteMaskBlue[i], pipeline->colorWriteMaskAlpha[i]);
	#endif

	if (Kinc_Internal_SupportsConservativeRaster) {
		if (pipeline->conservativeRasterization) {
			glEnable(0x9346); // GL_CONSERVATIVE_RASTERIZATION_NV
		}
		else {
			glDisable(0x9346);
		}
	}

	glCheckErrors();

	/*switch (state) {
	case Normalize:
	device->SetRenderState(D3DRS_NORMALIZENORMALS, on ? TRUE : FALSE);
	break;
	case BackfaceCulling:
	if (on) device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	else device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	break;
	case FogState:
	device->SetRenderState(D3DRS_FOGENABLE, on ? TRUE : FALSE);
	break;
	case ScissorTestState:
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, on ? TRUE : FALSE);
	break;
	case AlphaTestState:
	device->SetRenderState(D3DRS_ALPHATESTENABLE, on ? TRUE : FALSE);
	device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	break;
	default:
	throw Exception();
	}*/

	if (pipeline->depthWrite) {
		glDepthMask(GL_TRUE);
	}
	else {
		glDepthMask(GL_FALSE);
	}

	if (pipeline->depthMode != KINC_G4_COMPARE_ALWAYS) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	GLenum func = GL_ALWAYS;
	switch (pipeline->depthMode) {
	default:
	case KINC_G4_COMPARE_ALWAYS:
		func = GL_ALWAYS;
		break;
	case KINC_G4_COMPARE_NEVER:
		func = GL_NEVER;
		break;
	case KINC_G4_COMPARE_EQUAL:
		func = GL_EQUAL;
		break;
	case KINC_G4_COMPARE_NOT_EQUAL:
		func = GL_NOTEQUAL;
		break;
	case KINC_G4_COMPARE_LESS:
		func = GL_LESS;
		break;
	case KINC_G4_COMPARE_LESS_EQUAL:
		func = GL_LEQUAL;
		break;
	case KINC_G4_COMPARE_GREATER:
		func = GL_GREATER;
		break;
	case KINC_G4_COMPARE_GREATER_EQUAL:
		func = GL_GEQUAL;
		break;
	}
	glDepthFunc(func);
	glCheckErrors();

	switch (pipeline->cullMode) {
	case KINC_G4_CULL_CLOCKWISE:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glCheckErrors();
		break;
	case KINC_G4_CULL_COUNTER_CLOCKWISE:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glCheckErrors();
		break;
	case KINC_G4_CULL_NOTHING:
		glDisable(GL_CULL_FACE);
		glCheckErrors();
		break;
	default:
		break;
	}

	/*switch (state) {
	case DepthTestCompare:
	switch (v) {
	// TODO: Cmp-Konstanten systemabhaengig abgleichen
	default:
	case ZCmp_Always      : v = D3DCMP_ALWAYS; break;
	case ZCmp_Never       : v = D3DCMP_NEVER; break;
	case ZCmp_Equal       : v = D3DCMP_EQUAL; break;
	case ZCmp_NotEqual    : v = D3DCMP_NOTEQUAL; break;
	case ZCmp_Less        : v = D3DCMP_LESS; break;
	case ZCmp_LessEqual   : v = D3DCMP_LESSEQUAL; break;
	case ZCmp_Greater     : v = D3DCMP_GREATER; break;
	case ZCmp_GreaterEqual: v = D3DCMP_GREATEREQUAL; break;
	}
	device->SetRenderState(D3DRS_ZFUNC, v);
	break;
	case FogTypeState:
	switch (v) {
	case LinearFog:
	device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	}
	break;
	case AlphaReferenceState:
	device->SetRenderState(D3DRS_ALPHAREF, (DWORD)v);
	break;
	default:
	throw Exception();
	}*/

	if (pipeline->blendSource != KINC_G4_BLEND_ONE || pipeline->blendDestination != KINC_G4_BLEND_ZERO || pipeline->alphaBlendSource != KINC_G4_BLEND_ONE ||
	    pipeline->alphaBlendDestination != KINC_G4_BLEND_ZERO) {
		glEnable(GL_BLEND);
	}
	else {
		glDisable(GL_BLEND);
	}

	// glBlendFunc(convert(pipeline->blendSource), convert(pipeline->blendDestination));
	glBlendFuncSeparate(convertBlendingOperation(pipeline->blendSource), convertBlendingOperation(pipeline->blendDestination),
	                    convertBlendingOperation(pipeline->alphaBlendSource), convertBlendingOperation(pipeline->alphaBlendDestination));
}

Kinc_G4_ConstantLocation Kinc_G4_PipelineState_GetConstantLocation(Kinc_G4_PipelineState *state, const char *name) {
	Kinc_G4_ConstantLocation location;
	location.impl.location = glGetUniformLocation(state->impl.programId, name);
	location.impl.type = GL_FLOAT;
	GLint count = 0;
	glGetProgramiv(state->impl.programId, GL_ACTIVE_UNIFORMS, &count);
	char arrayName[1024];
	strcpy(arrayName, name);
	strcat(arrayName, "[0]");
	for (GLint i = 0; i < count; ++i) {
		GLenum type;
		char uniformName[1024];
		GLsizei length;
		GLint size;
		glGetActiveUniform(state->impl.programId, i, 1024 - 1, &length, &size, &type, uniformName);
		if (strcmp(uniformName, name) == 0 || strcmp(uniformName, arrayName) == 0) {
			location.impl.type = type;
			break;
		}
	}
	glCheckErrors();
	if (location.impl.location < 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Uniform %s not found.", name);
	}
	return location;
}

static int findTexture(Kinc_G4_PipelineState *state, const char *name) {
	for (int index = 0; index < state->impl.textureCount; ++index) {
		if (strcmp(state->impl.textures[index], name) == 0) return index;
	}
	return -1;
}

Kinc_G4_TextureUnit Kinc_G4_PipelineState_GetTextureUnit(Kinc_G4_PipelineState *state, const char *name) {
	int index = findTexture(state, name);
	if (index < 0) {
		int location = glGetUniformLocation(state->impl.programId, name);
		glCheckErrors();
		index = state->impl.textureCount;
		state->impl.textureValues[index] = location;
		strcpy(state->impl.textures[index], name);
		++state->impl.textureCount;
	}
	Kinc_G4_TextureUnit unit;
	unit.impl.unit = index;
	return unit;
}
