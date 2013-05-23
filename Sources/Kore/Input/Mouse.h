#pragma once

#include "MouseEvent.h"

namespace Kore {
	class Mouse {
	public:
		static Mouse* the();
		void (*Move)(MouseEvent);
		void (*PressLeft)(MouseEvent);
		void (*PressRight)(MouseEvent);
		void (*ReleaseLeft)(MouseEvent);
		void (*ReleaseRight)(MouseEvent);
		
		//for backend
		void _move(MouseEvent);
		void _pressLeft(MouseEvent);
		void _pressRight(MouseEvent);
		void _releaseLeft(MouseEvent);
		void _releaseRight(MouseEvent);
	};
}
