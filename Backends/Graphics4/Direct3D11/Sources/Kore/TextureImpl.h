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
		~TextureImpl();
		void unmipmap();
		void unset();

		bool mipmap;
		int stage;
		ID3D11Texture2D* texture;
		ID3D11ShaderResourceView* view;
		ID3D11UnorderedAccessView* computeView;
		int rowPitch;
	};
}
