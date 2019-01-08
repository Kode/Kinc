#pragma once

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

namespace Kore {
	class TextureUnitImpl {
	public:
		int unit;
		bool vertex;
	};

	class TextureImpl {
	public:
		TextureImpl();
		~TextureImpl();
		void enableMipmaps(int texWidth, int texHeight, int format);
		void unmipmap();
		void unset();

		bool hasMipmaps;
		int stage;
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* view;
		ID3D11UnorderedAccessView* computeView;
		int rowPitch;
	};
}
