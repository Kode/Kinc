#pragma once

#include <Kore/MutexImpl.h>

#if !defined(KORE_WINDOWS) && !defined(KORE_WINDOWSAPP) && defined(KORE_POSIX)
#include <pthread.h>
#endif

namespace Kore {
	class Mutex : public MutexImpl {
	public:
		void create();
		void destroy();
		void lock();
		bool tryToLock();
		void unlock();

	private:
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_XBOX_ONE)
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

		bool create(const wchar_t* name);
		void destroy();
		void lock();
		void unlock();
	};
}
