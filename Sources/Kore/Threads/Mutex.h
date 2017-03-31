#pragma once

#if !defined(KORE_WINDOWS) && !defined(KORE_WINDOWSAPP) && defined(KORE_POSIX)
#include <pthread.h>
#endif

namespace Kore {
	class Mutex {
	public:
		void Create();
		void Free();
		void Lock();
		void Unlock();

	private:
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
		struct CriticalSection {
			void* DebugInfo;
			long LockCount;
			long RecursionCount;
			void* OwningThread;
			void* LockSemaphore;
			unsigned long __w64 SpinCount;
		} criticalSection;
#elif defined(KORE_POSIX)
		pthread_mutex_t pthread_mutex;
#endif
	};

	class UberMutex {
	public:
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
		void* id;
#endif

		bool Create(const wchar_t* name);
		void Free();

		void Lock();
		void Unlock();
	};
}
