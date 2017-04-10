#include "pch.h"

#include <Kore/Graphics4/Shader.h>
#include <Kore/Math/Core.h>

using namespace Kore;

ShaderImpl::ShaderImpl(void* _data, int length, Graphics5::ShaderType type) : _shader(_data, length, type) {}

Graphics4::Shader::Shader(void* _data, int length, ShaderType type) : ShaderImpl(_data, length, (Graphics5::ShaderType)type) {}

Graphics4::Shader::Shader(const char* source, Graphics4::ShaderType type) : ShaderImpl(nullptr, 0, (Graphics5::ShaderType)type) {}
