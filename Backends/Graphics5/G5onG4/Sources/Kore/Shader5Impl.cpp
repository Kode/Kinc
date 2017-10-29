#include "pch.h"

#include <Kore/Graphics5/Shader.h>
#include <Kore/Math/Core.h>

#include <Kore/Graphics4/Shader.h>

using namespace Kore;

Shader5Impl::Shader5Impl() {}

Graphics5::Shader::Shader(void* _data, int length, ShaderType type) {
	shader = new Graphics4::Shader(_data, length, type == VertexShader ? Graphics4::VertexShader : Graphics4::FragmentShader);
}
