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

bool bilinearFiltering = false;

namespace {
	ID3D12DescriptorHeap* samplerDescriptorHeapPoint;
	ID3D12DescriptorHeap* samplerDescriptorHeapBilinear;
	static const int heapSize = 1024;
	static int heapIndex = 0;
	ID3D12DescriptorHeap* srvHeap;
	ID3D12DescriptorHeap* samplerHeap;
}

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
static int d3d12_textureAlignment() {
	return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}
#else
int d3d12_textureAlignment();
#endif

static DXGI_FORMAT convertFormat(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case KINC_IMAGE_FORMAT_RGBA64:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case KINC_IMAGE_FORMAT_RGB24:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case KINC_IMAGE_FORMAT_A32:
		return DXGI_FORMAT_R32_FLOAT;
	case KINC_IMAGE_FORMAT_A16:
		return DXGI_FORMAT_R16_FLOAT;
	case KINC_IMAGE_FORMAT_GREY8:
		return DXGI_FORMAT_R8_UNORM;
	case KINC_IMAGE_FORMAT_BGRA32:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case KINC_IMAGE_FORMAT_RGBA32:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	default:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

static int formatByteSize(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return 16;
	case KINC_IMAGE_FORMAT_RGBA64:
		return 8;
	case KINC_IMAGE_FORMAT_RGB24:
		return 4;
	case KINC_IMAGE_FORMAT_A32:
		return 4;
	case KINC_IMAGE_FORMAT_A16:
		return 2;
	case KINC_IMAGE_FORMAT_GREY8:
		return 1;
	case KINC_IMAGE_FORMAT_BGRA32:
	case KINC_IMAGE_FORMAT_RGBA32:
		return 4;
	default:
		return 4;
	}
}

void kinc_g5_internal_set_textures(ID3D12GraphicsCommandList* commandList) {

	if (currentRenderTargets[0] != nullptr || currentTextures[0] != nullptr) {
		int srvStep = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		int samplerStep = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		if (heapIndex + textureCount >= heapSize) {
			heapIndex = 0;
		}

		ID3D12DescriptorHeap* samplerDescriptorHeap = bilinearFiltering ? samplerDescriptorHeapBilinear : samplerDescriptorHeapPoint;
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpu = srvHeap->GetGPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE samplerGpu = samplerHeap->GetGPUDescriptorHandleForHeapStart();
		srvGpu.ptr += heapIndex * srvStep;
		samplerGpu.ptr += heapIndex * samplerStep;

		for (int i = 0; i < textureCount; ++i) {
			if (currentRenderTargets[i] != nullptr || currentTextures[i] != nullptr) {

				D3D12_CPU_DESCRIPTOR_HANDLE srvCpu = srvHeap->GetCPUDescriptorHandleForHeapStart();
				D3D12_CPU_DESCRIPTOR_HANDLE samplerCpu = samplerHeap->GetCPUDescriptorHandleForHeapStart();
				srvCpu.ptr += heapIndex * srvStep;
				samplerCpu.ptr += heapIndex * samplerStep;
				++heapIndex;

				if (currentRenderTargets[i] != nullptr) {
					bool is_depth = currentRenderTargets[i]->impl.stage_depth == i;
					D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu = is_depth ?
						currentRenderTargets[i]->impl.srvDepthDescriptorHeap->GetCPUDescriptorHandleForHeapStart() :
						currentRenderTargets[i]->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
					device->CopyDescriptorsSimple(1, srvCpu, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					device->CopyDescriptorsSimple(1, samplerCpu, samplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

					if (is_depth) {
						currentRenderTargets[i]->impl.stage_depth = -1;
						currentRenderTargets[i] = nullptr;
					}
				}
				else {
					D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu = currentTextures[i]->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
					device->CopyDescriptorsSimple(1, srvCpu, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					device->CopyDescriptorsSimple(1, samplerCpu, samplerDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
				}
			}
		}

		ID3D12DescriptorHeap* heaps[2] = {srvHeap, samplerHeap};
		commandList->SetDescriptorHeaps(2, heaps);
		commandList->SetGraphicsRootDescriptorTable(0, srvGpu);
		commandList->SetGraphicsRootDescriptorTable(1, samplerGpu);
	}
}

static void createSampler(bool bilinear, D3D12_FILTER filter) {
	D3D12_DESCRIPTOR_HEAP_DESC descHeapSampler = {};
	descHeapSampler.NumDescriptors = 2;
	descHeapSampler.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	descHeapSampler.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
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

void createSamplersAndHeaps() {
	createSampler(false, D3D12_FILTER_MIN_MAG_MIP_POINT);
	createSampler(true, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = heapSize;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&heapDesc, IID_GRAPHICS_PPV_ARGS(&srvHeap));

	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
	samplerHeapDesc.NumDescriptors = heapSize;
	samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&samplerHeapDesc, IID_GRAPHICS_PPV_ARGS(&samplerHeap));
}

void kinc_g5_texture_init_from_image(kinc_g5_texture_t *texture, kinc_image_t *image) {
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->impl.mipmap = true;
	texture->texWidth = image->width;
	texture->texHeight = image->height;

	DXGI_FORMAT d3dformat = convertFormat(image->format);
	int formatSize = formatByteSize(image->format);

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	                                &CD3DX12_RESOURCE_DESC::Tex2D(d3dformat, texture->texWidth, texture->texHeight, 1, 1),
	                                D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_GRAPHICS_PPV_ARGS(&texture->impl.image));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture->impl.image, 0, 1);
	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&texture->impl.uploadImage));

	texture->impl.stride = (int)kinc_ceil(uploadBufferSize / (float)image->height);
	if (texture->impl.stride < d3d12_textureAlignment()) {
		texture->impl.stride = d3d12_textureAlignment();
	}

	BYTE *pixel;
	texture->impl.uploadImage->Map(0, nullptr, reinterpret_cast<void **>(&pixel));
	int pitch = kinc_g5_texture_stride(texture);
	for (int y = 0; y < texture->texHeight; ++y) {
		memcpy(&pixel[y * pitch], &((uint8_t*)image->data)[y * texture->texWidth * formatSize], texture->texWidth * formatSize);
	}
	texture->impl.uploadImage->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

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

void kinc_g5_texture_init(kinc_g5_texture *texture, int width, int height, kinc_image_format_t format) {
	//kinc_image_init(&texture->image, width, height, format, readable);
	memset(&texture->impl, 0, sizeof(texture->impl));
	texture->impl.stage = 0;
	texture->impl.mipmap = true;
	texture->texWidth = width;
	texture->texHeight = height;

	DXGI_FORMAT d3dformat = convertFormat(format);

	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	                                &CD3DX12_RESOURCE_DESC::Tex2D(d3dformat, texture->texWidth, texture->texHeight, 1, 1), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_GRAPHICS_PPV_ARGS(&texture->impl.image));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture->impl.image, 0, 1);
	device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
	                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_GRAPHICS_PPV_ARGS(&texture->impl.uploadImage));

	texture->impl.stride = (int)kinc_ceil(uploadBufferSize / (float)height);
	if (texture->impl.stride < d3d12_textureAlignment()) {
		texture->impl.stride = d3d12_textureAlignment();
	}

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 1;

	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

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
	texture->impl.image->Release();
	texture->impl.uploadImage->Release();
	texture->impl.srvDescriptorHeap->Release();
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
	BYTE *pixel;
	texture->impl.uploadImage->Map(0, nullptr, reinterpret_cast<void **>(&pixel));
	return pixel;
}

void kinc_g5_texture_unlock(kinc_g5_texture *texture) {
	texture->impl.uploadImage->Unmap(0, nullptr);
}

void kinc_g5_texture_clear(kinc_g5_texture_t *texture, int x, int y, int z, int width, int height, int depth, unsigned color) {}

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
