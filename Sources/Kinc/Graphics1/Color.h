#pragma once

namespace Kore {
	namespace Graphics1 {
		class Color {

		private:
			void getColorFromHex(uint color, float& red, float& green, float& blue, float& alpha);

		public:
			Color(uint color);

			float R;
			float G;
			float B;
			float A;

			static const uint Black = 0xff000000;
			static const uint White = 0xffffffff;
			static const uint Red = 0xffff0000;
			static const uint Blue = 0xff0000ff;
			static const uint Green = 0xff00ff00;
			static const uint Magenta = 0xffff00ff;
			static const uint Yellow = 0xffffff00;
			static const uint Cyan = 0xff00ffff;
			static const uint Purple = 0xff800080;
			static const uint Pink = 0xffffc0cb;
			static const uint Orange = 0xffffa500;
		};
	}
}
