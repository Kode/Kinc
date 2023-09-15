#pragma once

#ifdef KINC_KONG

#include <kinc/global.h>

#include <kinc/backend/graphics4/constantbuffer.h>

#include <kinc/math/matrix.h>
#include <kinc/math/vector.h>

/*! \file constantbuffer.h
    \brief Provides support for managing buffers of constant-data for shaders.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_g4_constant_buffer {
	kinc_g4_constant_buffer_impl impl;
} kinc_g4_constant_buffer;

/// <summary>
/// Initializes a constant-buffer.
/// </summary>
/// <param name="buffer">The buffer to initialize</param>
/// <param name="size">The size of the constant-data in the buffer in bytes</param>
KINC_FUNC void kinc_g4_constant_buffer_init(kinc_g4_constant_buffer *buffer, size_t size);

/// <summary>
/// Destroys a buffer.
/// </summary>
/// <param name="buffer">The buffer to destroy</param>
KINC_FUNC void kinc_g4_constant_buffer_destroy(kinc_g4_constant_buffer *buffer);

/// <summary>
/// Locks all of a constant-buffer to modify its contents.
/// </summary>
/// <param name="buffer">The buffer to lock</param>
/// <returns>The contents of the buffer</returns>
KINC_FUNC uint8_t *kinc_g4_constant_buffer_lock_all(kinc_g4_constant_buffer *buffer);

/// <summary>
/// Locks part of a constant-buffer to modify its contents.
/// </summary>
/// <param name="buffer">The buffer to lock</param>
/// <param name="start">The offset of where to start the lock in bytes</param>
/// <param name="count">The number of bytes to lock</param>
/// <returns>The contents of the buffer, starting at start</returns>
KINC_FUNC uint8_t *kinc_g4_constant_buffer_lock(kinc_g4_constant_buffer *buffer, size_t start, size_t count);

/// <summary>
/// Unlocks a constant-buffer so the changed contents can be used.
/// </summary>
/// <param name="buffer">The buffer to unlock</param>
KINC_FUNC void kinc_g4_constant_buffer_unlock_all(kinc_g4_constant_buffer *buffer);

/// <summary>
/// Unlocks parts of a constant-buffer so the changed contents can be used.
/// </summary>
/// <param name="buffer">The buffer to unlock</param>
/// /// <param name="count">The number of bytes to unlock, starting from the start-index from the previous lock-call</param>
KINC_FUNC void kinc_g4_constant_buffer_unlock(kinc_g4_constant_buffer *buffer, size_t count);

/// <summary>
/// Figures out the size of the constant-data in the buffer.
/// </summary>
/// <param name="buffer">The buffer to figure out the size for</param>
/// <returns>Returns the size of the constant-data in the buffer in bytes</returns>
KINC_FUNC size_t kinc_g4_constant_buffer_size(kinc_g4_constant_buffer *buffer);

#ifdef __cplusplus
}
#endif

#endif
