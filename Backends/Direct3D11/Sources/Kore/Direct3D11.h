#pragma once

#include <Kore/Graphics/Graphics.h>
#include <Kore/Math/Matrix.h>
#ifdef SYS_WINDOWS8
#include <d3d11_1.h>
#else
#pragma warning(disable: 4005)
#include <d3d11.h>
#endif

extern ID3D11Device* device;
extern ID3D11DeviceContext* context;
extern ID3D11RenderTargetView* renderTargetView;
extern ID3D11DepthStencilView* depthStencilView;
extern Kore::u8 vertexConstants[1024 * 4];
extern Kore::u8 fragmentConstants[1024 * 4];
extern Kore::u8 geometryConstants[1024 * 4];
extern Kore::u8 tessControlConstants[1024 * 4];
extern Kore::u8 tessEvalConstants[1024 * 4];
