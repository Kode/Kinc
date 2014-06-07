#pragma once

#include "MouseEvent.h"

namespace Kore {
	class Mouse {
	public:
		static Mouse* the();
		void (*Move)(MouseEvent);
		void (*PressLeft)(MouseEvent);
		void (*PressMiddle)(MouseEvent);
		void (*PressRight)(MouseEvent);
		void (*ReleaseLeft)(MouseEvent);
		void (*ReleaseMiddle)(MouseEvent);
		void (*ReleaseRight)(MouseEvent);
		
		//for backend
		void _move(MouseEvent);
		void _pressLeft(MouseEvent);
		void _pressMiddle(MouseEvent);
		void _pressRight(MouseEvent);
		void _releaseLeft(MouseEvent);
		void _releaseMiddle(MouseEvent);
		void _releaseRight(MouseEvent);
	};
}
