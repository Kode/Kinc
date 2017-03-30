#pragma once

#include <Kore/Error.h>

typedef long HRESULT;

namespace Kore {
	void affirm(HRESULT result);
	void affirm(HRESULT result, const char* format, ...);
}
