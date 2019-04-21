#include "pch.h"

#include "Graphics.h"

#include <Kinc/Graphics4/RenderTarget.h>

using namespace Kore;
using namespace Kore::Graphics4;

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	Kinc_G4_RenderTarget_UseColorAsTexture(&kincRenderTarget, unit.kincUnit);
}
