#include "pch.h"

#include "ogl.h"

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/Shader.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Kore;

namespace Kore {
#ifndef KORE_OPENGL_ES
	bool programUsesTessellation = false;
#endif
}

namespace {
	GLenum convert(Graphics4::StencilAction action) {
		switch (action) {
		default:
		case Graphics4::Decrement:
			return GL_DECR;
		case Graphics4::DecrementWrap:
			return GL_DECR_WRAP;
		case Graphics4::Increment:
			return GL_INCR;
		case Graphics4::IncrementWrap:
			return GL_INCR_WRAP;
		case Graphics4::Invert:
			return GL_INVERT;
		case Graphics4::Keep:
			return GL_KEEP;
		case Graphics4::Replace:
			return GL_REPLACE;
		case Graphics4::Zero:
			return GL_ZERO;
		}
	}

	GLenum convert(Graphics4::BlendingOperation operation) {
		switch (operation) {
		case Graphics4::BlendZero:
			return GL_ZERO;
		case Graphics4::BlendOne:
			return GL_ONE;
		case Graphics4::SourceAlpha:
			return GL_SRC_ALPHA;
		case Graphics4::DestinationAlpha:
			return GL_DST_ALPHA;
		case Graphics4::InverseSourceAlpha:
			return GL_ONE_MINUS_SRC_ALPHA;
		case Graphics4::InverseDestinationAlpha:
			return GL_ONE_MINUS_DST_ALPHA;
		case Graphics4::SourceColor:
			return GL_SRC_COLOR;
		case Graphics4::DestinationColor:
			return GL_DST_COLOR;
		case Graphics4::InverseSourceColor:
			return GL_ONE_MINUS_SRC_COLOR;
		case Graphics4::InverseDestinationColor:
			return GL_ONE_MINUS_DST_COLOR;
		default:
			return GL_ONE;
		}
	}
}

PipelineStateImpl::PipelineStateImpl()
    : textureCount(0) {
	// TODO: Get rid of allocations
	textures = new char*[16];
	for (int i = 0; i < 16; ++i) {
		textures[i] = new char[128];
		textures[i][0] = 0;
	}
	textureValues = new int[16];

	programId = glCreateProgram();
	glCheckErrors();
}

PipelineStateImpl::~PipelineStateImpl() {
	for (int i = 0; i < 16; ++i) {
		delete[] textures[i];
	}
	delete[] textures;
	delete[] textureValues;
	glDeleteProgram(programId);
}

namespace {
	int toGlShader(Graphics4::ShaderType type) {
		switch (type) {
		case Graphics4::VertexShader:
		default:
			return GL_VERTEX_SHADER;
		case Graphics4::FragmentShader:
			return GL_FRAGMENT_SHADER;
#ifndef KORE_OPENGL_ES
		case Graphics4::GeometryShader:
			return GL_GEOMETRY_SHADER;
		case Graphics4::TessellationControlShader:
			return GL_TESS_CONTROL_SHADER;
		case Graphics4::TessellationEvaluationShader:
			return GL_TESS_EVALUATION_SHADER;
#endif
		}
	}

	void compileShader(uint& id, const char* source, int length, Graphics4::ShaderType type) {
		id = glCreateShader(toGlShader(type));
		glCheckErrors();
		glShaderSource(id, 1, (const GLchar**)&source, 0);
		glCompileShader(id);

		int result;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);
		if (result != GL_TRUE) {
			int length;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
			char* errormessage = new char[length];
			glGetShaderInfoLog(id, length, nullptr, errormessage);
			printf("GLSL compiler error: %s\n", errormessage);
			delete[] errormessage;
		}
	}
}

void Graphics4::PipelineState::compile() {
	compileShader(vertexShader->id, vertexShader->source, vertexShader->length, VertexShader);
	compileShader(fragmentShader->id, fragmentShader->source, fragmentShader->length, FragmentShader);
#ifndef OPENGLES
	if (geometryShader != nullptr) compileShader(geometryShader->id, geometryShader->source, geometryShader->length, GeometryShader);
	if (tessellationControlShader != nullptr)
		compileShader(tessellationControlShader->id, tessellationControlShader->source, tessellationControlShader->length, TessellationControlShader);
	if (tessellationEvaluationShader != nullptr)
		compileShader(tessellationEvaluationShader->id, tessellationEvaluationShader->source, tessellationEvaluationShader->length,
		              TessellationEvaluationShader);
#endif
	glAttachShader(programId, vertexShader->id);
	glAttachShader(programId, fragmentShader->id);
#ifndef OPENGLES
	if (geometryShader != nullptr) glAttachShader(programId, geometryShader->id);
	if (tessellationControlShader != nullptr) glAttachShader(programId, tessellationControlShader->id);
	if (tessellationEvaluationShader != nullptr) glAttachShader(programId, tessellationEvaluationShader->id);
#endif
	glCheckErrors();

	int index = 0;
	for (int i1 = 0; inputLayout[i1] != nullptr; ++i1) {
		for (int i2 = 0; i2 < inputLayout[i1]->size; ++i2) {
			VertexElement element = inputLayout[i1]->elements[i2];
			glBindAttribLocation(programId, index, element.name);
			glCheckErrors();
			if (element.data == Float4x4VertexData) {
				index += 4;
			}
			else {
				++index;
			}
		}
	}

	glLinkProgram(programId);

	int result;
	glGetProgramiv(programId, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		int length;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);
		char* errormessage = new char[length];
		glGetProgramInfoLog(programId, length, nullptr, errormessage);
		printf("GLSL linker error: %s\n", errormessage);
		delete[] errormessage;
	}

#ifndef KORE_OPENGL_ES
#ifndef KORE_LINUX
	if (tessellationControlShader != nullptr) {
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glCheckErrors();
	}
#endif
#endif
}

void PipelineStateImpl::set(Graphics4::PipelineState* pipeline) {
#ifndef KORE_OPENGL_ES
	programUsesTessellation = pipeline->tessellationControlShader != nullptr;
#endif
	glUseProgram(programId);
	glCheckErrors();
	for (int index = 0; index < textureCount; ++index) {
		glUniform1i(textureValues[index], index);
		glCheckErrors();
	}

	if (pipeline->stencilMode == Graphics4::ZCompareAlways && pipeline->stencilBothPass == Graphics4::Keep && pipeline->stencilDepthFail == Graphics4::Keep && pipeline->stencilFail == Graphics4::Keep) {
		glDisable(GL_STENCIL_TEST);
	}
	else {
		glEnable(GL_STENCIL_TEST);
		int stencilFunc = 0;
		switch (pipeline->stencilMode) {
		case Graphics4::ZCompareAlways:
			stencilFunc = GL_ALWAYS;
			break;
		case Graphics4::ZCompareEqual:
			stencilFunc = GL_EQUAL;
			break;
		case Graphics4::ZCompareGreater:
			stencilFunc = GL_GREATER;
			break;
		case Graphics4::ZCompareGreaterEqual:
			stencilFunc = GL_GEQUAL;
			break;
		case Graphics4::ZCompareLess:
			stencilFunc = GL_LESS;
			break;
		case Graphics4::ZCompareLessEqual:
			stencilFunc = GL_LEQUAL;
			break;
		case Graphics4::ZCompareNever:
			stencilFunc = GL_NEVER;
			break;
		case Graphics4::ZCompareNotEqual:
			stencilFunc = GL_NOTEQUAL;
			break;
		}
		glStencilMask(pipeline->stencilWriteMask);
		glStencilOp(convert(pipeline->stencilFail), convert(pipeline->stencilDepthFail), convert(pipeline->stencilBothPass));
		glStencilFunc(stencilFunc, pipeline->stencilReferenceValue, pipeline->stencilReadMask);
	}

	glColorMask(pipeline->colorWriteMaskRed, pipeline->colorWriteMaskGreen, pipeline->colorWriteMaskBlue, pipeline->colorWriteMaskAlpha);

	if (pipeline->conservativeRasterization) {
		glEnable(0x9346); // GL_CONSERVATIVE_RASTERIZATION_NV 
	}
	else {
		glDisable(0x9346);
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

	if (pipeline->depthMode != Graphics4::ZCompareAlways) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	GLenum func = GL_ALWAYS;
	switch (pipeline->depthMode) {
	default:
	case Graphics4::ZCompareAlways:
		func = GL_ALWAYS;
		break;
	case Graphics4::ZCompareNever:
		func = GL_NEVER;
		break;
	case Graphics4::ZCompareEqual:
		func = GL_EQUAL;
		break;
	case Graphics4::ZCompareNotEqual:
		func = GL_NOTEQUAL;
		break;
	case Graphics4::ZCompareLess:
		func = GL_LESS;
		break;
	case Graphics4::ZCompareLessEqual:
		func = GL_LEQUAL;
		break;
	case Graphics4::ZCompareGreater:
		func = GL_GREATER;
		break;
	case Graphics4::ZCompareGreaterEqual:
		func = GL_GEQUAL;
		break;
	}
	glDepthFunc(func);
	glCheckErrors();

	switch (pipeline->cullMode) {
	case Graphics4::Clockwise:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glCheckErrors();
		break;
	case Graphics4::CounterClockwise:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glCheckErrors();
		break;
	case Graphics4::NoCulling:
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

	if (pipeline->blendSource != Graphics4::BlendOne || pipeline->blendDestination != Graphics4::BlendZero || pipeline->alphaBlendSource != Graphics4::BlendOne || pipeline->alphaBlendDestination != Graphics4::BlendZero) {
		glEnable(GL_BLEND);
	}
	else {
		glDisable(GL_BLEND);
	}

	//glBlendFunc(convert(pipeline->blendSource), convert(pipeline->blendDestination));
	glBlendFuncSeparate(convert(pipeline->blendSource), convert(pipeline->blendDestination), convert(pipeline->alphaBlendSource), convert(pipeline->alphaBlendDestination));
}

Graphics4::ConstantLocation Graphics4::PipelineState::getConstantLocation(const char* name) {
	ConstantLocation location;
	location.location = glGetUniformLocation(programId, name);
	location.type = GL_FLOAT;
	GLint count = 0;
	glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &count);
	char arrayName[1024];
	strcpy(arrayName, name);
	strcat(arrayName, "[0]");
	for (GLint i = 0; i < count; ++i) {
		GLenum type;
		char uniformName[1024];
		GLsizei length;
		GLint size;
		glGetActiveUniform(programId, i, 1024 - 1, &length, &size, &type, uniformName);
		if (strcmp(uniformName, name) == 0 || strcmp(uniformName, arrayName) == 0) {
			location.type = type;
			break;
		}
	}
	glCheckErrors();
	if (location.location < 0) {
		log(Warning, "Uniform %s not found.", name);
	}
	return location;
}

int PipelineStateImpl::findTexture(const char* name) {
	for (int index = 0; index < textureCount; ++index) {
		if (strcmp(textures[index], name) == 0) return index;
	}
	return -1;
}

Graphics4::TextureUnit Graphics4::PipelineState::getTextureUnit(const char* name) {
	int index = findTexture(name);
	if (index < 0) {
		int location = glGetUniformLocation(programId, name);
		glCheckErrors();
		index = textureCount;
		textureValues[index] = location;
		strcpy(textures[index], name);
		++textureCount;
	}
	TextureUnit unit;
	unit.unit = index;
	return unit;
}
