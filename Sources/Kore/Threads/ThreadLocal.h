#pragma once

#include <kinc/threads/threadlocal.h>

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
