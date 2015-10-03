#pragma once

#include <Kore/Graphics/Graphics.h>
#include <Kore/Math/Matrix.h>
#include <d3d12.h>

static const int QUEUE_SLOT_COUNT = 3;
extern int currentBackBuffer;
extern ID3D12Device* device;
extern ID3D12RootSignature* rootSignature;
extern ID3D12GraphicsCommandList* commandList;
extern ID3D12Resource* constantBuffers[QUEUE_SLOT_COUNT];

extern Kore::u8 vertexConstants[1024 * 4];
extern Kore::u8 fragmentConstants[1024 * 4];
extern Kore::u8 geometryConstants[1024 * 4];
extern Kore::u8 tessControlConstants[1024 * 4];
extern Kore::u8 tessEvalConstants[1024 * 4];
