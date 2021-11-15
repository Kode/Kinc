#pragma once

#include <kinc/global.h>

/*! \file acceleration.h
    \brief Provides data provided by acceleration-sensors.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Sets the acceleration-callback which is called with measured acceleration-data in three dimensions.
/// </summary>
/// <param name="value">The callback</param>
KINC_FUNC void kinc_acceleration_set_callback(void (*value)(float /*x*/, float /*y*/, float /*z*/));

void kinc_internal_on_acceleration(float x, float y, float z);


#ifdef __cplusplus
}
#endif
