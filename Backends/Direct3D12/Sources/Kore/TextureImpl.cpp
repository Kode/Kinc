#include "pch.h"
#include "Direct3D12.h"
#include "TextureImpl.h"
#include <Kore/WinError.h>
#include "Direct3D12.h"
#include "d3dx12.h"

using namespace Kore;

static const int textureCount = 16;

RenderTarget* currentRenderTargets[textureCount] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
Texture* currentTextures[textureCount] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

void TextureImpl::setTextures() {
	if (currentRenderTargets[0] != nullptr) {
		ID3D12DescriptorHeap* heaps[textureCount];
		for (int i = 0; i < textureCount; ++i) {
			heaps[i] = currentRenderTargets[i] == nullptr ? nullptr : currentRenderTargets[i]->srvDescriptorHeap;
		}
		commandList->SetDescriptorHeaps(1, heaps);
		commandList->SetGraphicsRootDescriptorTable(0, currentRenderTargets[0]->srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	}
	if (currentTextures[0] != nullptr) {
		ID3D12DescriptorHeap* heaps[textureCount];
		for (int i = 0; i < textureCount; ++i) {
			heaps[i] = currentTextures[i] == nullptr ? nullptr : currentTextures[i]->srvDescriptorHeap;
		}
		commandList->SetDescriptorHeaps(1, heaps);
		commandList->SetGraphicsRootDescriptorTable(0, currentTextures[0]->srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	}
}

Texture::Texture(const char* filename, bool readable) : Image(filename, readable) {
	stage = 0;
	mipmap = true;
	texWidth = width;
	texHeight = height;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, texWidth, texHeight, 1, 1),
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&image));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(image, 0, 1);
	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadImage));

	D3D12_SUBRESOURCE_DATA srcData;
	srcData.pData = this->data;
	srcData.RowPitch = width * 4;
	srcData.SlicePitch = width * height * 4;

	UpdateSubresources(commandList, image, uploadImage, 0, 0, 1, &srcData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(image, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(image, &shaderResourceViewDesc, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	
	if (!readable) {
		delete[] this->data;
		this->data = nullptr;
	}
}

Texture::Texture(int width, int height, Format format, bool readable) : Image(width, height, format, readable) {
	stage = 0;
	mipmap = true;
	texWidth = width;
	texHeight = height;

	DXGI_FORMAT d3dformat = (format == Image::RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8_UNORM);

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Tex2D(d3dformat, texWidth, texHeight, 1, 1),
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&image));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(image, 0, 1);
	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadImage));

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = d3dformat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(image, &shaderResourceViewDesc, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

TextureImpl::~TextureImpl() {
	unset();
	
}

void TextureImpl::unmipmap() {
	mipmap = false;
	
}

void Texture::_set(TextureUnit unit) {
	if (unit.unit < 0) return;
	//context->PSSetShaderResources(unit.unit, 1, &view);
	this->stage = unit.unit;
	currentTextures[stage] = this;
	currentRenderTargets[stage] = nullptr;
}

void TextureImpl::unset() {
	if (currentTextures[stage] == this) {
		
		currentTextures[stage] = nullptr;
	}
}

u8* Texture::lock() {
	return (u8*)data;
}

void Texture::unlock() {
	D3D12_SUBRESOURCE_DATA srcData;
	srcData.pData = this->data;
	srcData.RowPitch = format == Image::RGBA32 ? (width * 4) : width;
	srcData.SlicePitch = format == Image::RGBA32 ? (width * height * 4) : (width * height);

	UpdateSubresources(commandList, image, uploadImage, 0, 0, 1, &srcData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(image, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

int Texture::stride() {
	return 1;
}
