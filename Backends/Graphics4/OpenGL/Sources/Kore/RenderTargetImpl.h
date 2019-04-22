#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned _framebuffer;
	unsigned _texture;
	unsigned _depthTexture;
	bool _hasDepth;
	// unsigned _depthRenderbuffer;
	int contextId;
	int format;
} Kinc_G4_RenderTargetImpl;

#ifdef __cplusplus
}
#endif
