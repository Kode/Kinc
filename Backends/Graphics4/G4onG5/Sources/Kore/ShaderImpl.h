#pragma once

#include <Kinc/Graphics5/Shader.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	//ShaderImpl(void *data, int length, Graphics5::ShaderType type);
	kinc_g5_shader_t _shader;
} ShaderImpl;

#ifdef __cplusplus
}
#endif
