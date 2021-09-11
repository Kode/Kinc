#pragma once

#include <kinc/threads/threadlocal.h>

namespace Kore {
	class ThreadLocal {
	public:
		void create();
		void destroy();
		void *get();
		void set(void *);

	private:
		kinc_thread_local_t local;
	};
}
