#pragma once

#include "Error.h"
#include <stdarg.h>

namespace Kore {
	void affirmArgs(bool, const char* format, va_list args);
	void errorArgs(const char* format, va_list args);
}
