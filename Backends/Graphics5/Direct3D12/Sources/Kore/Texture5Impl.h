#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct ID3D12Resource;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;

typedef struct {
	int unit;
} TextureUnit5Impl;

typedef struct {
	//void unmipmap();
	//void unset();

	bool mipmap;
	int stage;

	struct ID3D12Resource *image;
	struct ID3D12Resource *uploadImage;
	struct ID3D12DescriptorHeap *srvDescriptorHeap;
	//static void setTextures(ID3D12GraphicsCommandList *commandList);
} Texture5Impl;

#ifdef __cplusplus
}
#endif
