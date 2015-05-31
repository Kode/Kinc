#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/Math/Core.h>
#include <Kore/Graphics/Graphics.h>
#include <Metal/Metal.h>
#include <objc/runtime.h>

using namespace Kore;

id getMetalLibrary();

ShaderImpl::ShaderImpl(void* source, int length) {
	u8* data = (u8*)source;
	for (int i = 0; i < length; ++i) {
		name[i] = data[i];
	}
	name[length] = 0;
}

Shader::Shader(void* source, int length, ShaderType type) : ShaderImpl(source, length) {
	id <MTLLibrary> library = getMetalLibrary();
	id <MTLFunction> program;
	program = [library newFunctionWithName:[NSString stringWithCString:name encoding:NSUTF8StringEncoding]];
	mtlFunction = program;
}
