#pragma once

#include <kinc/global.h>

#include <kinc/image.h>

#include "rendertarget.h"
#include "shader.h"
#include "vertexstructure.h"

#include <kinc/backend/graphics5/graphics.h>

#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>

/*! \file graphics.h
    \brief Contains the base G5-functionality.
*/

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC extern bool kinc_g5_fullscreen;

typedef struct kinc_g5_features {
	bool instancedRendering;
	bool computeShaders;
	bool blendConstants;
	bool nonPow2Textures;
	bool invertedY;
    bool raytracing;
} kinc_g5_features_t;

typedef struct kinc_g5_limits {
	size_t maxBoundTextures;
} kinc_g5_limits_t;

/// <summary>
///	Fills the passed object with information about supported features.
/// </summary>
/// <param name="features">Object to be filled with info about features.</param>
KINC_FUNC void kinc_g5_get_features(kinc_g5_features_t *features);

/// <summary>
///	Fills the passed object with information about supported limits.
/// </summary>
/// <param name="limits">Object to be filled with info about limits.</param>
KINC_FUNC void kinc_g5_get_limits(kinc_g5_limits_t *limits);

/// <summary>
/// I think this does nothing.
/// </summary>
KINC_FUNC void kinc_g5_flush(void);

/// <summary>
/// Returns the currently used number of samples for hardware-antialiasing.
/// </summary>
/// <returns>The number of samples</returns>
KINC_FUNC int kinc_g5_antialiasing_samples(void);

/// <summary>
/// Sets the number of samples used for hardware-antialiasing. This typically uses multisampling and typically only works with a few specific numbers of
/// sample-counts - 2 and 4 are pretty save bets. It also might do nothing at all.
/// </summary>
/// <param name="samples">The number of samples</param>
KINC_FUNC void kinc_g5_set_antialiasing_samples(int samples);

/// <summary>
/// Returns whether textures are supported which have widths/heights which are not powers of two - which is always the case.
/// </summary>
/// <returns>True, always</returns>
KINC_FUNC bool kinc_g5_non_pow2_textures_qupported(void);

KINC_FUNC bool kinc_g5_render_targets_inverted_y(void);

/// <summary>
/// Needs to be called before rendering to a window. Typically called at the start of each frame.
/// </summary>
/// <param name="window">The window to render to</param>
KINC_FUNC void kinc_g5_begin(kinc_g5_render_target_t *renderTarget, int window);

/// <summary>
/// Needs to be called after rendering to a window. Typically called at the end of each frame.
/// </summary>
/// <param name="window">The window to render to</param>
/// <returns></returns>
KINC_FUNC void kinc_g5_end(int window);

/// <summary>
/// Needs to be called to make the rendered frame visible. Typically called at the very end of each frame.
/// </summary>
KINC_FUNC bool kinc_g5_swap_buffers(void);

/// <summary>
/// Returns how many textures can be used at the same time in a fragment-shader.
/// </summary>
/// <returns>The number of textures</returns>
KINC_FUNC int kinc_g5_max_bound_textures(void);

#ifndef KINC_DOCS
void kinc_g5_internal_init(void);
void kinc_g5_internal_init_window(int window, int depth_buffer_bits, int stencil_buffer_bits, bool vsync);
void kinc_g5_internal_destroy_window(int window);
void kinc_g5_internal_destroy(void);
#endif

#ifdef __cplusplus
}
#endif
