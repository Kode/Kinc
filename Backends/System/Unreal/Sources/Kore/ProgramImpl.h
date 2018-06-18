#pragma once

#include "RHIStaticStates.h"

namespace Kore {
	class Shader;

	class ProgramImpl {
	public:
		Shader* vertexShader;
		Shader* fragmentShader;

		FVertexDeclarationRHIRef _vertexDeclaration;
	};
}
