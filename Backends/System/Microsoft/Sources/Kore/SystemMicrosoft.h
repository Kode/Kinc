#pragma once

#include <Kore/Error.h>

#include <stdio.h>

typedef long HRESULT;

namespace Kore {
	namespace Microsoft {
		void affirm(HRESULT result);
		void affirm(HRESULT result, const char* format, ...);
		void format(const char* format, va_list args, wchar_t* buffer);
	}
}
