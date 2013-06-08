#pragma once

namespace Kore {
	void affirm(bool);
	void affirm(bool, const char* message);
	void error();
	void error(const char* message);
}
