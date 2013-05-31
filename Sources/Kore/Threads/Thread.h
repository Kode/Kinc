#pragma once

namespace Kore {
	const uint MAX_THREADS = 8;

	class Thread {
	public:
	};

	void threadsInit();
	void threadsQuit();

	Thread* createAndRunThread(void (*thread)(void* param), void* param);
	// Folgende Funktionen für einen bestimmten SR_Thread nur von einem einzigen Thread aus aufrufen:
	void waitForThreadStopThenFree(Thread* sr);
	bool isThreadStoppedThenFree  (Thread* sr);
}