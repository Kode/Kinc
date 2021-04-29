#pragma once

#include <kinc/backend/graphics5/indexbuffer.h>
#include <kinc/backend/graphics5/rendertarget.h>
#include <kinc/backend/graphics5/texture.h>
#include <kinc/backend/graphics5/vertexbuffer.h>

#if defined(KORE_IOS) || defined(__arm64__)
#define KINC_APPLE_SOC
#endif
