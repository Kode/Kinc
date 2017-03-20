#include "pch.h"

#include <Kore/Graphics/Shader.h>
#include <Kore/Log.h>
#include <Kore/System.h>

#include "ProgramImpl.h"

#include "ShaderParameterUtils.h"
#include "RHIStaticStates.h"
#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "ShaderParameters.h"
#include "CoreUObject.h"
#include "Engine.h"

#include "PixelShaderDeclaration.h"

using namespace Kore;

Program::Program() {
	vertexShader = nullptr;
	fragmentShader = nullptr;
}

void Program::setVertexShader(Shader* shader) {
	vertexShader = shader;
}

void Program::setFragmentShader(Shader* shader) {
	fragmentShader = shader;
}

void Program::setGeometryShader(Shader* shader) {

}

void Program::setTessellationControlShader(Shader* shader) {

}

void Program::setTessellationEvaluationShader(Shader* shader) {

}

void Program::link(VertexStructure** structures, int count) {
	uint stride = 0;
	for (int i = 0; i < structures[0]->size; ++i) {
		switch (structures[0]->elements[i].data) {
		case Kore::Float1VertexData:
			stride += 4;
			break;
		case Kore::Float2VertexData:
			stride += 4 * 2;
			break;
		case Kore::Float3VertexData:
			stride += 4 * 3;
			break;
		case Kore::Float4VertexData:
			stride += 4 * 4;
			break;
		}
	}

	FVertexDeclarationElementList elements;
	int offset = 0;
	for (int i = 0; i < structures[0]->size; ++i) {
		switch (structures[0]->elements[i].data) {
			case Kore::Float1VertexData:
				elements.Add(FVertexElement(0, offset, VET_Float1, i, stride));
				offset += 4;
				break;
			case Kore::Float2VertexData:
				elements.Add(FVertexElement(0, offset, VET_Float2, i, stride));
				offset += 4 * 2;
				break;
			case Kore::Float3VertexData:
				elements.Add(FVertexElement(0, offset, VET_Float3, i, stride));
				offset += 4 * 3;
				break;
			case Kore::Float4VertexData:
				elements.Add(FVertexElement(0, offset, VET_Float4, i, stride));
				offset += 4 * 4;
				break;
		}
	}
	_vertexDeclaration = RHICreateVertexDeclaration(elements);
}

void Program::set() {
	static FGlobalBoundShaderState shaderState;
	TShaderMapRef<FVertexShaderExample> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	TShaderMapRef<FPixelShaderDeclaration> PixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	FRHICommandListImmediate& commandList = GRHICommandList.GetImmediateCommandList();
	SetGlobalBoundShaderState(commandList, ERHIFeatureLevel::SM5, shaderState, _vertexDeclaration, *VertexShader, *PixelShader);
}

ConstantLocation Program::getConstantLocation(const char* name) {
	ConstantLocation location = ConstantLocation();
	TShaderMapRef<FVertexShaderExample> VertexShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	location.parameter.Bind(VertexShader->parameters, ANSI_TO_TCHAR(name));
	return location;
}

TextureUnit Program::getTextureUnit(const char* name) {
	TextureUnit unit = TextureUnit();
	TShaderMapRef<FPixelShaderDeclaration> PixelShader(GetGlobalShaderMap(ERHIFeatureLevel::SM5));
	unit.parameter.Bind(PixelShader->parameters, ANSI_TO_TCHAR(name));
	char samplerName[1024];
	strcpy(samplerName, name);
	strcat(samplerName, "Sampler");
	unit.sampler.Bind(PixelShader->parameters, ANSI_TO_TCHAR(samplerName));
	return unit;
}
