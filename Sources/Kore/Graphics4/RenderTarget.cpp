#include "pch.h"

#include "Graphics.h"

#include <Kinc/Graphics4/RenderTarget.h>

using namespace Kore;
using namespace Kore::Graphics4;

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	kinc_g4_render_target_use_color_as_texture(&kincRenderTarget, unit.kincUnit);
}
