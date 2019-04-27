#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int unit;
} TextureUnit5Impl;

typedef struct {
	bool mipmap;
	int stage;
} Texture5Impl;

#ifdef __cplusplus
}
#endif
