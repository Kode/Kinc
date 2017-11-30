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
//extern ID3D12GraphicsCommandList* commandList;
