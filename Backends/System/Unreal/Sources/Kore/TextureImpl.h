#pragma once

#include "ShaderParameters.h"

#include "Engine/Texture2D.h"

namespace Kore {
	class TextureUnitImpl {
	public:
		FShaderResourceParameter parameter;
		FShaderResourceParameter sampler;
	};

	class TextureImpl {
	public:
		virtual ~TextureImpl();
		void unmipmap();
		void unset();
		int stage;
		bool mipmap;
		u8 pixfmt;
		int pitch;

		FTexture2DRHIRef _tex;
	};
}
