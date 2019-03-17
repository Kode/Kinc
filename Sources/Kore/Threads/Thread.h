#pragma once

#include <Kinc/Threads/Thread.h>

namespace Kore {
	class Thread {
	public:
		Kinc_Thread thread;
	};

	void threadsInit();
	void threadsQuit();

	Thread *createAndRunThread(void (*func)(void *param), void *param);
	void waitForThreadStopThenFree(Thread *sr);
	bool isThreadStoppedThenFree(Thread *sr);
	void threadSleep(int milliseconds);
}
