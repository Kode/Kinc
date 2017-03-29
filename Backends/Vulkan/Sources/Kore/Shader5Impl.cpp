#include "pch.h"

#include <Kore/Graphics5/Graphics.h>
#include <Kore/Graphics5/Shader.h>
#include <Kore/Math/Core.h>

using namespace Kore;

Shader5Impl::Shader5Impl(void* source, int length) : length(length), id(0) {
	this->source = new char[length + 1];
	for (int i = 0; i < length; ++i) {
		this->source[i] = ((char*)source)[i];
	}
	this->source[length] = 0;
}

Shader5Impl::~Shader5Impl() {
	delete[] source;
	source = nullptr;
}

Graphics5::Shader::Shader(void* source, int length, ShaderType type) : Shader5Impl(source, length) {}
