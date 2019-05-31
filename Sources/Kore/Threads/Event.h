#pragma once

#include <kinc/threads/event.h>

namespace Kore {
	class Event {
	public:
		void create();
		void destroy();
		void signal();
		void wait();
		bool tryToWait(double seconds);
		void reset();
	private:
		Kinc_Event event;
	};
}
