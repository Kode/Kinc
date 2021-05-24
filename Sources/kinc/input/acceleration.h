#pragma once

#include <kinc/global.h>

/*! \file acceleration.h
    \brief Provides data provided by acceleration-sensors.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Is called with measured acceleration-data in three dimensions.
/// </summary>
KINC_FUNC extern void (*kinc_acceleration_callback)(float /*x*/, float /*y*/, float /*z*/);

#ifdef __cplusplus
}
#endif
