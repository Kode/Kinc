#pragma once

#include <objc/runtime.h>

namespace Kore {
	class RenderTarget5Impl {
	public:
		RenderTarget5Impl();
		~RenderTarget5Impl();
		id _tex;
		id _sampler;
		id _depthTex;
	};
}
