#pragma once

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;

namespace Kore {
	class RenderTargetImpl {
	public:
		ID3D11Texture2D* textureRender;
		ID3D11Texture2D* textureSample;
		ID3D11RenderTargetView* renderTargetViewRender[6];
		ID3D11RenderTargetView* renderTargetViewSample[6];
		ID3D11Texture2D* depthStencil;
		ID3D11DepthStencilView* depthStencilView[6];
		ID3D11ShaderResourceView* renderTargetSRV;
		ID3D11ShaderResourceView* depthStencilSRV;
		int lastBoundUnit;
		int lastBoundDepthUnit;
	};
}
