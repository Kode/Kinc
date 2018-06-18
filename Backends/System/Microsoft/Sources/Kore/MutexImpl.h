#pragma once

struct _RTL_CRITICAL_SECTION;

namespace Kore {
	class MutexImpl {
	protected:
		struct CriticalSection {
			void* DebugInfo;
			long LockCount;
			long RecursionCount;
			void* OwningThread;
			void* LockSemaphore;
			unsigned long SpinCount;
		} criticalSection;
	};
}
