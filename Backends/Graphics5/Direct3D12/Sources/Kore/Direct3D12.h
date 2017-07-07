#pragma once

#include <Kore/Graphics5/Graphics.h>
#include <Kore/Math/Matrix.h>
#include <d3d12.h>
#ifdef KORE_WINDOWS
#include "d3dx12.h"
#endif

#ifndef IID_GRAPHICS_PPV_ARGS
#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)
#endif

static const int QUEUE_SLOT_COUNT = 3;
extern int currentBackBuffer;
extern ID3D12Device* device;
extern ID3D12RootSignature* rootSignature;
extern ID3D12GraphicsCommandList* commandList;

extern Kore::u8 vertexConstants[1024 * 4];
extern Kore::u8 fragmentConstants[1024 * 4];
extern Kore::u8 geometryConstants[1024 * 4];
extern Kore::u8 tessControlConstants[1024 * 4];
extern Kore::u8 tessEvalConstants[1024 * 4];
