#pragma once

#include <kinc/global.h>

#include <kinc/backend/graphics6/swapchain.h>

/*! \file graphics.h
    \brief Provides functions for setting up and using the g6 api.
*/

#ifdef __cplusplus
extern "C" {
#endif

struct kinc_g6_texture;
struct kinc_g6_command_buffer;

typedef struct kinc_g6_swapchain {
	kinc_g6_swapchain_impl_t impl;
} kinc_g6_swapchain_t;
// TODO : gpu selection, qpue queue, etc...

KINC_FUNC void kinc_g6_init();
// KINC_FUNC uint32_t kinc_g6_gpu_count(kinc_g6_instance_t *instance);
// KINC_FUNC void kinc_g6_get_gpu(kinc_g6_instance_t *instance, uint32_t index, kinc_g6_gpu_t *gpu);
KINC_FUNC void kinc_g6_destroy();

// KINC_FUNC void kinc_g6_get_limits(kinc_g6_limits_t *limits);

KINC_FUNC void kinc_g6_swapchain_init(kinc_g6_swapchain_t *swapchain, int window, int width, int height);
KINC_FUNC void kinc_g6_swapchain_resize(kinc_g6_swapchain_t *swapchain, int width, int height);
KINC_FUNC void kinc_g6_swapchain_destroy(kinc_g6_swapchain_t *swapchain);

KINC_FUNC struct kinc_g6_texture *kinc_g6_swapchain_next_texture(kinc_g6_swapchain_t *swapchain);

KINC_FUNC void kinc_g6_submit(kinc_g6_swapchain_t *swapchain, struct kinc_g6_command_buffer **buffers, int count);
KINC_FUNC void kinc_g6_present(kinc_g6_swapchain_t *swapchain);

#ifdef __cplusplus
}
#endif