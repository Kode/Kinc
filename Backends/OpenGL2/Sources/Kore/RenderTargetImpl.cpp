#include "pch.h"
#include "RenderTargetImpl.h"
#include <Kore/Graphics/Graphics.h>

using namespace Kore;

RenderTarget::RenderTarget(int width, int height, bool zBuffer, bool antialiasing, RenderTargetFormat format) {
	myWidth = width;
	myHeight = height;
}

void RenderTarget::useColorAsTexture(int texunit) {
	
}

int RenderTarget::width() {
	return myWidth;
}

int RenderTarget::height() {
	return myHeight;
}
