#include "pch.h"
#include "Direct3D12.h"
#include "RenderTargetImpl.h"
#include <Kore/Graphics/Graphics.h>
#include <Kore/WinError.h>

using namespace Kore;

RenderTarget::RenderTarget(int width, int height, bool zBuffer, bool antialiasing, RenderTargetFormat format) {
	this->texWidth = this->width = width;
	this->texHeight = this->height = height;


}

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	if (unit.unit < 0) return;
	//context->PSSetShaderResources(unit.unit, 1, &view);
}
