#pragma once

#include <kinc/threads/semaphore.h>

namespace Kore {
	class Semaphore {
	public:
		void create(int current, int max);
		void destroy();
		void release(int count = 1);
		void acquire();
		bool tryToAcquire(double seconds);

	private:
		kinc_semaphore_t semaphore;
	};
}
