#include "pch.h"

#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/shader.h>
#include <kinc/math/core.h>

#include <Metal/Metal.h>

id getMetalDevice();
id getMetalLibrary();

void kinc_g5_shader_destroy(kinc_g5_shader_t *shader) {
	shader->impl.mtlFunction = nil;
}

void kinc_g5_shader_init(kinc_g5_shader_t *shader, void *source, size_t length, kinc_g5_shader_type_t type) {
	memset(&shader->impl, 0, sizeof(shader->impl));

	{
		uint8_t *data = (uint8_t*)source;
		if (length > 1 && data[0] == '>') {
			memcpy(shader->impl.name, data + 1, length - 1);
			shader->impl.name[length - 1] = 0;
		}
		else {
			for (int i = 3; i < length; ++i) {
				if (data[i] == '\n') {
					shader->impl.name[i - 3] = 0;
					break;
				}
				else {
					shader->impl.name[i - 3] = data[i];
				}
			}
		}
	}

	char* data = (char*)source;
	id<MTLLibrary> library = nil;
	if (length > 1 && data[0] == '>') {
		library = getMetalLibrary();
	}
	else {
		id<MTLDevice> device = getMetalDevice();
		library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:data length:length encoding:NSUTF8StringEncoding] options:nil error:nil];
	}
	shader->impl.mtlFunction = [library newFunctionWithName:[NSString stringWithCString:shader->impl.name encoding:NSUTF8StringEncoding]];
	assert(shader->impl.mtlFunction);
}
