#pragma once

namespace Kore {
	class Surface {
	public:
		static Surface* the();
		void (*Move)(int index, int x, int y);
		void (*TouchStart)(int index, int x, int y);
		void (*TouchEnd)(int index, int x, int y);
		
		//for backend
		void _move(int index, int x, int y);
		void _touchStart(int index, int x, int y);
		void _touchEnd(int index, int x, int y);
	};
}
