#pragma once

#include <Kinc/Threads/Event.h>

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
