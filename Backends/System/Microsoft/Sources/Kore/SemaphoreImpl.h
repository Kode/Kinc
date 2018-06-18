#pragma once

#include <Kore/Threads/Event.h>
#include <Kore/Threads/Mutex.h>

namespace Kore {
	class SemaphoreImpl {
	protected:
		Event event;
		Mutex mutex;
		int current;
		int max;
	};
}
