#pragma once

#include <Kore/SemaphoreImpl.h>

namespace Kore {
	class Semaphore : public SemaphoreImpl {
	public:
		void set();
		void waitForever();
		bool wait(double seconds);
	};
}
