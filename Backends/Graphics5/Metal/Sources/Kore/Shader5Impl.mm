#include "pch.h"

#include <Kore/Graphics5/Graphics.h>
#include <Kore/Graphics5/Shader.h>
#include <Kore/Math/Core.h>

#include <Metal/Metal.h>

using namespace Kore;

id getMetalDevice();
id getMetalLibrary();

Shader5Impl::Shader5Impl(void* source, int length) : mtlFunction(0) {
	u8* data = (u8*)source;
	if (length > 1 && data[0] == '>') {
		memcpy(name, data + 1, length - 1);
		name[length - 1] = 0;
	}
	else {
		for (int i = 3; i < length; ++i) {
			if (data[i] == '\n') {
				name[i - 3] = 0;
				break;
			}
			else {
				name[i - 3] = data[i];
			}
		}
	}
}

Shader5Impl::~Shader5Impl() {
	mtlFunction = nil;
}

Graphics5::Shader::Shader(void* source, int length, ShaderType type) : Shader5Impl(source, length) {
	char* data = (char*)source;
	id<MTLLibrary> library = nil;
	if (length > 1 && data[0] == '>') {
		library = getMetalLibrary();
	}
	else {
		id<MTLDevice> device = getMetalDevice();
		library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:data length:length encoding:NSUTF8StringEncoding] options:nil error:nil];
	}
	mtlFunction = [library newFunctionWithName:[NSString stringWithCString:name encoding:NSUTF8StringEncoding]];
  assert(mtlFunction);
}
