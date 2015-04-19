#include "pch.h"
#include <Kore/Graphics/Shader.h>
#include <Kore/Math/Core.h>
#include <Kore/Graphics/Graphics.h>

using namespace Kore;

ShaderImpl::ShaderImpl(void* source, int length) : source((u8*)source), length(length) {

}

Shader::Shader(void* source, int length, ShaderType type) : ShaderImpl(source, length) {
	
}
