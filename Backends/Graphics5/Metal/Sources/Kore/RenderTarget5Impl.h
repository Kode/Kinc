#pragma once

#include <objc/runtime.h>

namespace Kore {
	class RenderTarget5Impl {
	public:
		id _tex;
		id _sampler;
		id _depthTex;
	};
}
