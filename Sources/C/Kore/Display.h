#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int x;
	int y;
	int width;
	int height;
	int pixels_per_inch;
	int frequency;
	int bits_per_pixel;
} Kore_DisplayMode;

/// <summary>
/// Retrieves the index of the primary display
/// </summary>
/// <remarks>
/// The primary is defined by the 
/// </remarks>
/// <returns>The index of the primary display</returns>
KORE_FUNC int Kore_PrimaryDisplay();

/// <summary>
/// Retrieves the number of displays connected to the system
/// </summary>
/// <remarks>
/// All indices from 0 to Kore_CountDisplays() - 1 are legal display indices.
/// </remarks>
/// <returns>The number of displays connected to the system</returns>
KORE_FUNC int Kore_CountDisplays();

/// <summary>
/// Checks whether the display index points to an available display
/// </summary>
/// <param name="display_index">Index of the display to check</param>
/// <returns>
/// Returns true if the index points to an available display,
/// false otherwise
/// </returns>
KORE_FUNC bool Kore_DisplayAvailable(int display_index);

/// <summary>
/// Retrieves the system name of a display
/// </summary>
/// <param name="display_index">Index of the display to retrieve the name from</param>
/// <returns>The system name of the display</returns>
KORE_FUNC const char *Kore_DisplayName(int display_index);

/// <summary>
/// Retrieves the current mode of a display
/// </summary>
/// <param name="display_index">Index of the display to retrieve the mode from</param>
/// <returns>The current display mode</returns>
KORE_FUNC Kore_DisplayMode Kore_DisplayCurrentMode(int display_index);

/// <summary>
/// Retrieves the number of available modes of a display
/// </summary>
/// <param name="display_index">Index of the display to retrieve the modes count from</param>
/// <returns>The number of available modes of the display</returns>
KORE_FUNC int Kore_DisplayCountAvailableModes(int display_index);

/// <summary>
/// Retrieves a specific mode of a display
/// </summary>
/// <param name="display_index">Index of the display to retrieve the mode from</param>
/// <param name="mode_index">Index of the mode to retrieve</param>
/// <returns>The display mode</returns>
KORE_FUNC Kore_DisplayMode Kore_DisplayAvailableMode(int display_index, int mode_index);

#ifdef __cplusplus
}
#endif
