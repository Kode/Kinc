#pragma once

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace Kore {
	class TextureUnitImpl {
	public:
		int unit;
	};

	class TextureImpl {
	public:
		~TextureImpl();
		void unmipmap();
		void unset();
	
		bool mipmap;
		int stage;
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* view;
	};
}
