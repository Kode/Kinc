#pragma once

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;

namespace Kore {
	class RenderTargetImpl {
	public:
		ID3D11Texture2D* texture;
		ID3D11RenderTargetView* renderTargetView;
		ID3D11ShaderResourceView* view;
	};
}
