#pragma once

#include <Kore/ThreadLocalImpl.h>

namespace Kore {
	class ThreadLocal : public ThreadLocalImpl {
	public:
		void* get();
		void set(void*);
	};
}
