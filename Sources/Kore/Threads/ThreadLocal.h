#pragma once

#include <Kinc/Threads/ThreadLocal.h>

namespace Kore {
	class ThreadLocal {
	public:
		void create();
		void destroy();
		void* get();
		void set(void*);
	private:
		Kinc_ThreadLocal local;
	};
}
