#include "pch.h"

#include "Direct3D12.h"
#include "Texture5Impl.h"

#include <kinc/graphics5/rendertarget.h>
#include <kinc/graphics5/texture.h>
#include <kinc/math/core.h>

#include <Kore/SystemMicrosoft.h>

static const int textureCount = 16;

kinc_g5_render_target_t *currentRenderTargets[textureCount] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                               nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
kinc_g5_texture *currentTextures[textureCount] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

void kinc_g5_internal_set_textures(ID3D12GraphicsCommandList* commandList) {
	if (currentRenderTargets[0] != nullptr) {
		ID3D12DescriptorHeap* heaps[textureCount];
		for (int i = 0; i < textureCount; ++i) {
			heaps[i] = currentRenderTargets[i] == nullptr ? nullptr : currentRenderTargets[i]->impl.srvDescriptorHeap;
		}
		commandList->SetDescriptorHeaps(1, heaps);
		commandList->SetGraphicsRootDescriptorTable(0, currentRenderTargets[0]->impl.srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	}
	if (currentTextures[0] != nullptr) {
		ID3D12DescriptorHeap* heaps[textureCount];
		for (int i = 0; i < textureCount; ++i) {
			heaps[i] = currentTextures[i] == nullptr ? nullptr : currentTextures[i]->impl.srvDescriptorHeap;
		}
		commandList->SetDescriptorHeaps(1, heaps);
		commandList->SetGraphicsRootDescriptorTable(0, currentTextures[0]->impl.srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	}
}

void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image_t *image) {
	texture->impl.stage = 0;
	texture->impl.mipmap = true;
	texture->texWidth = image->width;
	texture->texHeight = image->height;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	                                &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, texture->texWidth, texture->texHeight, 1, 1),
	                                D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_GRAPHICS_PPV_ARGS(&texture->impl.image));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture->impl.image, 0, 1);
	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&texture->impl.uploadImage));

	texture->impl.stride = (int)kinc_ceil(uploadBufferSize / (float)image->height);

	BYTE *pixel;
	texture->impl.uploadImage->Map(0, nullptr, reinterpret_cast<void **>(&pixel));
	int pitch = kinc_g5_texture_stride(texture);
	for (int y = 0; y < texture->texHeight; ++y) {
		memcpy(&pixel[y * pitch], &((uint8_t*)image->data)[y * texture->texWidth * 4], texture->texWidth * 4);
	}
	texture->impl.uploadImage->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&texture->impl.srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(texture->impl.image, &shaderResourceViewDesc, texture->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void kinc_g5_texture_init(kinc_g5_texture *texture, int width, int height, kinc_image_format_t format) {
	//kinc_image_init(&texture->image, width, height, format, readable);
	texture->impl.stage = 0;
	texture->impl.mipmap = true;
	texture->texWidth = width;
	texture->texHeight = height;

	DXGI_FORMAT d3dformat = (format == KINC_IMAGE_FORMAT_RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8_UNORM);

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	                                &CD3DX12_RESOURCE_DESC::Tex2D(d3dformat, texture->texWidth, texture->texHeight, 1, 1), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_GRAPHICS_PPV_ARGS(&texture->impl.image));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture->impl.image, 0, 1);
	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&texture->impl.uploadImage));

	texture->impl.stride = (int)kinc_ceil(uploadBufferSize / (float)height);

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&texture->impl.srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = d3dformat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(texture->impl.image, &shaderResourceViewDesc, texture->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void kinc_g5_texture_init3d(kinc_g5_texture *texture, int width, int height, int depth, kinc_image_format_t format, bool readable) {
	//kinc_image_init3d(&texture->image, width, height, depth, format, readable);
}

void kinc_g5_internal_texture_unset(kinc_g5_texture *texture);

void kinc_g5_texture_destroy(kinc_g5_texture *texture) {
	kinc_g5_internal_texture_unset(texture);
}

void kinc_g5_internal_texture_unmipmap(kinc_g5_texture *texture) {
	texture->impl.mipmap = false;
}

void kinc_g5_internal_texture_set(kinc_g5_texture *texture, int unit) {
	if (unit < 0) return;
	// context->PSSetShaderResources(unit.unit, 1, &view);
	texture->impl.stage = unit;
	currentTextures[texture->impl.stage] = texture;
	currentRenderTargets[texture->impl.stage] = nullptr;
}

void kinc_g5_internal_texture_unset(kinc_g5_texture *texture) {
	if (currentTextures[texture->impl.stage] == texture) {

		currentTextures[texture->impl.stage] = nullptr;
	}
}

uint8_t *kinc_g5_texture_lock(kinc_g5_texture *texture) {
	return (uint8_t *)NULL;//texture->image.data;
}

void kinc_g5_texture_unlock(kinc_g5_texture *texture) {
	BYTE* pixel;
	texture->impl.uploadImage->Map(0, nullptr, reinterpret_cast<void **>(&pixel));
	int pitch = kinc_g5_texture_stride(texture);
	for (int y = 0; y < texture->texHeight; ++y) {
		//memcpy(&pixel[y * pitch], &texture->image.data[y * texture->texWidth],
		//       texture->format == KINC_IMAGE_FORMAT_RGBA32 ? texture->texWidth * 4 : texture->texWidth);
	}
	texture->impl.uploadImage->Unmap(0, nullptr);
}

void kinc_g5_texture_clear(kinc_g5_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color) {}

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
int d3d12_textureAlignment() {
	return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}
#else
int d3d12_textureAlignment();
#endif

int kinc_g5_texture_stride(kinc_g5_texture *texture) {
	/*int baseStride = texture->format == KINC_IMAGE_FORMAT_RGBA32 ? (texture->texWidth * 4) : texture->texWidth;
	if (texture->format == KINC_IMAGE_FORMAT_GREY8) return texture->texWidth; // please investigate further
	for (int i = 0;; ++i) {
		if (d3d12_textureAlignment() * i >= baseStride) {
			return d3d12_textureAlignment() * i;
		}
	}*/
	return texture->impl.stride;
}

void kinc_g5_texture_generate_mipmaps(kinc_g5_texture *texture, int levels) {}

void kinc_g5_texture_set_mipmap(kinc_g5_texture *texture, kinc_g5_texture *mipmap, int level) {}
