#pragma once

#include <Kore/EventImpl.h>

namespace Kore {
	class Event : public EventImpl {
	public:
		void create();
		void destroy();
		void signal();
		void wait();
		bool tryToWait(double seconds);
		void reset();
	};
}
