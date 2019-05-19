#pragma once

namespace Kore {
	class TextureUnit5Impl {
	public:
		int unit;
	};

	class Texture5Impl {
	public:
		void unmipmap();
		void unset();

		bool mipmap;
		int stage;
	};
}
