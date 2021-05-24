#pragma once

#include <kinc/global.h>

/*! \file rotation.h
    \brief Provides support for rotation-sensors.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Called when the device is rotated. Act quickly when this is called for a desktop-system.
/// </summary>
KINC_FUNC extern void (*kinc_rotation_callback)(float /*x*/, float /*y*/, float /*z*/);

#ifdef __cplusplus
}
#endif
