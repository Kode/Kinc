#include "pch.h"

#include "Graphics.h"

#include <kinc/graphics4/rendertarget.h>

using namespace Kore;
using namespace Kore::Graphics4;

void RenderTarget::useColorAsTexture(TextureUnit unit) {
	kinc_g4_render_target_use_color_as_texture(&kincRenderTarget, unit.kincUnit);
}
