#pragma once

#include <kinc/threads/thread.h>

namespace Kore {
	class Thread {
	public:
		kinc_thread_t thread;
	};

	void threadsInit();
	void threadsQuit();

	Thread *createAndRunThread(void (*func)(void *param), void *param);
	void waitForThreadStopThenFree(Thread *sr);
	bool isThreadStoppedThenFree(Thread *sr);
	void threadSleep(int milliseconds);
}
