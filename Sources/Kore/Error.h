#pragma once

namespace Kore {
	void affirm(bool);
	void affirm(bool, const char* format, ...);
	void error();
	void error(const char* format, ...);
}
