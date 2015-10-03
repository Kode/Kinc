#pragma once

struct ID3D12Resource;
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

		ID3D12Resource* image;
		ID3D12Resource* uploadImage;
		ID3D12DescriptorHeap* srvDescriptorHeap;
		static void setTextures();
	};
}
