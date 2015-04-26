#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/Math/Core.h>
#include <Kore/Graphics/Graphics.h>
#include <Metal/Metal.h>
#include <objc/runtime.h>

using namespace Kore;

id getMetalLibrary();

ShaderImpl::ShaderImpl(void* source, int length) {

}

Shader::Shader(void* source, int length, ShaderType type) : ShaderImpl(source, length) {
	id <MTLLibrary> library = getMetalLibrary();
	id <MTLFunction> program;
	if (type == VertexShader)
		program = [library newFunctionWithName:@"kore_vertex"];
	else
		program = [library newFunctionWithName:@"kore_fragment"];
	mtlFunction = program;
}
