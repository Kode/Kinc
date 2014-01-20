#pragma once

#include "Log.h"
#include <stdarg.h>

namespace Kore {
	void logArgs(LogLevel level, const char* format, va_list args);
}
