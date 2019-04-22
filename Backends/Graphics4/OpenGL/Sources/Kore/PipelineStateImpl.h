#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned programId;
	char **textures;
	int *textureValues;
	int textureCount;
} Kinc_G4_PipelineStateImpl;

#ifdef __cplusplus
}
#endif
