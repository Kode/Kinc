#pragma once

#include <Kore/ShaderImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	KINC_SHADER_TYPE_FRAGMENT,
	KINC_SHADER_TYPE_VERTEX,
	KINC_SHADER_TYPE_GEOMETRY,
	KINC_SHADER_TYPE_TESSELLATION_CONTROL,
	KINC_SHADER_TYPE_TESSELLATION_EVALUATION
} Kinc_G4_ShaderType;

typedef struct _Kinc_G4_Shader {
	Kinc_G4_ShaderImpl impl;
} Kinc_G4_Shader;

void Kinc_G4_Shader_Create(Kinc_G4_Shader *shader, void *data, int length, Kinc_G4_ShaderType type);
void Kinc_G4_Shader_CreateFromSource(Kinc_G4_Shader *shader, const char *source, Kinc_G4_ShaderType type); // Beware, this is not portable
void Kinc_G4_Shader_Destroy(Kinc_G4_Shader *shader);

#ifdef __cplusplus
}
#endif
