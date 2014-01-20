#pragma once

namespace Kore {
	enum LogLevel {
		Info,
		Warning,
		Error
	};

	void log(LogLevel level, const char* format, ...);
}
