#pragma once

#include <Kore/Graphics1/Color.h>

namespace Kore {
	namespace Graphics1 {
		void init(int width, int height);
		void begin();
		void end();
		void setPixel(int x, int y, float red, float green, float blue);
		int width();
		int height();
	}
}
