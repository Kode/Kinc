#pragma once

namespace Kore {
	class Surface {
	public:
		static Surface* the();
		void (*Move)(int index, int x, int y);
		void (*TouchStart)(int index, int x, int y);
		void (*TouchEnd)(int index, int x, int y);
		Surface();
	};
}
