#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	bool available;
	int x;
	int y;
	int width;
	int height;
	bool primary;
	int number;
} kinc_display_t;

typedef struct {
	int x;
	int y;
	int width;
	int height;
	int pixels_per_inch;
	int frequency;
	int bits_per_pixel;
} kinc_display_mode_t;

/// <summary>
/// Retrieves the index of the primary display
/// </summary>
/// <remarks>
/// The primary is defined by the 
/// </remarks>
/// <returns>The index of the primary display</returns>
KINC_FUNC int kinc_primary_display(void);

/// <summary>
/// Retrieves the number of displays connected to the system
/// </summary>
/// <remarks>
/// All indices from 0 to Kinc_CountDisplays() - 1 are legal display indices.
/// </remarks>
/// <returns>The number of displays connected to the system</returns>
KINC_FUNC int kinc_count_displays(void);

/// <summary>
/// Checks whether the display index points to an available display
/// </summary>
/// <param name="display_index">Index of the display to check</param>
/// <returns>
/// Returns true if the index points to an available display,
/// false otherwise
/// </returns>
KINC_FUNC bool kinc_display_available(int display_index);

/// <summary>
/// Retrieves the system name of a display
/// </summary>
/// <param name="display_index">Index of the display to retrieve the name from</param>
/// <returns>The system name of the display</returns>
KINC_FUNC const char *kinc_display_name(int display_index);

/// <summary>
/// Retrieves the current mode of a display
/// </summary>
/// <param name="display_index">Index of the display to retrieve the mode from</param>
/// <returns>The current display mode</returns>
KINC_FUNC kinc_display_mode_t kinc_display_current_mode(int display_index);

/// <summary>
/// Retrieves the number of available modes of a display
/// </summary>
/// <param name="display_index">Index of the display to retrieve the modes count from</param>
/// <returns>The number of available modes of the display</returns>
KINC_FUNC int kinc_display_count_available_modes(int display_index);

/// <summary>
/// Retrieves a specific mode of a display
/// </summary>
/// <param name="display_index">Index of the display to retrieve the mode from</param>
/// <param name="mode_index">Index of the mode to retrieve</param>
/// <returns>The display mode</returns>
KINC_FUNC kinc_display_mode_t kinc_display_available_mode(int display_index, int mode_index);

#ifdef __cplusplus
}
#endif
