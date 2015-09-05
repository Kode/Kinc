#pragma once

#include <Kore/Graphics/Graphics.h>
#include <Kore/Math/Matrix.h>
#include <d3d12.h>

/*const int frameCount = 3;
extern ID3D12Device* device;
extern ID3D12GraphicsCommandList* commandList;
extern ID3D12RootSignature* rootSignature;
extern ID3D12CommandAllocator* bundleAllocator;
extern ID3D12CommandAllocator* commandAllocators[frameCount];
extern unsigned currentFrame;*/

static const int QUEUE_SLOT_COUNT = 3;
extern int currentBackBuffer_;
extern ID3D12Device* device_;
extern ID3D12RootSignature* rootSignature_;
extern ID3D12GraphicsCommandList* commandList;
extern ID3D12Resource* image_;
extern ID3D12Resource* uploadImage_;
extern ID3D12DescriptorHeap* srvDescriptorHeap_;
extern ID3D12Resource* constantBuffers_[QUEUE_SLOT_COUNT];

//extern ID3D12DeviceContext* context;
//extern ID3D12RenderTargetView* renderTargetView;
//extern ID3D12DepthStencilView* depthStencilView;

extern Kore::u8 vertexConstants[1024 * 4];
extern Kore::u8 fragmentConstants[1024 * 4];
extern Kore::u8 geometryConstants[1024 * 4];
extern Kore::u8 tessControlConstants[1024 * 4];
extern Kore::u8 tessEvalConstants[1024 * 4];
