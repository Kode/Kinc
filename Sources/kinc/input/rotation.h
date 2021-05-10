#pragma once

#include <kinc/global.h>

#ifdef __cplusplus
extern "C" {
#endif

KINC_FUNC extern void (*kinc_rotation_callback)(float /*x*/, float /*y*/, float /*z*/);

#ifdef __cplusplus
}
#endif
