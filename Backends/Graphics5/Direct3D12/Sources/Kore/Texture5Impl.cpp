#include "pch.h"

#include "Direct3D12.h"
#include "Texture5Impl.h"

#include <Kore/SystemMicrosoft.h>

using namespace Kore;

static const int textureCount = 16;

Graphics5::RenderTarget* currentRenderTargets[textureCount] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                               nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
Graphics5::Texture* currentTextures[textureCount] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                                     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

bool bilinearFiltering = false;

namespace {
	ID3D12DescriptorHeap* samplerDescriptorHeapPoint;
	ID3D12DescriptorHeap* samplerDescriptorHeapBilinear;
}

void Texture5Impl::setTextures(ID3D12GraphicsCommandList* commandList) {
	if (currentRenderTargets[0] != nullptr) {
		ID3D12DescriptorHeap* heaps[textureCount];
		for (int i = 0; i < textureCount; ++i) {
			heaps[i] = currentRenderTargets[i] == nullptr ? nullptr : currentRenderTargets[i]->srvDescriptorHeap;
		}
		heaps[1] = bilinearFiltering ? samplerDescriptorHeapBilinear : samplerDescriptorHeapPoint;
		commandList->SetDescriptorHeaps(2, heaps);
		commandList->SetGraphicsRootDescriptorTable(0, currentRenderTargets[0]->srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		commandList->SetGraphicsRootDescriptorTable(1, heaps[1]->GetGPUDescriptorHandleForHeapStart());
	}
	if (currentTextures[0] != nullptr) {
		ID3D12DescriptorHeap* heaps[textureCount];
		for (int i = 0; i < textureCount; ++i) {
			heaps[i] = currentTextures[i] == nullptr ? nullptr : currentTextures[i]->srvDescriptorHeap;
		}
		heaps[1] = bilinearFiltering ? samplerDescriptorHeapBilinear : samplerDescriptorHeapPoint;
		commandList->SetDescriptorHeaps(2, heaps);
		commandList->SetGraphicsRootDescriptorTable(0, currentTextures[0]->srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		commandList->SetGraphicsRootDescriptorTable(1, heaps[1]->GetGPUDescriptorHandleForHeapStart());
	}
}

static void createSampler(bool bilinear, D3D12_FILTER filter) {
	D3D12_DESCRIPTOR_HEAP_DESC descHeapSampler = {};
	descHeapSampler.NumDescriptors = 2;
	descHeapSampler.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	descHeapSampler.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&descHeapSampler, IID_GRAPHICS_PPV_ARGS(&(bilinear ? samplerDescriptorHeapBilinear : samplerDescriptorHeapPoint)));

	D3D12_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D12_SAMPLER_DESC));
	samplerDesc.Filter = filter;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	device->CreateSampler(&samplerDesc, (bilinear ? samplerDescriptorHeapBilinear : samplerDescriptorHeapPoint)->GetCPUDescriptorHandleForHeapStart());
}

void createSamplers() {
	createSampler(false, D3D12_FILTER_MIN_MAG_MIP_POINT);
	createSampler(true, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);
}

void Graphics5::Texture::_init(const char* format, bool readable) {
	stage = 0;
	mipmap = true;
	texWidth = width;
	texHeight = height;

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	                                &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, 1, 1), D3D12_RESOURCE_STATE_COPY_DEST,
	                                nullptr, IID_GRAPHICS_PPV_ARGS(&image));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(image, 0, 1);
	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&uploadImage));

	BYTE* pixel;
	uploadImage->Map(0, nullptr, reinterpret_cast<void**>(&pixel));
	int pitch = stride();
	for (int y = 0; y < height; ++y) {
		memcpy(&pixel[y * pitch], &this->data[y * width * 4], width * 4);
	}
	uploadImage->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(image, &shaderResourceViewDesc, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	if (!readable) {
		delete[] this->data;
		this->data = nullptr;
	}
}

Graphics5::Texture::Texture(int width, int height, Format format, bool readable) : Image(width, height, format, readable) {
	stage = 0;
	mipmap = true;
	texWidth = width;
	texHeight = height;

	DXGI_FORMAT d3dformat = (format == Image::RGBA32 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R8_UNORM);

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	                                &CD3DX12_RESOURCE_DESC::Tex2D(d3dformat, texWidth, texHeight, 1, 1), D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
	                                IID_GRAPHICS_PPV_ARGS(&image));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(image, 0, 1);
	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&uploadImage));

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&srvDescriptorHeap));

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.Format = d3dformat;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	device->CreateShaderResourceView(image, &shaderResourceViewDesc, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

Graphics5::Texture::Texture(int width, int height, int depth, Image::Format format, bool readable) : Image(width, height, depth, format, readable) {}

Texture5Impl::~Texture5Impl() {
	unset();
	image->Release();
	uploadImage->Release();
	srvDescriptorHeap->Release();
}

void Texture5Impl::unmipmap() {
	mipmap = false;
}

void Graphics5::Texture::_set(TextureUnit unit) {
	if (unit.unit < 0) return;
	// context->PSSetShaderResources(unit.unit, 1, &view);
	this->stage = unit.unit;
	currentTextures[stage] = this;
	currentRenderTargets[stage] = nullptr;
}

void Texture5Impl::unset() {
	if (currentTextures[stage] == this) {

		currentTextures[stage] = nullptr;
	}
}

u8* Graphics5::Texture::lock() {
	return (u8*)this->data;
}

void Graphics5::Texture::unlock() {
	BYTE* pixel;
	uploadImage->Map(0, nullptr, reinterpret_cast<void**>(&pixel));
	int pitch = stride();
	for (int y = 0; y < height; ++y) {
		memcpy(&pixel[y * pitch], &this->data[y * width], format == Image::RGBA32 ? width * 4 : width);
	}
	uploadImage->Unmap(0, nullptr);
}

void Graphics5::Texture::clear(int x, int y, int z, int width, int height, int depth, uint color) {}

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
int d3d12_textureAlignment() {
	return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}
#else
int d3d12_textureAlignment();
#endif

int Graphics5::Texture::stride() {
	int baseStride = format == Image::RGBA32 ? (width * 4) : width;
	if (format == Image::Grey8) return width; // please investigate further
	for (int i = 0;; ++i) {
		if (d3d12_textureAlignment() * i >= baseStride) {
			return d3d12_textureAlignment() * i;
		}
	}
}

void Graphics5::Texture::generateMipmaps(int levels) {}

void Graphics5::Texture::setMipmap(Texture* mipmap, int level) {}
