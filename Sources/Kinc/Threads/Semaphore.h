#pragma once

#include <Kore/SemaphoreImpl.h>

namespace Kore {
	class Semaphore : public SemaphoreImpl {
	public:
		void create(int current, int max);
		void destroy();
		void release(int count = 1);
		void acquire();
		bool tryToAcquire(double seconds);
	};
}
