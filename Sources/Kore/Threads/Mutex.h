#pragma once

#ifndef SYS_WINDOWS
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
	#ifdef SYS_WINDOWS
		struct CriticalSection {
			void* DebugInfo;
			long LockCount;
			long RecursionCount;
			void* OwningThread;
			void* LockSemaphore;
			unsigned long __w64 SpinCount;
		} criticalSection;
	#else
		pthread_mutex_t pthread_mutex;
	#endif
	};

	class UberMutex {
	public:
		#if defined SYS_WINDOWS
		void *id;
		#elif defined SYS_IPH
		#elif defined SYS_NDS
		#endif

		bool Create(const wchar_t *name);
		void Free  ();

		void Lock();
		void Unlock();
	};
	// Prozess-übergreifend (zur Zeit nur unter Windows).
}
