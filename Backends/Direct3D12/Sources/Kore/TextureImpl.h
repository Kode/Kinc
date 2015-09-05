#pragma once

struct ID3D12DescriptorHeap;

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
		//ID3D11Texture2D* texture;
		//ID3D11ShaderResourceView* view;
		ID3D12DescriptorHeap* srvDescriptorHeap;
		static void setTextures();
	};
}
