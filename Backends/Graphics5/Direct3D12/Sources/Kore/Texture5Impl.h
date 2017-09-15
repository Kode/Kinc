#pragma once

struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;

namespace Kore {
	class TextureUnit5Impl {
	public:
		int unit;
	};

	class Texture5Impl {
	public:
		~Texture5Impl();
		void unmipmap();
		void unset();

		bool mipmap;
		int stage;

		ID3D12Resource* image;
		ID3D12Resource* uploadImage;
		ID3D12DescriptorHeap* srvDescriptorHeap;
		static void setTextures(ID3D12GraphicsCommandList* commandList);
	};
}
