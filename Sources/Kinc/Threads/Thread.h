#pragma once

namespace Kore {
	const uint MAX_THREADS = 8;

	class Thread {
	public:
	};

	void threadsInit();
	void threadsQuit();

	Thread* createAndRunThread(void (*thread)(void* param), void* param);
	void waitForThreadStopThenFree(Thread* sr);
	bool isThreadStoppedThenFree(Thread* sr);
	void threadSleep(int milliseconds);
}
