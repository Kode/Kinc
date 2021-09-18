#pragma once

#include <kinc/threads/event.h>

namespace Kore {
	class Event {
	public:
		void create(bool autoClear = true);
		void destroy();
		void signal();
		void wait();
		bool tryToWait(double seconds);
		void reset();

	private:
		kinc_event_t event;
	};
}
