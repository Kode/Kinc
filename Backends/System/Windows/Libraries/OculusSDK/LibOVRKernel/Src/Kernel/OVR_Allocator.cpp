/************************************************************************************

Filename    :   OVR_Allocator.cpp
Content     :   Installable memory allocator implementation
Created     :   September 19, 2012
Notes       :

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License");
you may not use the Oculus VR Rift SDK except in compliance with the License,
which is provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-3.3

Unless required by applicable law or agreed to in writing, the Oculus VR SDK
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#include "OVR_Allocator.h"
#include "OVR_DebugHelp.h"
#include "OVR_Alg.h"
#include "OVR_Std.h"
#include "Util/Util_SystemInfo.h"
#include <stdlib.h>
#include <stdio.h>
#include <exception>
#include <algorithm>
#include <sstream>
#include <memory>

#if defined(_MSC_VER)
#include <crtdbg.h>
OVR_DISABLE_MSVC_WARNING(4996) // 'sscanf': This function or variable may be unsafe.
OVR_DISABLE_MSVC_WARNING(4351) // elements of array will be default initialized
#endif

#if !defined(WIN32)
#include <unistd.h>
#include <sys/mman.h>
#include <execinfo.h>
#include <malloc/malloc.h>
#endif

#if !defined(OVR_DEBUG_TRACE)
#if defined(_WIN32)
#define OVR_DEBUG_TRACE(str) ::OutputDebugStringA(str);
#else
#define OVR_DEBUG_TRACE(str) ::fputs(str, stderr);
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_DEBUG_CRT_PRESENT
//
// Defined as 0 or 1. 0 means release CRT is present (release build).
// Indicates if the debug version or the CRT will be linked into the application.
// If it's present then malloc hooking will need to include the debug malloc functions
// that are present in the debug CRT. If it's not present then we need to not include
// them, as they won't be present.
// You can't modify this value' it's a property of the build environment.
//
#if !defined(OVR_DEBUG_CRT_PRESENT)
#if defined(_DEBUG)
#define OVR_DEBUG_CRT_PRESENT 1
#else
#define OVR_DEBUG_CRT_PRESENT 0
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_STATIC_CRT_PRESENT
//
// Defined as 0 or 1. 0 means DLL CRT, static means static CRT.
// Indicates if the CRT being compiled against is the static CRT or the DLL CRT.
// You can't modify this value' it's a property of the build environment.
//
#if !defined(OVR_STATIC_CRT_PRESENT)
#if defined(_DLL) // VC++ defines _DLL if you are building against the DLL CRT.
#define OVR_STATIC_CRT_PRESENT 0
#else
#define OVR_STATIC_CRT_PRESENT 1
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_ALLOCATOR_DEBUG_PAGE_HEAP_ENABLED
//
// Defined as 0 or 1.
// If enabled then we use our debug page heap instead of a regular heap by default.
// However, even if this is disabled it can still be enabled at runtime by manually
// setting the appropriate environment variable/registry key.
// The debug page heap is available only with 64 bit platforms, as the page use is
// too high for some 32 bit platforms.
//
#ifndef OVR_ALLOCATOR_DEBUG_PAGE_HEAP_ENABLED
#if defined(OVR_BUILD_DEBUG)
// Disabled because currently the Oculus OAF module usage of this feature makes the app slow.
#define OVR_ALLOCATOR_DEBUG_PAGE_HEAP_ENABLED 0
#else
#define OVR_ALLOCATOR_DEBUG_PAGE_HEAP_ENABLED 0
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_ALLOCATOR_TRACKING_ENABLED
//
// Defined as 0 or 1.
// If enabled then memory is tracked by default and reports on it can be done at runtime.
// However, even if this is disabled it can still be enabled at runtime by manually
// setting the appropriate environment variable/registry key:
// HKEY_LOCAL_MACHINE\SOFTWARE\Oculus\HeapTrackingEnabled
//
#ifndef OVR_ALLOCATOR_TRACKING_ENABLED
#if defined(OVR_BUILD_DEBUG)
// Disabled because currently Microsoft iterator debugging makes the app slow.
#define OVR_ALLOCATOR_TRACKING_ENABLED 0
#else
#define OVR_ALLOCATOR_TRACKING_ENABLED 0
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_REDIRECT_CRT_MALLOC
//
// Defined as 0 or 1.
// If enabled and if a default allocator is used, then the malloc family of functions
// is redirected to the default allocator. This allows memory tracking of malloc in
// addition to operator new.
//
#ifndef OVR_REDIRECT_CRT_MALLOC
#if defined(_MSC_VER) && defined(_DEBUG) && (_MSC_VER >= 1900) && \
    OVR_STATIC_CRT_PRESENT // Not supported for DLL CRT until we can figure out how to work around
// some difficulties.
#define OVR_REDIRECT_CRT_MALLOC 0 // Disabled until we are confident in it.
#else
#define OVR_REDIRECT_CRT_MALLOC 0
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** CRT internal symbols we use.
//
#if defined(_MSC_VER)
#if OVR_STATIC_CRT_PRESENT
#if (_MSC_VER < 1900)
extern "C" HANDLE _crtheap;
#else
extern "C" void __cdecl __acrt_lock(int lock);
extern "C" void __cdecl __acrt_unlock(int lock);
extern "C" HANDLE __acrt_heap;
#endif
#else
// The CRT locks are not accessible from outside the MSVCRT DLL.
// Maybe it's privately exported through an export table via ordinal number, though.
#if (_MSC_VER >= 1900)
extern "C" void __cdecl __acrt_lock(int /*lock*/) {
  OVR_FAIL();
} // We don't currently have a way to access this, but we don't support this pattern of usage
// anyway.
extern "C" void __cdecl __acrt_unlock(int /*lock*/) {
  OVR_FAIL();
}
extern "C" HANDLE __cdecl __acrt_get_msvcrt_heap_handle() {
  OVR_FAIL();
  return NULL;
}
#endif
#endif

inline HANDLE GetCRTHeapHandle() {
#if OVR_STATIC_CRT_PRESENT
#if (_MSC_VER < 1900) // If VS2013 or earlier...
  return _crtheap;
#else
  return __acrt_heap;
#endif
#else
#if (_MSC_VER < 1900) // If VS2013 or earlier...
#error "Need to find the function that does this"
#else
  return __acrt_get_msvcrt_heap_handle();
#endif
#endif
}

#if OVR_DEBUG_CRT_PRESENT
// We need to replicate a couple items from the debug CRT heap. This may change with
// future VC++ versions, though that's unlikely and wouldn't likely change by much.
struct CrtMemBlockHeader {
  CrtMemBlockHeader* block_header_next;
  CrtMemBlockHeader* block_header_prev;
  const char* file_name;
  int line_number;
#if defined(_WIN64) || (_MSC_VER >= 1900)
  int block_use;
  size_t data_size;
#else
  size_t data_size;
  int block_use;
#endif
  long request_number;
  unsigned char gap[4];
  // unsigned char    data[data_size]; // User pointer.
  // unsigned char    another_gap[4];
};

static const CrtMemBlockHeader* header_from_block(const void* block) {
  return (static_cast<const CrtMemBlockHeader*>(block) - 1);
}

// Clone of _malloc_dbg
#if (_MSC_VER >= 1900)
static const uint8_t no_mans_land_fill = 0xFD;
static const uint8_t clean_land_fill = 0xCD;
static const size_t no_mans_land_size = 4;
static const long request_number_for_ignore_blocks = 0;
static const int line_number_for_ignore_blocks = static_cast<int>(0xFEDCBABC);

static void* block_from_header(void* header) {
  return (static_cast<CrtMemBlockHeader*>(header) + 1);
}

extern "C" void* crt_malloc_dbg(size_t size, int /*blockUse*/, const char* /*file*/, int /*line*/) {
  struct AutoLock {
    AutoLock() {
      __acrt_lock(0);
    }
    ~AutoLock() {
      __acrt_unlock(0);
    }
  } autoLock;
  void* block = nullptr;

  if (size > (size_t)((_HEAP_MAXREQ - no_mans_land_size) - sizeof(CrtMemBlockHeader))) {
    errno = ENOMEM;
    return nullptr;
  }

  size_t const block_size = sizeof(CrtMemBlockHeader) + size + no_mans_land_size;
  CrtMemBlockHeader* header =
      static_cast<CrtMemBlockHeader*>(HeapAlloc(GetCRTHeapHandle(), 0, block_size));

  if (!header) {
    errno = ENOMEM;
    return nullptr;
  }

  // Set this block to be ignored by the debug heap. This makes it somewhat invisible.
  header->block_header_next = nullptr;
  header->block_header_prev = nullptr;
  header->file_name = nullptr;
  header->line_number = line_number_for_ignore_blocks;
  header->data_size = size;
  header->block_use = _IGNORE_BLOCK;
  header->request_number = request_number_for_ignore_blocks;

  memset(header->gap, no_mans_land_fill, no_mans_land_size);
  memset((char*)block_from_header(header) + size, no_mans_land_fill, no_mans_land_size);
  memset(block_from_header(header), clean_land_fill, size);

  block = block_from_header(header);

  return block;
}
#else
#if OVR_STATIC_CRT_PRESENT
extern "C" void* __cdecl _nh_malloc_dbg(
    size_t size,
    int /*flag*/,
    int nBlockUse,
    const char* file,
    int line);

extern "C" void* crt_malloc_dbg(size_t size, int blockUse, const char* file, int line) {
  return _nh_malloc_dbg(size, 0, blockUse, file, line);
}
#endif
#endif
#elif OVR_STATIC_CRT_PRESENT
extern "C" void* __cdecl _aligned_malloc_base(size_t size, size_t align);
extern "C" void __cdecl _free_base(void* p);
// extern "C" void* __cdecl _realloc_base(void* p, size_t newsize);
#if defined(WINDOWS_SDK_VERSION) && WINDOWS_SDK_VERSION >= 17763
// These functions aren't available in the 10.0.17763.0 Windows SDK.
extern "C" void __cdecl _aligned_free_base(void* /*p*/) {
  OVR_FAIL();
}
extern "C" void* __cdecl _aligned_realloc_base(void* /*p*/, size_t /*newSize*/, size_t /*align*/) {
  OVR_FAIL();
  return nullptr;
}
#else
extern "C" void __cdecl _aligned_free_base(void* p);
extern "C" void* __cdecl _aligned_realloc_base(void* p, size_t newSize, size_t align);
#endif
#else
extern "C" _ACRTIMP void __cdecl _free_base(void* p);
extern "C" void __cdecl _aligned_free_base(void* /*p*/) {
  OVR_FAIL();
}
extern "C" void* __cdecl _aligned_realloc_base(void* /*p*/, size_t /*newSize*/, size_t /*align*/) {
  OVR_FAIL();
  return nullptr;
}
#endif
#endif // defined(_MSC_VER)

//-----------------------------------------------------------------------------------
// ***** OVR_ALLOCATOR_UNSPECIFIED_TAG
//
#define OVR_ALLOCATOR_UNSPECIFIED_TAG "none"

//-----------------------------------------------------------------------------------
// ***** OVR_USE_JEMALLOC
//
// Defined as 0 or 1.
// If enabled then jemalloc is used as a heap instead of other heaps.
//
#ifndef OVR_USE_JEMALLOC
// Currently unilaterally disabled because our jemalloc has stability problems on Windows.
#define OVR_USE_JEMALLOC 0
#endif
#if OVR_USE_JEMALLOC
#include "src/jemalloc/jemalloc.h"
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_HUNT_UNTRACKED_ALLOCS
//
// Defined as 0 or 1.
// This will cause an assertion to trip whenever an allocation occurs outside of our
// custom allocator. This helps track down allocations that are not being done
// correctly via OVR_ALLOC().
//
#ifndef OVR_HUNT_UNTRACKED_ALLOCS
#define OVR_HUNT_UNTRACKED_ALLOCS 0
#endif

#if OVR_HUNT_UNTRACKED_ALLOCS

static const char* WhiteList[] = {"OVR_Allocator.cpp",
                                  "OVR_Log.cpp",
                                  "crtw32", // Ignore CRT internal allocations
                                  nullptr};

static int
HuntUntrackedAllocHook(int, void*, size_t, int, long, const unsigned char* szFileName, int) {
  if (!szFileName) {
    return TRUE;
  }

  for (int i = 0; WhiteList[i] != nullptr; ++i) {
    if (strstr((const char*)szFileName, WhiteList[i]) != 0) {
      return TRUE;
    }
  }

  // At this point we have an allocation that's occurring ourside our custom allocator.
  // It means that the application is going around our OVR_ALLOC interface.
  OVR_ASSERT(false);
  return FALSE;
}

#endif // OVR_HUNT_UNTRACKED_ALLOCS

//-----------------------------------------------------------------------------------
// ***** OVR_BENCHMARK_ALLOCATOR
//
// Defined as 0 or 1.
// If we are benchmarking the allocator, define this.
// Do not enable this in shipping code!
//
#ifndef OVR_BENCHMARK_ALLOCATOR
#define OVR_BENCHMARK_ALLOCATOR 0
#endif

#if OVR_BENCHMARK_ALLOCATOR
#error \
    "This code should not be compiled!  It really hurts performance.  Only enable this during testing."

// This gets the double constant that can convert ::QueryPerformanceCounter
// LARGE_INTEGER::QuadPart into a number of seconds.
// This is the same as in the Timer code except we cannot use Timer code
// because the allocator gets called during static initializers before
// the Timer code is initialized.
static double GetPerfFrequencyInverse() {
  // Static value containing frequency inverse of performance counter
  static double PerfFrequencyInverse = 0.;

  // If not initialized,
  if (PerfFrequencyInverse == 0.) {
    // Initialize the inverse (same as in Timer code)
    LARGE_INTEGER freq;
    ::QueryPerformanceFrequency(&freq);
    PerfFrequencyInverse = 1.0 / (double)freq.QuadPart;
  }

  return PerfFrequencyInverse;
}

// Record a delta timestamp for an allocator operation
static void ReportDT(LARGE_INTEGER& t0, LARGE_INTEGER& t1) {
  // Stats lock to avoid multiple threads corrupting the shared stats
  // This lock is the reason we cannot enable this code.
  static Lock theLock;

  // Running stats
  static double timeSum = 0.; // Sum of dts
  static double timeMax = 0.; // Max dt in set
  static int timeCount = 0; // Number of dts recorded

  // Calculate delta time between start and end of operation
  // based on the provided QPC timestamps
  double dt = (t1.QuadPart - t0.QuadPart) * GetPerfFrequencyInverse();

  // Init the average and max to print to zero.
  // If they stay zero we will not print them.
  double ravg = 0., rmax = 0.;
  {
    // Hold the stats lock
    Lock::Locker locker(&theLock);

    // Accumulate stats
    timeSum += dt;
    if (dt > timeMax)
      timeMax = dt;

    // Every X recordings,
    if (++timeCount >= 1000) {
      // Set average/max to print
      ravg = timeSum / timeCount;
      rmax = timeMax;

      timeSum = 0;
      timeMax = 0;
      timeCount = 0;
    }
  }

  // If printing,
  if (rmax != 0.) {
    LogText(
        "------- Allocator Stats: AvgOp = %lf usec, MaxOp = %lf usec\n",
        ravg * 1000000.,
        rmax * 1000000.);
  }
}

#define OVR_ALLOC_BENCHMARK_START() \
  LARGE_INTEGER t0;                 \
  ::QueryPerformanceCounter(&t0);
#define OVR_ALLOC_BENCHMARK_END() \
  LARGE_INTEGER t1;               \
  ::QueryPerformanceCounter(&t1); \
  ReportDT(t0, t1);
#else
#define OVR_ALLOC_BENCHMARK_START()
#define OVR_ALLOC_BENCHMARK_END()
#endif // OVR_BENCHMARK_ALLOCATOR

namespace OVR {

bad_alloc::bad_alloc(const char* description) OVR_NOEXCEPT {
  if (description)
    OVR_strlcpy(Description, description, sizeof(Description));
  else
    Description[0] = '\0';

  OVR_strlcat(Description, " at ", sizeof(Description));

  // read the current backtrace
  // We cannot attempt to symbolize this here as that would attempt to
  // allocate memory. That would be unwise within a bad_alloc exception.
  void* backtrace_data[20];
  char addressDescription[256] =
      {}; // Write into this temporary instead of member Description in case an exception is thrown.

#if defined(_WIN32)
  int count = CaptureStackBackTrace(
      2, sizeof(backtrace_data) / sizeof(backtrace_data[0]), backtrace_data, nullptr);
#else
  int count = backtrace(backtrace_data, sizeof(backtrace_data) / sizeof(backtrace_data[0]));
#endif

  for (int i = 0; i < count; ++i) {
    char address[(sizeof(void*) * 2) + 1 + 1]; // hex address string plus possible space plus null
    // terminator.
    snprintf(address, sizeof(address), "%p%s", backtrace_data[i], (i + 1 < count) ? " " : "");
    OVR_strlcat(addressDescription, address, sizeof(addressDescription));
  }

  OVR_strlcat(Description, addressDescription, sizeof(Description));
}

//-----------------------------------------------------------------------------------
// ***** SysMemAlloc / SysMemFree
//
void* SysMemAlloc(size_t n) {
#if defined(_WIN32)
  void* p = HeapAlloc(GetProcessHeap(), 0, n);
  return p;
#else
  // To do: Need to replace this with a true system memory source.
  return malloc(n);
#endif
}

void SysMemFree(void* p, size_t /*n*/) {
#if defined(_WIN32)
  if (p)
    HeapFree(GetProcessHeap(), 0, p);
#else
  // To do: Need to replace this with a true system memory source.
  free(p);
#endif
}

// To consider: Move Symbols to the Allocator class member data. The problem with that
// is that it exposes the debug interface from header file, which can be done but we
// would rather not if possible.
static OVR::SymbolLookup Symbols;

//-----------------------------------------------------------------------------------
// ***** InterceptCRTMalloc
//
// Intercept malloc and posssibly redirect it to an alternative.
//
// Primary use cases:
//     - We want to hook (listen in on calls to) the existing malloc but not replace it.
//     - We want to replace the existing malloc.
//
// Requirements:
//     - We need to be able to hook malloc after it has already been used. This necessary
//       because main startup function called directly by the OS does mallocs before the
//       application code can possibly override it (without prohibitively invasive alternatives).
//     - We need to be able to stop what we are doing at any point and restore the system
//       to how it was before we startd intercepting it.
//
// VC++ doesn't support overriding malloc and so we have to do it manually, the hard way.
// There are many functions (at least 28) that need overriding in order to fully and properly
// handle this.
//
class InterceptCRTMalloc {
 public:
  typedef void* (*ovr_malloc_type)(size_t size);
  typedef void* (*ovr_calloc_type)(size_t count, size_t size);
  typedef void* (*ovr_realloc_type)(void* p, size_t newSize);
  typedef void* (*ovr_recalloc_type)(void* p, size_t count, size_t size);
  typedef void* (*ovr_expand_type)(void* p, size_t newSize);
  typedef size_t (*ovr_msize_type)(void* p);
  typedef void (*ovr_free_type)(void* p);
  typedef void* (*ovr_aligned_malloc_type)(size_t size, size_t align);
  typedef void* (*ovr_aligned_offset_malloc_type)(size_t size, size_t align, size_t offset);
  typedef void* (*ovr_aligned_realloc_type)(void* p, size_t size, size_t align);
  typedef void* (
      *ovr_aligned_offset_realloc_type)(void* p, size_t size, size_t align, size_t offset);
  typedef void* (*ovr_aligned_recalloc_type)(void* p, size_t count, size_t size, size_t align);
  typedef void* (*ovr_aligned_offset_recalloc_type)(
      void* p,
      size_t count,
      size_t size,
      size_t align,
      size_t offset);
  typedef size_t (*ovr_aligned_msize_type)(void* p, size_t align, size_t offset);
  typedef void (*ovr_aligned_free_type)(void* p);
  typedef void* (*ovr_malloc_dbg_type)(size_t size, int blockUse, const char* file, int line);
  typedef void* (
      *ovr_calloc_dbg_type)(size_t count, size_t size, int blockUse, const char* file, int line);
  typedef void* (
      *ovr_realloc_dbg_type)(void* p, size_t newSize, int blockUse, const char* file, int line);
  typedef void* (*ovr_recalloc_dbg_type)(
      void* p,
      size_t count,
      size_t size,
      int blockUse,
      const char* file,
      int line);
  typedef void* (
      *ovr_expand_dbg_type)(void* p, size_t newSize, int blockType, const char* file, int line);
  typedef size_t (*ovr_msize_dbg_type)(void* p, int blockUse);
  typedef void (*ovr_free_dbg_type)(void* p, int blockUse);
  typedef void* (
      *ovr_aligned_malloc_dbg_type)(size_t size, size_t align, const char* file, int line);
  typedef void* (*ovr_aligned_offset_malloc_dbg_type)(
      size_t size,
      size_t align,
      size_t offset,
      const char* file,
      int line);
  typedef void* (*ovr_aligned_realloc_dbg_type)(
      void* p,
      size_t size,
      size_t align,
      const char* file,
      int line);
  typedef void* (*ovr_aligned_offset_realloc_dbg_type)(
      void* p,
      size_t size,
      size_t align,
      size_t offset,
      const char* file,
      int line);
  typedef void* (*ovr_aligned_recalloc_dbg_type)(
      void* p,
      size_t count,
      size_t size,
      size_t align,
      const char* file,
      int line);
  typedef void* (*ovr_aligned_offset_recalloc_dbg_type)(
      void* p,
      size_t count,
      size_t size,
      size_t align,
      size_t offset,
      const char* file,
      int line);
  typedef size_t (*ovr_aligned_msize_dbg_type)(void* p, size_t align, size_t offset);
  typedef void (*ovr_aligned_free_dbg_type)(void* p);

  // Grouping of all the malloc functions into a struct.
  struct MallocFunctionPointers {
    ovr_malloc_type malloc_ptr;
    ovr_calloc_type calloc_ptr;
    ovr_realloc_type realloc_ptr;
    ovr_recalloc_type recalloc_ptr;
    ovr_expand_type expand_ptr;
    ovr_msize_type msize_ptr;
    ovr_free_type free_ptr;
    ovr_aligned_malloc_type aligned_malloc_ptr;
    ovr_aligned_offset_malloc_type aligned_offset_malloc_ptr;
    ovr_aligned_realloc_type aligned_realloc_ptr;
    ovr_aligned_offset_realloc_type aligned_offset_realloc_ptr;
    ovr_aligned_recalloc_type aligned_recalloc_ptr;
    ovr_aligned_offset_recalloc_type aligned_offset_recalloc_ptr;
    ovr_aligned_msize_type aligned_msize_ptr;
    ovr_aligned_free_type aligned_free_ptr;
    ovr_malloc_dbg_type malloc_dbg_ptr;
    ovr_calloc_dbg_type calloc_dbg_ptr;
    ovr_realloc_dbg_type realloc_dbg_ptr;
    ovr_recalloc_dbg_type recalloc_dbg_ptr;
    ovr_expand_dbg_type expand_dbg_ptr;
    ovr_msize_dbg_type msize_dbg_ptr;
    ovr_free_dbg_type free_dbg_ptr;
    ovr_aligned_malloc_dbg_type aligned_malloc_dbg_ptr;
    ovr_aligned_offset_malloc_dbg_type aligned_offset_malloc_dbg_ptr;
    ovr_aligned_realloc_dbg_type aligned_realloc_dbg_ptr;
    ovr_aligned_offset_realloc_dbg_type aligned_offset_realloc_dbg_ptr;
    ovr_aligned_recalloc_dbg_type aligned_recalloc_dbg_ptr;
    ovr_aligned_offset_recalloc_dbg_type aligned_offset_recalloc_dbg_ptr;
    ovr_aligned_msize_dbg_type aligned_msize_dbg_ptr;
    ovr_aligned_free_dbg_type aligned_free_dbg_ptr;
  };

#if defined(_WIN32)
#include <crtdbg.h>
#endif

 protected:
  struct SavedMallocFunctions {
    OVR::SavedFunction malloc_saved;
    OVR::SavedFunction calloc_saved;
    OVR::SavedFunction realloc_saved;
    OVR::SavedFunction recalloc_saved;
    OVR::SavedFunction expand_saved;
    OVR::SavedFunction msize_saved;
    OVR::SavedFunction free_saved;
    OVR::SavedFunction aligned_malloc_saved;
    OVR::SavedFunction aligned_offset_malloc_saved;
    OVR::SavedFunction aligned_realloc_saved;
    OVR::SavedFunction aligned_offset_realloc_saved;
    OVR::SavedFunction aligned_recalloc_saved;
    OVR::SavedFunction aligned_offset_recalloc_saved;
    OVR::SavedFunction aligned_msize_saved;
    OVR::SavedFunction aligned_free_saved;
    OVR::SavedFunction malloc_dbg_saved;
    OVR::SavedFunction calloc_dbg_saved;
    OVR::SavedFunction realloc_dbg_saved;
    OVR::SavedFunction recalloc_dbg_saved;
    OVR::SavedFunction expand_dbg_saved;
    OVR::SavedFunction msize_dbg_saved;
    OVR::SavedFunction free_dbg_saved;
    OVR::SavedFunction aligned_malloc_dbg_saved;
    OVR::SavedFunction aligned_offset_malloc_dbg_saved;
    OVR::SavedFunction aligned_realloc_dbg_saved;
    OVR::SavedFunction aligned_offset_realloc_dbg_saved;
    OVR::SavedFunction aligned_recalloc_dbg_saved;
    OVR::SavedFunction aligned_offset_recalloc_dbg_saved;
    OVR::SavedFunction aligned_msize_dbg_saved;
    OVR::SavedFunction aligned_free_dbg_saved;
  };

 public:
  InterceptCRTMalloc();
  ~InterceptCRTMalloc();

  // Replaces the existing CRT malloc functions with the ones referred to by mallocFunctionPointers.
  bool ReplaceCRTMalloc(InterceptCRTMalloc::MallocFunctionPointers& mallocFunctionPointers);

  bool RestoreCRTMalloc();

  const SavedMallocFunctions& SavedFunctions() const {
    return SavedMallocFunctions;
  }

 protected:
  bool MallocReplaced; // True if ReplaceMalloc was successfully called.
  bool CRTMallocPreserved; // If true then PreservedMallocFunctionPointers is valid. It turns out
  // this is possible only if malloc is implemented as a jump to a real
  // malloc implementation.
  MallocFunctionPointers
      PreservedMallocFunctionPointers; // Points to callable vesions of the CRT malloc functions.
  bool MallocFunctionsSaved; // If true then SavedMallocFunctions contains saved functions. This
  // will typically be always true if MallocReplaced is true. Possibly we
  // can omit this variable.
  SavedMallocFunctions SavedMallocFunctions; //
};

InterceptCRTMalloc::InterceptCRTMalloc()
    : MallocReplaced(false),
      CRTMallocPreserved(false),
      PreservedMallocFunctionPointers(),
      MallocFunctionsSaved(false),
      SavedMallocFunctions() {}

InterceptCRTMalloc::~InterceptCRTMalloc() {
  // Default behavior is to not RestoreCRTMalloc().
}

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmicrosoft-cast"
#endif
bool InterceptCRTMalloc::ReplaceCRTMalloc(
    InterceptCRTMalloc::MallocFunctionPointers& mallocFunctionPointers) {
  // Malloc calls are made by seemingly one of three means:
  //
  // 1) static CRT (debug builds):
  //     E8 42 38 00 00       call malloc
  //         ->
  //     E9 rel32             This is a direct intra-module 32 bit relative jump. E9 cd -- JMP rel32
  //     -- Jump near, relative, RIP = RIP + 32-bit displacement sign extended to 64-bits
  //         ->
  //     <impl>
  //
  // 2) static CRT (release builds):
  //     E8 88 1F 00 00       call malloc
  //         ->
  //     <impl>
  //
  // 3) DLL CRT (debug and release builds):
  //     FF 15 6B DD 00 00    call qword ptr [__imp_malloc]  This is an indirect jump to the 64 bit
  //     destination stored at the memory location 0000DD6B bytes from this location. __imp__malloc
  //     is an internal variable name which refers to this memory location. FF /5 -- JMP m16:64 --
  //     Jump far, absolute indirect, address given in m16:64.
  //         ->
  //     <impl>
  //
  // In cases 1 and 3 above, malloc is called through a jump. That means we can replace the jump but
  // leave the jump destination (actual malloc implementation) unmodified and thus potentially
  // callable. There isn't a good solution for dealing with case #2, because in that case we would
  // be overwriting the bytes of the actual malloc implementation, this making it uncallable in the
  // future (without the risky trick of modifying the bytes as we call them). Copying the malloc
  // instruction memory to a new location is not an option because malloc's implementation will have
  // relative addess redirects which would break if it was in a different location.

  // We currently just set all the pointers to NULL. In the future we will want to make it so that
  // in the case that malloc is a jump to a real malloc, we will want to set these pointers to the
  // real malloc that it jumps to, so that we have the option of directly calling that malloc
  // through PreservedMallocFunctionPointers.
  PreservedMallocFunctionPointers = MallocFunctionPointers();
  CRTMallocPreserved = false;

// We modify the initial instruction bytes of the CRT malloc functions to instead jump to the
// malloc functions specified by mallocFunctionPointers. Note that the initial CRT malloc function
// instruction bytes may be merly a jump to the real CRT malloc function implementation.
//
// The code here redirects functions by replacing their initial instruction bytes with instructions
// that redirect (jmp) execution to our own code. A primary risk of this is that the replacement
// will happen while the bytes are being executed by another thread, in which case the results are
// unpredictable and likely exceptional. We need to be careful to do this at some known safe time,
// such as before the application has started executing additional threads.
//
#if OVR_STATIC_CRT_PRESENT && defined(_MSC_VER)
  // Override malloc, which is the CRT malloc function or code to jump to the CRT malloc function.
  RedirectCdeclFunction(
      malloc, mallocFunctionPointers.malloc_ptr, &SavedMallocFunctions.malloc_saved);
  RedirectCdeclFunction(
      calloc, mallocFunctionPointers.calloc_ptr, &SavedMallocFunctions.calloc_saved);
  RedirectCdeclFunction(
      realloc, mallocFunctionPointers.realloc_ptr, &SavedMallocFunctions.realloc_saved);
  RedirectCdeclFunction(
      _recalloc, mallocFunctionPointers.recalloc_ptr, &SavedMallocFunctions.recalloc_saved);
  RedirectCdeclFunction(
      _expand, mallocFunctionPointers.expand_ptr, &SavedMallocFunctions.expand_saved);
  RedirectCdeclFunction(
      _msize, mallocFunctionPointers.msize_ptr, &SavedMallocFunctions.msize_saved);
  RedirectCdeclFunction(free, mallocFunctionPointers.free_ptr, &SavedMallocFunctions.free_saved);
  RedirectCdeclFunction(
      _aligned_malloc,
      mallocFunctionPointers.aligned_malloc_ptr,
      &SavedMallocFunctions.aligned_malloc_saved);
  RedirectCdeclFunction(
      _aligned_offset_malloc,
      mallocFunctionPointers.aligned_offset_malloc_ptr,
      &SavedMallocFunctions.aligned_offset_malloc_saved);
  RedirectCdeclFunction(
      _aligned_realloc,
      mallocFunctionPointers.aligned_realloc_ptr,
      &SavedMallocFunctions.aligned_realloc_saved);
  RedirectCdeclFunction(
      _aligned_offset_realloc,
      mallocFunctionPointers.aligned_offset_realloc_ptr,
      &SavedMallocFunctions.aligned_offset_realloc_saved);
  RedirectCdeclFunction(
      _aligned_recalloc,
      mallocFunctionPointers.aligned_recalloc_ptr,
      &SavedMallocFunctions.aligned_recalloc_saved);
  RedirectCdeclFunction(
      _aligned_offset_recalloc,
      mallocFunctionPointers.aligned_offset_recalloc_ptr,
      &SavedMallocFunctions.aligned_offset_recalloc_saved);
  RedirectCdeclFunction(
      _aligned_msize,
      mallocFunctionPointers.aligned_msize_ptr,
      &SavedMallocFunctions.aligned_msize_saved);
  RedirectCdeclFunction(
      _aligned_free,
      mallocFunctionPointers.aligned_free_ptr,
      &SavedMallocFunctions.aligned_free_saved);

#if OVR_DEBUG_CRT_PRESENT // Within an ifdef because _malloc_dbg isn't present in a non-debug build.
  RedirectCdeclFunction(
      _malloc_dbg, mallocFunctionPointers.malloc_dbg_ptr, &SavedMallocFunctions.malloc_dbg_saved);
  RedirectCdeclFunction(
      _calloc_dbg, mallocFunctionPointers.calloc_dbg_ptr, &SavedMallocFunctions.calloc_dbg_saved);
  RedirectCdeclFunction(
      _realloc_dbg,
      mallocFunctionPointers.realloc_dbg_ptr,
      &SavedMallocFunctions.realloc_dbg_saved);
  RedirectCdeclFunction(
      _recalloc_dbg,
      mallocFunctionPointers.recalloc_dbg_ptr,
      &SavedMallocFunctions.recalloc_dbg_saved);
  RedirectCdeclFunction(
      _expand_dbg, mallocFunctionPointers.expand_dbg_ptr, &SavedMallocFunctions.expand_dbg_saved);
  RedirectCdeclFunction(
      _msize_dbg, mallocFunctionPointers.msize_dbg_ptr, &SavedMallocFunctions.msize_dbg_saved);
  RedirectCdeclFunction(
      _free_dbg, mallocFunctionPointers.free_dbg_ptr, &SavedMallocFunctions.free_dbg_saved);
  RedirectCdeclFunction(
      _aligned_malloc_dbg,
      mallocFunctionPointers.aligned_malloc_dbg_ptr,
      &SavedMallocFunctions.aligned_malloc_dbg_saved);
  RedirectCdeclFunction(
      _aligned_offset_malloc_dbg,
      mallocFunctionPointers.aligned_offset_malloc_dbg_ptr,
      &SavedMallocFunctions.aligned_offset_malloc_dbg_saved);
  RedirectCdeclFunction(
      _aligned_realloc_dbg,
      mallocFunctionPointers.aligned_realloc_dbg_ptr,
      &SavedMallocFunctions.aligned_realloc_dbg_saved);
  RedirectCdeclFunction(
      _aligned_offset_realloc_dbg,
      mallocFunctionPointers.aligned_offset_realloc_dbg_ptr,
      &SavedMallocFunctions.aligned_offset_realloc_dbg_saved);
  RedirectCdeclFunction(
      _aligned_recalloc_dbg,
      mallocFunctionPointers.aligned_recalloc_dbg_ptr,
      &SavedMallocFunctions.aligned_recalloc_dbg_saved);
  RedirectCdeclFunction(
      _aligned_offset_recalloc_dbg,
      mallocFunctionPointers.aligned_offset_recalloc_dbg_ptr,
      &SavedMallocFunctions.aligned_offset_recalloc_dbg_saved);
  RedirectCdeclFunction(
      _aligned_msize_dbg,
      mallocFunctionPointers.aligned_msize_dbg_ptr,
      &SavedMallocFunctions.aligned_msize_dbg_saved);
  RedirectCdeclFunction(
      _aligned_free_dbg,
      mallocFunctionPointers.aligned_free_dbg_ptr,
      &SavedMallocFunctions.aligned_free_dbg_saved);
#endif
#else
  // It would be better to modify __imp_malloc, which is a pointer-sized data variable (void*) that
  // indicates the addess to malloc in the CRT DLL. That way we could call the real malloc in the
  // DLL directly if needed. Actually, if we con't override __imp_malloc and try to directly
  // override malloc in the DLL then on 64 bit platforms the redirect will probably fail with our
  // current implementation because it relies on a 32 bit relative jump, which will usually be too
  // short to jump between modules. Note that overriding __imp_malloc needs to be done via modifying
  // it as a data address and not by modifying it via a call to RedirectCdeclFunction below.

  OVR_UNUSED(mallocFunctionPointers);
#endif

  // We currently assume that if the first one succeeded, all succeeded.
  if (SavedMallocFunctions.malloc_saved.Function)
    MallocFunctionsSaved = true;
  else {
    MallocFunctionsSaved = false;
    CRTMallocPreserved = false;
    return false;
  }

  MallocReplaced = true;
  return true;
}

bool InterceptCRTMalloc::RestoreCRTMalloc() {
  MallocReplaced = false;

  if (MallocFunctionsSaved) {
    MallocFunctionsSaved = false;

    RestoreCdeclFunction(&SavedMallocFunctions.malloc_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.calloc_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.realloc_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.recalloc_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.expand_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.msize_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.free_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_malloc_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_malloc_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_realloc_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_offset_realloc_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_recalloc_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_offset_recalloc_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_msize_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_free_saved);

#if OVR_DEBUG_CRT_PRESENT
    RestoreCdeclFunction(&SavedMallocFunctions.malloc_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.calloc_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.realloc_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.recalloc_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.expand_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.msize_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.free_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_malloc_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_offset_malloc_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_realloc_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_offset_realloc_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_recalloc_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_offset_recalloc_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_msize_dbg_saved);
    RestoreCdeclFunction(&SavedMallocFunctions.aligned_free_dbg_saved);
#endif
  }

  CRTMallocPreserved = false;

  return true;
}
#ifdef __clang__
#pragma GCC diagnostic pop
#endif

void* ovr_malloc(size_t size) {
  return OVR::Allocator::GetInstance()->Alloc(size, nullptr);
}

void* ovr_calloc(size_t count, size_t size) {
  return OVR::Allocator::GetInstance()->Calloc(count, size, nullptr);
}

void* ovr_realloc(void* p, size_t newSize) {
  return OVR::Allocator::GetInstance()->Realloc(p, newSize);
}

void* ovr_recalloc(void* p, size_t count, size_t size) {
  return OVR::Allocator::GetInstance()->Recalloc(p, count, size);
}

void* ovr_expand(void* /*p*/, size_t /*size*/) {
  return nullptr;
} // Always fail an expand request, which is valid to do.

size_t ovr_msize(void* p) {
  return OVR::Allocator::GetInstance()->GetAllocSize(p);
}

void ovr_free(void* p) {
  return OVR::Allocator::GetInstance()->Free(p);
}

void* ovr_aligned_malloc(size_t size, size_t align) {
  return OVR::Allocator::GetInstance()->AllocAligned(size, align, nullptr);
}

void* ovr_aligned_offset_malloc(size_t size, size_t align, size_t offset) {
  OVR_ASSERT_AND_UNUSED(offset == 0, offset);
  return ovr_aligned_malloc(size, align);
} // We don't currently support alignment offset. I've rarely seen it used.

void* ovr_aligned_realloc(void* p, size_t size, size_t align) {
  return OVR::Allocator::GetInstance()->ReallocAligned(p, size, align);
}

void* ovr_aligned_offset_realloc(void* p, size_t size, size_t align, size_t offset) {
  OVR_ASSERT_AND_UNUSED(offset == 0, offset);
  return ovr_aligned_realloc(p, size, align);
}

void* ovr_aligned_recalloc(void* p, size_t count, size_t size, size_t align) {
  return OVR::Allocator::GetInstance()->RecallocAligned(p, count, size, align);
}

void* ovr_aligned_offset_recalloc(void* p, size_t count, size_t size, size_t align, size_t offset) {
  OVR_ASSERT_AND_UNUSED(offset == 0, offset);
  return ovr_aligned_recalloc(p, count, size, align);
}

size_t ovr_aligned_msize(void* p, size_t align, size_t offset) {
  OVR_ASSERT_AND_UNUSED(offset == 0, offset);
  return OVR::Allocator::GetInstance()->GetAllocAlignedSize(p, align);
}

void ovr_aligned_free(void* p) {
  return OVR::Allocator::GetInstance()->FreeAligned(p);
}

#ifdef OVR_DEBUG_CRT_PRESENT // This is within an ifdef because the Microsoft dbg functions aren't
// present in non-debug-CRT builds.
void* ovr_malloc_dbg(size_t size, int /*blockUse*/, const char* file, int line) {
  return OVR::Allocator::GetInstance()->AllocDebug(size, nullptr, file, line);
}

void* ovr_calloc_dbg(size_t count, size_t size, int /*blockUse*/, const char* file, int line) {
  return OVR::Allocator::GetInstance()->CallocDebug(count, size, nullptr, file, line);
}

void* ovr_realloc_dbg(void* p, size_t newSize, int /*blockUse*/, const char* file, int line) {
  return OVR::Allocator::GetInstance()->ReallocDebug(p, newSize, file, line);
}

void* ovr_recalloc_dbg(
    void* p,
    size_t count,
    size_t size,
    int /*blockUse*/,
    const char* file,
    int line) {
  return OVR::Allocator::GetInstance()->RecallocDebug(p, count, size, file, line);
}

void* ovr_expand_dbg(
    void* /*p*/,
    size_t /*newSize*/,
    int /*blockUse*/,
    const char* /*file*/,
    int /*line*/) {
  return nullptr;
} // Always fail an expand request, which is valid to do.

size_t ovr_msize_dbg(void* p, int /*blockUse*/) {
  return OVR::Allocator::GetInstance()->GetAllocSize(p);
}

void ovr_free_dbg(void* p, int /*blockUse*/) {
  return OVR::Allocator::GetInstance()->Free(p);
}

void* ovr_aligned_malloc_dbg(size_t size, size_t align, const char* file, int line) {
  return OVR::Allocator::GetInstance()->AllocAlignedDebug(size, align, nullptr, file, line);
}

void* ovr_aligned_offset_malloc_dbg(
    size_t size,
    size_t align,
    size_t offset,
    const char* file,
    int line) {
  OVR_ASSERT_AND_UNUSED(offset == 0, offset);
  return ovr_aligned_malloc_dbg(size, align, file, line);
}

void* ovr_aligned_realloc_dbg(void* p, size_t size, size_t align, const char* file, int line) {
  return OVR::Allocator::GetInstance()->ReallocAlignedDebug(p, size, align, file, line);
}

void* ovr_aligned_offset_realloc_dbg(
    void* p,
    size_t size,
    size_t align,
    size_t offset,
    const char* file,
    int line) {
  OVR_ASSERT_AND_UNUSED(offset == 0, offset);
  return ovr_aligned_realloc_dbg(p, size, align, file, line);
}

void* ovr_aligned_recalloc_dbg(
    void* p,
    size_t count,
    size_t size,
    size_t align,
    const char* file,
    int line) {
  return OVR::Allocator::GetInstance()->RecallocAlignedDebug(p, count, size, align, file, line);
}

void* ovr_aligned_offset_recalloc_dbg(
    void* p,
    size_t count,
    size_t size,
    size_t align,
    size_t offset,
    const char* file,
    int line) {
  OVR_ASSERT_AND_UNUSED(offset == 0, offset);
  return ovr_aligned_recalloc_dbg(p, count, size, align, file, line);
}

size_t ovr_aligned_msize_dbg(void* p, size_t align, size_t offset) {
  OVR_ASSERT_AND_UNUSED(offset == 0, offset);
  return ovr_aligned_msize(p, align, 0);
}

void ovr_aligned_free_dbg(void* p) {
  return OVR::Allocator::GetInstance()->FreeAligned(p);
}
#endif

//-----------------------------------------------------------------------------------
// ***** AllocatorAutoCreate
//
#if defined(_MSC_VER)
// #pragma init_seg(lib) statement makes it so that this module's globals are initialized right
// after the C standard library has initialized, and are destroyed right before the C standard
// library is destroyed (after after all other app globals are destroyed). That way we can execute
// code before other global variable constructors are called and execute code after other global
// variable destructors are called. There are other init_seg directives that can be used, such as
// those below. The linker actually just goes in alphabetic order, so you could initialize before
// the CRT by using init_seg(".CRT$XCB"). Useful links:
// https://blogs.msdn.microsoft.com/ce_base/2008/06/02/dynamic-initialization-of-variables/
// http://shimpossible.blogspot.com/2013_07_01_archive.html
//#pragma init_seg(compiler) // Same as init_seg(".CRT$XCC")
//#pragma init_seg(lib)      // Same as init_seg(".CRT$XCL")
//#pragma init_seg(user)     // Same as init_seg(".CRT$XCU")
//#pragma init_seg("user_defined_segment_name")

#pragma warning(disable : 4073) // warning C4073: initializers put in library initialization area.
#pragma warning(disable : 4074) // warning C4075: initializers put in compiler initialization area.
#pragma warning( \
    disable : 4075) // warning C4075: initializers put in unrecognized initialization area.
#pragma init_seg(lib)
#endif

struct AllocatorAutoCreate {
  AllocatorAutoCreate() {
    Allocator::GetInstance(true);
  }

  ~AllocatorAutoCreate() {
    if (Allocator::GetInstance(false))
      Allocator::DestroyInstance();
  }
};

#if defined(_MSC_VER)
// Some linkers (including sometimes VC++) eliminate unreferenced globals such as the
// AllocatorAutoCreate instance below. However, we can prevent the linker from doing this via
// various techniques, such as dll-exporting the instance.
__declspec(dllexport)
#endif
    AllocatorAutoCreate allocatorAutoCreate;

//-----------------------------------------------------------------------------------
// ***** Allocator
//

Allocator* Allocator::DefaultAllocator = nullptr;
uint64_t Allocator::ReferenceHeapTimeNs = 0; // Don't set this to GetCurrentHeapTimeNs() because we
// may need to initialize it earlier than that
// construction occurs.

Allocator* Allocator::GetInstance(bool create) {
  if (!DefaultAllocator && create) {
// This is not thread-safe. Two calls could race to this point.
#if defined(_WIN32)
    // Cannot allocate memory while doing the following.
    wchar_t moduleNameW[MAX_PATH];
    char defaultAllocatorName[MAX_PATH * 6] =
        {}; // Maximum possible requirement for a UTF16 to UTF8 conversion.
    DWORD nameStrlen = GetModuleFileNameW(nullptr, moduleNameW, MAX_PATH);
    WideCharToMultiByte(
        CP_UTF8,
        0,
        moduleNameW,
        nameStrlen + 1,
        defaultAllocatorName,
        sizeof(defaultAllocatorName),
        nullptr,
        nullptr); // +1 because WideCharToMultiByte will only 0-terminate if you include the 0 as
    // part of the input.
    defaultAllocatorName[sizeof(defaultAllocatorName) - 1] = '\0';
    std::transform(
        defaultAllocatorName,
        defaultAllocatorName + strlen(defaultAllocatorName),
        defaultAllocatorName,
        [](char c) { return ((c == '/') ? '\\' : c); }); // Convert any / to \.
    const char* lastSeparator = strrchr(defaultAllocatorName, '\\');
    if (lastSeparator)
      memmove(defaultAllocatorName, lastSeparator + 1, strlen(lastSeparator + 1) + 1);
    OVR_strlcat(defaultAllocatorName, " Default Allocator", sizeof(defaultAllocatorName));
#else
    const char* defaultAllocatorName = "Default Allocator";
#endif
    DefaultAllocator = new (SysMemAlloc(sizeof(Allocator))) Allocator(defaultAllocatorName);
    DefaultAllocator->Init();
  }

  return DefaultAllocator;
}

void Allocator::DestroyInstance() {
  if (DefaultAllocator) {
    // This is not thread-safe. Two calls could race to this line.
    DefaultAllocator->Shutdown();
    DefaultAllocator->~Allocator();
    SysMemFree(DefaultAllocator, sizeof(Allocator));
    DefaultAllocator = nullptr;
  }
}

Allocator::Allocator(const char* allocatorName)
    : AllocatorName{},
      Heap(nullptr),
      DebugPageHeapEnabled(false),
      OSHeapEnabled(false),
      MallocRedirectEnabled(false),
      MallocRedirect(nullptr),
      TrackingEnabled(false),
      TraceAllocationsOnShutdown(false),
      TrackLock(),
      TrackIterator(),
      AllocationMap(),
      DelayedFreeList(),
      DelayedAlignedFreeList(),
      CurrentCounter(),
      SymbolLookupEnabled(false),
      TagMap(),
      TagMapLock() {
  SetAllocatorName(allocatorName);

  if (ReferenceHeapTimeNs == 0) // There is a thread race condition for the case that on startup two
    // threads somehow execute this line at the same time.
    ReferenceHeapTimeNs = GetCurrentHeapTimeNs();
}

void Allocator::SetAllocatorName(const char* allocatorName) {
  if (allocatorName)
    OVR::OVR_strlcpy(AllocatorName, allocatorName, OVR_ARRAY_COUNT(AllocatorName));
}

const char* Allocator::GetAllocatorName() const {
  return AllocatorName;
}

Allocator::~Allocator() {
  Allocator::Shutdown();
}

bool Allocator::Init() {
  if (!Heap) // If not already initialized...
  {
    // Potentially redirect the CRT malloc family of functions.
    if (!MallocRedirectEnabled) // If not programmatically enabled before this init call...
    {
#if OVR_REDIRECT_CRT_MALLOC
      MallocRedirectEnabled = true;
#elif defined(_WIN32)
      // "HKEY_LOCAL_MACHINE\SOFTWARE\Oculus\MallocRedirectEnabled"
      // This code uses the registry API instead of OVR::Util::SettingsManager, because this code
      // is allocator code which is special in that it needs to execute before all else is
      // initialized.
      MallocRedirectEnabled =
          OVR::Util::GetRegistryBoolW(L"Software\\Oculus", L"MallocRedirectEnabled", false);
#else
      MallocRedirectEnabled = false;
#endif

      if (Allocator::GetInstance(false) != this) // Only redirect malloc if we are the default heap.
        MallocRedirectEnabled = false;
    }

    if (MallocRedirectEnabled) {
      // We will need to enable tracking so that we can distinguish between our pointers and
      // pointers allocated via malloc before we did this redirect.
      TrackingEnabled = true;

      // Make a struct of our override function pointers.
      OVR::InterceptCRTMalloc::MallocFunctionPointers mfp;

      mfp.malloc_ptr = ovr_malloc;
      mfp.calloc_ptr = ovr_calloc;
      mfp.realloc_ptr = ovr_realloc;
      mfp.recalloc_ptr = ovr_recalloc;
      mfp.msize_ptr = ovr_msize;
      mfp.free_ptr = ovr_free;
      mfp.aligned_malloc_ptr = ovr_aligned_malloc;
      mfp.aligned_offset_malloc_ptr = ovr_aligned_offset_malloc;
      mfp.aligned_realloc_ptr = ovr_aligned_realloc;
      mfp.aligned_offset_realloc_ptr = ovr_aligned_offset_realloc;
      mfp.aligned_recalloc_ptr = ovr_aligned_recalloc;
      mfp.aligned_offset_recalloc_ptr = ovr_aligned_offset_recalloc;
      mfp.aligned_msize_ptr = ovr_aligned_msize;
      mfp.aligned_free_ptr = ovr_aligned_free;

#ifdef OVR_DEBUG_CRT_PRESENT
      mfp.malloc_dbg_ptr = ovr_malloc_dbg;
      mfp.calloc_dbg_ptr = ovr_calloc_dbg;
      mfp.realloc_dbg_ptr = ovr_realloc_dbg;
      mfp.recalloc_dbg_ptr = ovr_recalloc_dbg;
      mfp.msize_dbg_ptr = ovr_msize_dbg;
      mfp.free_dbg_ptr = ovr_free_dbg;
      mfp.aligned_malloc_dbg_ptr = ovr_aligned_malloc_dbg;
      mfp.aligned_offset_malloc_dbg_ptr = ovr_aligned_offset_malloc_dbg;
      mfp.aligned_realloc_dbg_ptr = ovr_aligned_realloc_dbg;
      mfp.aligned_offset_realloc_dbg_ptr = ovr_aligned_offset_realloc_dbg;
      mfp.aligned_recalloc_dbg_ptr = ovr_aligned_recalloc_dbg;
      mfp.aligned_offset_recalloc_dbg_ptr = ovr_aligned_offset_recalloc_dbg;
      mfp.aligned_msize_dbg_ptr = ovr_aligned_msize_dbg;
      mfp.aligned_free_dbg_ptr = ovr_aligned_free_dbg;
#endif

      MallocRedirect = new (SysMemAlloc(sizeof(OVR::InterceptCRTMalloc))) OVR::InterceptCRTMalloc;
      MallocRedirect->ReplaceCRTMalloc(mfp);
    }

    // Potentially enable the debug page heap.
    if (!DebugPageHeapEnabled) // If not programmatically enabled before this init call...
    {
// The debug page heap is restricted to 64 bit platforms due to the potential for address space
// exhaustion on 32-bit platforms.
#if defined(OVR_CPU_X86_64)
#if OVR_ALLOCATOR_DEBUG_PAGE_HEAP_ENABLED // If we should try to enable the debug heap...
      DebugPageHeapEnabled = true;
#elif defined(_MSC_VER)
      // "HKEY_LOCAL_MACHINE\SOFTWARE\Oculus\DebugPageHeapEnabled"
      // This code uses the registry API instead of OVR::Util::SettingsManager, because this code
      // is allocator code which is special in that it needs to execute before all else is
      // initialized.
      DebugPageHeapEnabled = OVR::Util::GetRegistryBoolW(
          L"Software\\Oculus", L"DebugPageHeapEnabled", DebugPageHeapEnabled);
#else
      DebugPageHeapEnabled = false;
#endif

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmicrosoft-cast"
#endif
#if defined(OVR_BUILD_DEBUG) && defined(_MSC_VER)
      if (DebugPageHeapEnabled) {
        // Make _CrtIsValidHeapPointer always return true. The VC++ concurrency library has a bug in
        // that it's calling _CrtIsValidHeapPointer, which is invalid and recommended against by
        // Microsoft themselves. We need to deal with this nevertheless. The problem is that the
        // VC++ concurrency library is calling _CrtIsValidHeapPointer on the default heap instead of
        // the current heap (DebugPageHeap). So we modify the _CrtIsValidHeapPointer implementation
        // to always return true. The primary risk with this change is that there's some code
        // somewhere that uses it for a non-diagnostic purpose. However this os
        // Oculus-debug-internal and so has no effect on any formally published software.
        DebugPageHeapEnabled = OVR::KillCdeclFunction(
            _CrtIsValidHeapPointer,
            true); // If we can successfully kill _CrtIsValidHeapPointer, enable our debug heap.
      }
#ifdef __clang__
#pragma GCC diagnostic pop
#endif
#endif
#endif
    }

    if (DebugPageHeapEnabled) {
      // We will need to enable tracking so that we can distinguish between our pointers and
      // pointers allocated via malloc before we did this redirect.
      TrackingEnabled = true;

      Heap = new (SysMemAlloc(sizeof(DebugPageHeap))) DebugPageHeap;
      Heap->Init();
    } else if (MallocRedirectEnabled) {
      // We will need to enable tracking so that we can distinguish between our pointers and
      // pointers allocated via malloc before we did this redirect.
      TrackingEnabled = true;
      OSHeapEnabled = true;

      // If we are redirecting CRT malloc then we can't use the default heap, because it used CRT
      // malloc, which would we be circular.
      Heap = new (SysMemAlloc(sizeof(OSHeap))) OSHeap;
      Heap->Init();
    } else // Else default heap (which uses malloc).
    {
      Heap = new (SysMemAlloc(sizeof(DefaultHeap))) DefaultHeap;
      Heap->Init();
    }

    // Potentially enable allocation tracking.
    if (!TrackingEnabled) // If not programmatically enabled before this init call...
    {
#if OVR_ALLOCATOR_TRACKING_ENABLED
      TrackingEnabled = true;
#else
      TrackingEnabled = IsHeapTrackingRegKeyEnabled(TrackingEnabled);
#endif
    }

    // Initialize the symbol and backtrace utility library
    SymbolLookupEnabled = SymbolLookup::Initialize();
  }

  return true;
}

void Allocator::Shutdown() {
  if (Heap) // If we were initialized...
  {
    if (TraceAllocationsOnShutdown)
      TraceTrackedAllocations(nullptr, 0);

    if (SymbolLookupEnabled)
      SymbolLookup::Shutdown();

    if (MallocRedirectEnabled) {
      MallocRedirect->RestoreCRTMalloc();
      MallocRedirect->~InterceptCRTMalloc();
      SysMemFree(MallocRedirect, sizeof(OVR::InterceptCRTMalloc));
      MallocRedirect = nullptr;
    }

#if defined(_MSC_VER)
    for (auto p : DelayedAlignedFreeList)
      _aligned_free(p);
#endif
    DelayedAlignedFreeList.clear();

    for (auto p : DelayedFreeList)
      free(p);
    DelayedFreeList.clear();

    AllocationMap.clear();
    TagMap.clear();
    CurrentCounter = 0;

    // Free the heap.
    if (Heap) {
      Heap->Shutdown();
      Heap->~Heap();
      if (DebugPageHeapEnabled)
        SysMemFree(Heap, sizeof(DebugPageHeap));
      else
        SysMemFree(Heap, sizeof(DefaultHeap));
    }
    Heap = nullptr;
  }
}

void* Allocator::Alloc(size_t size, const char* tag) {
  return AllocDebug(size, tag, nullptr, 0);
}

void* Allocator::Calloc(size_t count, size_t size, const char* tag) {
  return CallocDebug(count, size, tag, nullptr, 0);
}

void* Allocator::AllocAligned(size_t size, size_t align, const char* tag) {
  return AllocAlignedDebug(size, align, tag, nullptr, 0);
}

void* Allocator::AllocDebug(size_t size, const char* tag, const char* file, unsigned line) {
  OVR_ALLOC_BENCHMARK_START();

  void* p = Heap->Alloc(size);

  if (p) {
    TrackAlloc(p, size, tag, file, line);
  }

  OVR_ALLOC_BENCHMARK_END();

  return p;
}

void* Allocator::CallocDebug(
    size_t count,
    size_t size,
    const char* tag,
    const char* file,
    unsigned line) {
  void* p = AllocDebug(count * size, tag, file, line);

  if (p) {
    memset(p, 0, count * size);
  }

  return p;
}

void* Allocator::AllocAlignedDebug(
    size_t size,
    size_t align,
    const char* tag,
    const char* file,
    unsigned line) {
  OVR_ALLOC_BENCHMARK_START();

  void* p = Heap->AllocAligned(size, align);

  if (p) {
    TrackAlloc(p, size, tag, file, line);
  }

  OVR_ALLOC_BENCHMARK_END();

  return p;
}

size_t Allocator::GetAllocSize(const void* p) const {
  return Heap->GetAllocSize(p);
}

size_t Allocator::GetAllocAlignedSize(const void* p, size_t align) const {
  return Heap->GetAllocAlignedSize(p, align);
}

void Allocator::Free(void* p) {
  OVR_ALLOC_BENCHMARK_START();

  if (p) {
    if (UntrackAlloc(p)) // If this pointer is recognized as belonging to us...
    {
      Heap->Free(p);
    } else {
      // We don't recognize the pointer being freed. That almost always means one of two things:
      //     - We are overriding malloc/free and somebody is freeing memory that they allocated
      //     before we
      //       started overriding malloc and free.
      //     - We are not overriding malloc/free and somebody is freeing memory via operater delete
      //       which they allocated with malloc(). That's disallowed C++ but in fact it happens.
      // We have a number of options for dealing with this and try to choose the best option:
      //     - Call CRT free function.
      //       Can't do this if we are overriding CRT free and have lost access to the original CRT
      //       free function.
      //     - Call the CRT underlying _free_base function.
      //       Can't do this if the debug CRT is active, because p is not the actual pointer to
      //       free.
      //     - Call the OS underlying HeapFree function.
      //       Can't do this if the debug CRT is active, because this method results in the debug
      //       heap structures being corrupt.
      //     - Put the pointer in a list which we free later, in the case that we are overriding CRT
      //     free and have lost access to the original CRT free function.
      //       This works fairly well.
      //     - Do nothing and let the memory leak in the underlying heap.
      //       This works fairly well but results in there looking like there was a CRT memory leak.

      if (MallocRedirect) {
#if OVR_DEBUG_CRT_PRESENT || \
    (_MSC_VER <              \
     1900) // VS2013 doesn't expose _free_base, so use this delayed approach with VS2013.
        // In this case we can't just call the internal CRT _free_base function becase the debug
        // heap is active and we would need to call a dbg function. The best way for us to do that
        // would be to implement our own function which works the same as _free_dbg(). However, for
        // now we just add this pointer to the delayed free list and simply free it the normal way
        // later when we shutdown.
        DelayedFreeList.push_back(
            p); // We will call free on the pointer later when we've restored the MallocRedirect.
#else
        // In this case we can just call the internal CRT _free_base function that underlies all CRT
        // malloc functions.
        _free_base(p);
#endif
      } else {
        free(p);
      }
    }
  }

  OVR_ALLOC_BENCHMARK_END();
}

void Allocator::FreeAligned(void* p) {
  OVR_ALLOC_BENCHMARK_START();

  if (p) {
    if (UntrackAlloc(p)) {
      Heap->FreeAligned(p);
    } else {
#if defined(_MSC_VER)
      if (MallocRedirect) {
#if OVR_DEBUG_CRT_PRESENT || \
    (_MSC_VER <              \
     1900) // VS2013 doesn't expose _aligned_free_base, so use this delayed approach with VS2013.
        // In this case we can't just call the internal CRT _aligned_free_base function becase the
        // debug heap is active and we would need to call a dbg function. The best way for us to do
        // that would be to implement our own function which works the same as _aligned_free_dbg().
        // However, for now we just add this pointer to the delayed free list and simply free it the
        // normal way later when we shutdown.
        DelayedAlignedFreeList.push_back(p); // We will call _aligned_free on the pointer later when
// we've restored the MallocRedirect.
#else
        // In this case we can just call the internal CRT _free_base function that underlies all CRT
        // malloc functions.
        _aligned_free_base(p);
#endif
      } else {
        _aligned_free(p);
      }
#else
      free(p);
#endif
    }
  }

  OVR_ALLOC_BENCHMARK_END();
}

void* Allocator::Realloc(void* p, size_t newSize) {
  return ReallocDebug(p, newSize, nullptr, 0);
}

void* Allocator::Recalloc(void* p, size_t count, size_t newSize) {
  return RecallocDebug(p, count, newSize, nullptr, 0);
}

void* Allocator::ReallocAligned(void* p, size_t newSize, size_t newAlign) {
  return ReallocAlignedDebug(p, newSize, newAlign, nullptr, 0);
}

void* Allocator::RecallocAligned(void* p, size_t count, size_t newSize, size_t newAlign) {
  return RecallocAlignedDebug(p, count, newSize, newAlign, nullptr, 0);
}

void* Allocator::ReallocDebug(void* p, size_t newSize, const char* file, unsigned line) {
  OVR_ALLOC_BENCHMARK_START();

  // We have a tedious problem to solve here. If we have overridden malloc and the memory p was
  // allocated by malloc before we did the override, then p belongs to the original malloc heap. We
  // can attempt to reallocate it here with our own heap or we can reallocate it in the original
  // heap it came from. The latter is simpler.

  AllocMetadata metadata;
  bool valid = true; // realloc allows you to reallocate NULL, so set this to true by default.
  void* pNew = nullptr;

  if (p) {
    GetAllocMetadata(p, metadata);
    valid = UntrackAlloc(p); // valid will be true if p is NULL, which is what we want.
  }

  if (valid) {
    pNew = Heap->Realloc(p, newSize);

    if (pNew) {
      TrackAlloc(pNew, newSize, metadata.Tag, file, line);
    }
  } // Else p came from the CRT heap. It was likely malloc'd before we edirected malloc.
  else if (MallocRedirect) {
#if defined(_MSC_VER)
// VS2013 doesn't expose _realloc_base, so use this delayed approach with VS2013.
#if OVR_DEBUG_CRT_PRESENT || (_MSC_VER < 1900)
#if OVR_DEBUG_CRT_PRESENT
    size_t originalSize = header_from_block(p)->data_size; // We have to read internal debug data.
    pNew = crt_malloc_dbg(newSize, _NORMAL_BLOCK, file, line);
#else
    size_t originalSize = HeapSize(GetCRTHeapHandle(), 0, p);
    pNew = HeapAlloc(GetCRTHeapHandle(), 0, newSize);
#endif // OVR_DEBUG_CRT_PRESENT
    if (pNew) {
      size_t copySize = std::min(originalSize, newSize);
      memcpy(pNew, p, copySize);
      DelayedFreeList.push_back(p);
    } // Else don't free p and return nullptr.
#else
    pNew = _realloc_base(p, newSize);
#endif // OVR_DEBUG_CRT_PRESENT
#else
    OVR_UNUSED2(file, line);
    return realloc(p, newSize);
#endif // _MSC_VER
  } else {
    pNew = realloc(p, newSize);
  }

  OVR_ALLOC_BENCHMARK_END();

  return pNew;
}

void* Allocator::RecallocDebug(
    void* p,
    size_t count,
    size_t newSize,
    const char* file,
    unsigned line) {
  void* pNew = nullptr;

  if (!OVR::Alg::UnsignedMultiplyWouldOverflow(count, newSize)) {
    newSize *= count;

    size_t oldSize;
    bool valid = IsAllocTracked(p);

    if (valid) {
      oldSize = GetAllocSize(p);
    } else if (MallocRedirect) {
#if defined(_MSC_VER)
#if OVR_DEBUG_CRT_PRESENT
      oldSize = header_from_block(p)->data_size; // We have to read internal debug data.
#else
      oldSize = HeapSize(GetCRTHeapHandle(), 0, p);
#endif
#else
      oldSize = malloc_size(p);
#endif
    } else {
#if defined(_MSC_VER)
      // So which heap does this allocation come from?
      oldSize = _msize(p);
#else
      oldSize = malloc_size(p);
#endif
    }

    pNew = ReallocDebug(p, newSize, file, line);

    if (pNew && (newSize > oldSize)) {
      memset(
          reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pNew) + oldSize),
          0,
          (newSize - oldSize));
    }
  }

  return pNew;
}

void* Allocator::ReallocAlignedDebug(
    void* p,
    size_t newSize,
    size_t newAlign,
    const char* file,
    unsigned line) {
  OVR_ALLOC_BENCHMARK_START();

  AllocMetadata metadata;
  bool valid = true; // realloc allows you to reallocate NULL, so set this to true by default.
  void* pNew = nullptr;

  if (p) {
    GetAllocMetadata(p, metadata);
    valid = UntrackAlloc(p); // valid will be true if p is NULL, which is what we want.
  }

  if (valid) {
    pNew = Heap->ReallocAligned(p, newSize, newAlign);

    if (pNew) {
      TrackAlloc(pNew, newSize, metadata.Tag, file, line);
    }

    return pNew;
  } else if (MallocRedirect) // Else this must go to the CRT heap. It was likely malloc'd before we
  // existed.
  {
#if defined(_MSC_VER)
#if OVR_DEBUG_CRT_PRESENT || (_MSC_VER < 1900)
    // To do. This function in practice is not used by the CRT and so is not very important to us,
    // especially as this is debug-build only.
    OVR_FAIL_M("Allocator::ReallocAlignedDebug not implemented yet.");
#else
    pNew = _aligned_realloc_base(p, newSize, newAlign);
#endif
#else
    OVR_FAIL();
#endif
  } else {
#if defined(_MSC_VER)
    pNew = _aligned_realloc(p, newSize, newAlign);
#else
    OVR_FAIL();
#endif
  }

  OVR_ALLOC_BENCHMARK_END();

  return pNew;
}

void* Allocator::RecallocAlignedDebug(
    void* p,
    size_t count,
    size_t newSize,
    size_t newAlign,
    const char* file,
    unsigned line) {
  void* pNew = nullptr;

  if (!OVR::Alg::UnsignedMultiplyWouldOverflow(count, newSize)) {
    newSize *= count;

    size_t oldSize;
    bool valid = IsAllocTracked(p);

    if (valid) {
      oldSize = GetAllocAlignedSize(
          p, newAlign); // We really want the original alignment, not the new alignment.
    } else if (MallocRedirect) {
#if defined(_MSC_VER)
#if OVR_DEBUG_CRT_PRESENT
      oldSize = header_from_block(p)->data_size; // We have to read internal debug data.
#else
      oldSize = HeapSize(GetCRTHeapHandle(), 0, p);
#endif
#else
      oldSize = malloc_size(p);
#endif
    } else {
#if defined(_MSC_VER)
      // So which heap does this allocation come from?
      oldSize = _aligned_msize(
          p, newAlign, 0); // We really want the original alignment, not the new alignment.
#else
      oldSize = malloc_size(p);
#endif
    }

    pNew = ReallocAlignedDebug(p, newSize, newAlign, file, line);

    if (pNew && (newSize > oldSize)) {
      memset(
          reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pNew) + oldSize),
          0,
          (newSize - oldSize));
    }
  }

  return pNew;
}

uint64_t Allocator::GetCurrentHeapTimeNs() {
#if defined(_WIN32)
  LARGE_INTEGER tickCount;
  ::QueryPerformanceCounter(&tickCount);

  LARGE_INTEGER qpfFrequency;
  ::QueryPerformanceFrequency(&qpfFrequency); // To do: Move this to a single call on startup.

  return (uint64_t)((tickCount.QuadPart * UINT64_C(1000000000)) / qpfFrequency.QuadPart) -
      ReferenceHeapTimeNs;
#else
  return (uint64_t)(time(NULL) * UINT64_C(1000000000)) -
      ReferenceHeapTimeNs; // Crude value. Should improve this.
#endif
}

static AllocatorThreadId GetThreadId() {
#if defined(_WIN32)
  return ::GetCurrentThreadId();
#else
  return (AllocatorThreadId)pthread_self(); // This cast isn't strictly portable.
#endif
}

static bool ThreadIdIsValid(AllocatorThreadId threadId) {
#if defined(_WIN32)
  bool result = false;
  HANDLE h = ::OpenThread(THREAD_QUERY_LIMITED_INFORMATION, TRUE, threadId);

  if (h) {
    DWORD exitCode;
    BOOL bResult = ::GetExitCodeThread(h, &exitCode);
    result = ((bResult != FALSE) && (exitCode != STILL_ACTIVE));
    ::CloseHandle(h);
  }

  return result;
#else
  OVR_UNUSED(threadId);
  return true;
#endif
}

void Allocator::PushTag(const char* tag) {
  AllocatorThreadId threadId = GetThreadId();

  Lock::Locker locker(&TagMapLock);

  TagMap[threadId].push_back(tag);

  if (TagMap.size() > 128) // This is some number that should be more than the number of unique
    // threads we ever have.
    PurgeTagMap();
}

void Allocator::PopTag() {
  AllocatorThreadId threadId = GetThreadId();

  Lock::Locker locker(&TagMapLock);

  // We do some error checking to make sure we don't crash if this facility is mis-used.
  ThreadIdToTagVectorMap::iterator it = TagMap.find(threadId);

  if (it != TagMap.end()) {
    if (!it->second.empty())
      it->second.pop_back();
  }
}

const char* Allocator::GetTag(const char* defaultTag) {
  Lock::Locker locker(&TagMapLock);

  AllocatorThreadId threadId = GetThreadId();
  ThreadIdToTagVectorMap::const_iterator it = TagMap.find(threadId);

  if (it != TagMap.end()) {
    if (!it->second.empty())
      return it->second.back();
  }

  if (defaultTag)
    return defaultTag;

  return OVR_ALLOCATOR_UNSPECIFIED_TAG;
}

void Allocator::PurgeTagMap() {
  Lock::Locker locker(&TagMapLock);

  for (ThreadIdToTagVectorMap::iterator it = TagMap.begin(); it != TagMap.end();) {
    AllocatorThreadId threadId = it->first;

    if (!ThreadIdIsValid(threadId))
      it = TagMap.erase(it);
    else
      ++it;
  }
}

void Allocator::SetNewBlockMetadata(
    Allocator* allocator,
    AllocMetadata& amd,
    const void* alloc,
    uint64_t allocSize,
    uint64_t blockSize,
    const char* file,
    int line,
    const char* tag,
    void** backtraceArray,
    size_t backtraceArraySize) {
  amd.Alloc = alloc;
  amd.Backtrace.assign(backtraceArray, backtraceArray + backtraceArraySize);
  amd.BacktraceSymbols.clear(); // This is only set when needed.
  amd.File = file;
  amd.Line = line;
  amd.TimeNs = Allocator::GetCurrentHeapTimeNs();
  amd.Count = allocator->GetAndUpdateCounter();
  amd.AllocSize = allocSize;
  amd.BlockSize = blockSize;
  amd.Tag = tag;
  amd.ThreadId = GetThreadId();
  OVR::Thread::GetCurrentThreadName(
      amd.ThreadName, sizeof(amd.ThreadName)); // Currently works on Windows only for threads that
  // were named via our OVR thread naming API.
}

void Allocator::TrackAlloc(
    const void* p,
    size_t size,
    const char* tag,
    const char* file,
    int line) {
  if (p && TrackingEnabled) // To consider: Make TrackingEnabled an atomic.
  {
    auto AlignSizeUp =
        [](size_t value, size_t alignment) -> size_t { // To do: Have a centralized version of this.
      return ((value + (alignment - 1)) & ~(alignment - 1));
    };

    TrackedAllocMap::value_type value(p, AllocMetadata());

#if defined(_WIN64)
    void* addressArray[128];
    size_t frameCount = Symbols.GetBacktrace(addressArray, OVR_ARRAY_COUNT(addressArray), 2);
#else
    // Currently 32 bit backtrace reading is too slow. We can fix it by writing our own version
    // that reads the stack frames, but it's not a high priority since we work mostly with 64 bit.
    void* addressArray[1] = {nullptr};
    size_t frameCount = 0;
#endif

    if (!tag)
      tag = GetTag();

    SetNewBlockMetadata(
        this,
        value.second,
        p,
        size,
        AlignSizeUp(size, 8), // This is only a default value, and may be under-represented at time
        // time, until we can have that passed into this function as well.
        file,
        line,
        tag,
        addressArray,
        frameCount);

    Lock::Locker locker(&TrackLock);

    if (TrackingEnabled) // To consider: Do we really need to do this?
    {
      AllocationMap.insert(value);
    }
  }
}

bool Allocator::UntrackAlloc(const void* p) {
  if (!TrackingEnabled)
    return true; // Just assume the pointer is valid.

  if (p) {
    Lock::Locker locker(&TrackLock);

    TrackedAllocMap::iterator it = AllocationMap.find(p);

    if (it != AllocationMap.end()) {
      AllocationMap.erase(it);
      return true;
    }
  }

  return false;
}

bool Allocator::IsAllocTracked(const void* p) {
  if (!TrackingEnabled)
    return true; // Just assume the pointer is valid.

  if (p) {
    Lock::Locker locker(&TrackLock);

    return (AllocationMap.find(p) != AllocationMap.end());
  }

  return false;
}

bool Allocator::GetAllocMetadata(const void* p, AllocMetadata& metadata) {
  Lock::Locker locker(&TrackLock);

  TrackedAllocMap::iterator it = AllocationMap.find(p);

  if (it != AllocationMap.end()) {
    const TrackedAllocMap::value_type& v = *it;
    metadata = v.second;
    return true;
  }

  return false;
}

bool Allocator::EnableTracking(bool enable) {
  bool result = false;

  // We may need to deal with the case that this is called when we
  // have already started memory allocation activity. Currently disabled.
  Lock::Locker locker(&TrackLock);

  if (!Heap) // If we haven't initialized yet...
  {
    TrackingEnabled = enable;
    result = true;
  } else if (TrackingEnabled != enable) // if there is a change being requested...
  {
    // Currently we support the enabling and disabling of tracking after initialization
    // only if we aren't using the page debug heap. We may be able to work around that
    // in the future if needed. Tracking is enabled by default before initialization when
    // the page debug heap is enabled, so it's only a matter here of attempting to
    // disable tracking after it's been enabled, in practice.
    if (!DebugPageHeapEnabled) {
      TrackingEnabled = enable;

      if (!TrackingEnabled) // If we are disabling tracking...
      {
        AllocationMap.clear(); // Clear all the tracking we've done so far.
      }

      result = true;
    } // Else don't allow the change.
  } else // Else there's no change.
  {
    result = true;
  }

  return result;
}

bool Allocator::EnableDebugPageHeap(bool enable) {
  bool result = false;

  if (!Heap) // If we haven't initialized yet...
  {
    DebugPageHeapEnabled = enable;
    result = true;
  }

  return result;
}

bool Allocator::EnableMallocRedirect() {
  bool result = false;

  if (!Heap) {
    MallocRedirectEnabled = true;
    result = true;
  }

  return result;
}

bool Allocator::IsHeapTrackingRegKeyEnabled(bool defaultValue) {
#if defined(_WIN32)
  // "HKEY_LOCAL_MACHINE\SOFTWARE\Oculus\HeapTrackingEnabled", REG_DWORD of 0 or 1.
  // This code uses the registry API instead of OVR::Util::SettingsManager, because this code
  // is allocator code which is special in that it needs to execute before all else is initialized.
  return OVR::Util::GetRegistryBoolW(L"Software\\Oculus", L"HeapTrackingEnabled", defaultValue);
#else
  return defaultValue;
#endif
}

const AllocMetadata* Allocator::IterateHeapBegin() {
  TrackLock.DoLock(); // Will be unlocked in IterateHeapEnd().

  if (TrackingEnabled) {
    // We have a problem in the case that a single thread calls IterateHeapBegin twice
    // before calling IterateHeapEnd. It can be resolved the application calling IterateHeapEnd
    // twice as well, but do we want to support that usage? It's probably easier to just disallow
    // it.

    if (!AllocationMap.empty()) {
      TrackIterator = AllocationMap.begin();
      return &TrackIterator->second;
    }
  }

  return nullptr;
}

const AllocMetadata* Allocator::IterateHeapNext() {
  ++TrackIterator;

  if (TrackIterator == AllocationMap.end())
    return nullptr;

  return &TrackIterator->second;
}

void Allocator::IterateHeapEnd() {
  TrackLock.Unlock();
}

size_t Allocator::DescribeAllocation(
    const AllocMetadata* amd,
    int amdFlags,
    char* description,
    size_t descriptionCapacity,
    size_t appendedNewlineCount) {
  SysAllocatedString descriptionString;
  char buffer[2048];

  if (amdFlags & AMFAlloc) {
    snprintf(buffer, OVR_ARRAY_COUNT(buffer), "0x%p", amd->Alloc);
    descriptionString += buffer;
  }

  if (amdFlags & AMFAllocSize) {
    snprintf(buffer, OVR_ARRAY_COUNT(buffer), ", size: %llu", amd->AllocSize);
    descriptionString += buffer;
  }

  if (amdFlags & AMFBlockSize) {
    snprintf(buffer, OVR_ARRAY_COUNT(buffer), ", block size: %llu", amd->BlockSize);
    descriptionString += buffer;
  }

  if (amdFlags & AMFTime) {
    snprintf(buffer, OVR_ARRAY_COUNT(buffer), ", time: %llus", amd->TimeNs / UINT64_C(1000000000));
    descriptionString += buffer;
  }

  if (amdFlags & AMFCount) {
    snprintf(buffer, OVR_ARRAY_COUNT(buffer), ", #: %llu", amd->Count);
    descriptionString += buffer;
  }

  if (amdFlags & AMFTag) {
    snprintf(
        buffer,
        OVR_ARRAY_COUNT(buffer),
        ", tag: %s",
        amd->Tag ? amd->Tag : OVR_ALLOCATOR_UNSPECIFIED_TAG);
    descriptionString += buffer;
  }

  if (amdFlags & AMFThreadId) {
    snprintf(buffer, OVR_ARRAY_COUNT(buffer), ", tid: %lu", (unsigned long)amd->ThreadId);
    descriptionString += buffer;
  }

  if (amdFlags & AMFThreadName) {
    snprintf(buffer, OVR_ARRAY_COUNT(buffer), ", thread name: %s", amd->ThreadName);
    descriptionString += buffer;
  }

  if ((amdFlags & AMFFile) || (amdFlags & AMFLine)) {
    snprintf(buffer, OVR_ARRAY_COUNT(buffer), ", file/line: %s(%d)", amd->File, amd->Line);
    descriptionString += buffer;
  }

  if (amdFlags & (AMFBacktrace | AMFBacktraceSymbols)) {
    if (!descriptionString.empty()) // If anything was written above...
      descriptionString += "\n";

    for (size_t j = 0, jEnd = amd->Backtrace.size();
         (j < jEnd) && (descriptionString.length() < descriptionCapacity);
         ++j) {
      const bool shouldLookupSymbols =
          (SymbolLookupEnabled && ((amdFlags & AMFBacktraceSymbols) != 0));
      SymbolInfo symbolInfo;

      if (shouldLookupSymbols && Symbols.LookupSymbol((uint64_t)amd->Backtrace[j], symbolInfo) &&
          (symbolInfo.filePath[0] || symbolInfo.function[0])) {
        if (symbolInfo.filePath[0])
          snprintf(
              buffer,
              OVR_ARRAY_COUNT(buffer),
              "%2u: %s(%d): %s\n",
              (unsigned)j,
              symbolInfo.filePath,
              symbolInfo.fileLineNumber,
              symbolInfo.function[0] ? symbolInfo.function : "(unknown function)");
        else
          snprintf(
              buffer,
              OVR_ARRAY_COUNT(buffer),
              "%2u: 0x%p (unknown source file): %s\n",
              (unsigned)j,
              amd->Backtrace[j],
              symbolInfo.function);
      } else {
        snprintf(
            buffer,
            OVR_ARRAY_COUNT(buffer),
            "%2u: 0x%p (symbols unavailable)\n",
            (unsigned)j,
            amd->Backtrace[j]);
      }

      descriptionString += buffer;
    }

    descriptionString.erase(
        descriptionString.size() - 1); // Remove the last newline. We may add back below.
  }

  for (size_t i = 0; i < appendedNewlineCount; ++i)
    descriptionString += '\n';

  return OVR_strlcpy(description, descriptionString.c_str(), descriptionCapacity);
}

size_t Allocator::TraceTrackedAllocations(AllocationTraceCallback callback, uintptr_t context) {
  const bool symbolLookupWasInitialized = SymbolLookup::IsInitialized();
  const bool symbolLookupAvailable = SymbolLookup::Initialize();

  if (!symbolLookupWasInitialized) // If SymbolLookup::Initialize was the first time being
    // initialized, we need to refresh the Symbols view of modules,
    // etc.
    Symbols.Refresh();

  // If we're dumping while LibOVR is running, then we should hold the lock.
  Allocator* pAlloc = Allocator::GetInstance();

  // It's possible this is being called after the Allocator was shut down, at which
  // point we assume we are the only instance that can be executing at his time.
  Lock* lock = pAlloc ? &pAlloc->TrackLock : nullptr;
  if (lock)
    lock->DoLock();

  size_t measuredLeakCount = 0;
  size_t reportedLeakCount =
      0; // = realLeakCount minus leaks we ignore (e.g. C++ runtime concurrency leaks).
  const size_t leakReportBufferSize = 8192;
  char* leakReportBuffer = nullptr;

  // Print out detail for each leaked pointer, but filtering away some that we ignore.
  for (TrackedAllocMap::const_iterator it = AllocationMap.begin(); it != AllocationMap.end();
       ++it) {
    const TrackedAllocMap::value_type& v = *it;
    const void* p = v.first;
    const AllocMetadata& amd = v.second;

    measuredLeakCount++;

    if (!leakReportBuffer) // Lazy allocate this, as it wouldn't be needed unless we had a leak,
    // which we aim to be an unusual case.
    {
      leakReportBuffer = static_cast<char*>(SafeMMapAlloc(leakReportBufferSize));
      if (!leakReportBuffer)
        break;
    }
    leakReportBuffer[0] = '\0';

    char line[2048];
    snprintf(
        line,
        OVR_ARRAY_COUNT(line),
        "\n0x%p, size: %u, tag: %.64s\n",
        p,
        (unsigned)amd.AllocSize,
        amd.Tag ? amd.Tag : "none"); // Limit the tag length so that this can't exhaust the dest
    // buffer. We need more dest buffer space below.
    size_t currentStrlen = OVR_strlcat(leakReportBuffer, line, leakReportBufferSize);

    if (amd.Backtrace.empty()) {
      snprintf(line, OVR_ARRAY_COUNT(line), "(backtrace unavailable)\n");
      OVR_strlcat(leakReportBuffer, line, leakReportBufferSize);
    } else {
      size_t remainingCapacity = (leakReportBufferSize - currentStrlen);
      DescribeAllocation(
          &amd,
          (AMFBacktrace | AMFBacktraceSymbols),
          leakReportBuffer + currentStrlen,
          remainingCapacity,
          1);

      // There are some leaks that aren't real because they are allocated by the Standard Library at
      // runtime but aren't freed until shutdown. We don't want to report those, and so we filter
      // them out here.
      const char* ignoredPhrases[] = {"Concurrency::details" /*add any additional strings here*/};

      for (size_t j = 0; j < OVR_ARRAY_COUNT(ignoredPhrases); ++j) {
        if (strstr(leakReportBuffer, ignoredPhrases[j])) // If we should ignore this leak...
        {
          leakReportBuffer[0] = '\0';
        }
      }
    }

    if (leakReportBuffer[0]) // If we are to report this as a bonafide leak...
    {
      ++reportedLeakCount;

      // We cannot use normal logging system here because it will allocate more memory!
      if (callback)
        callback(context, leakReportBuffer);
      else
        OVR_DEBUG_TRACE(leakReportBuffer);
    }
  }

  char summaryBuffer[128];
  snprintf(
      summaryBuffer,
      OVR_ARRAY_COUNT(summaryBuffer),
      "Measured leak count: %llu, Reported leak count: %llu\n",
      (uint64_t)measuredLeakCount,
      (uint64_t)reportedLeakCount);

  if (callback)
    callback(context, summaryBuffer);
  else
    OVR_DEBUG_TRACE(summaryBuffer);

  if (leakReportBuffer) {
    SafeMMapFree(leakReportBuffer, leakReportBufferSize);
    leakReportBuffer = nullptr;
  }

  if (lock)
    lock->Unlock();

  if (symbolLookupAvailable)
    SymbolLookup::Shutdown();

  return reportedLeakCount;
}

//------------------------------------------------------------------------
// ***** HeapIterationFilterRPN
//

HeapIterationFilterRPN::HeapIterationFilterRPN()
    : AllocatorInstance(nullptr),
      Filter(nullptr),
      Instructions{},
      CurrentHeapTimeNs(Allocator::GetCurrentHeapTimeNs()) {}

bool HeapIterationFilterRPN::SetFilter(Allocator* allocator, const char* filter) {
  AllocatorInstance = allocator;
  Filter = filter;
  return Compile(filter);
}

const AllocMetadata* HeapIterationFilterRPN::IterateHeapBegin() {
  const AllocMetadata* amd = AllocatorInstance->IterateHeapBegin();

  while (amd && !Evaluate(amd))
    amd = AllocatorInstance->IterateHeapNext();

  return amd;
}

const AllocMetadata* HeapIterationFilterRPN::IterateHeapNext() {
  const AllocMetadata* amd = AllocatorInstance->IterateHeapNext();

  while (amd && !Evaluate(amd))
    amd = AllocatorInstance->IterateHeapNext();

  return amd;
}

void HeapIterationFilterRPN::IterateHeapEnd() {
  AllocatorInstance->IterateHeapEnd();
}

bool HeapIterationFilterRPN::Compile(const char* filter) {
  bool success = true; // To consider: We can report syntax errors and associated line numbers.

  static_assert(
      std::is_standard_layout<Instruction>::value, "Instructions is presumed to be a POD here.");
  memset(Instructions, 0, sizeof(Instructions));

  for (size_t instructionCount = 0; success && (instructionCount < OVR_ARRAY_COUNT(Instructions));
       ++instructionCount) // While reading each line until the end of the text...
  {
    while (isspace(*filter)) // Move past whitespace. Currently we need this only because of our
      // isOperandLine check below.
      ++filter;

    Instruction instruction{};
    char tempDataType[12], tempCompare[8], tempComparand[256], tempOperation[8], *nextChar;
    size_t i;
    bool isOperandLine = OVR_strnicmp(filter, "and", 3) &&
        OVR_strnicmp(filter, "or", 2); // To consider: Find a cleaner way to discern which of the
    // two kinds of lines this is (operand or operation).

    if ((*filter == '\r') || (*filter == '\n') || (*filter == '/')) {
      // Ignore lines that are empty or begin with /
    } else if (
        isOperandLine &&
#if defined(_MSC_VER)
        (sscanf_s(
             filter,
             "%11s %7s %255[^;]s",
             tempDataType,
             (unsigned)sizeof(tempDataType),
             tempCompare,
             (unsigned)sizeof(tempCompare),
             tempComparand,
             (unsigned)sizeof(tempComparand)) ==
         3)) // If this line looks like an operand (e.g. AllocSize > 100)...
#else
        (sscanf(filter, "%11s %7s %255[^;]s", tempDataType, tempCompare, tempComparand) ==
         3)) // If this line looks like an operand (e.g. AllocSize > 100)...
#endif
    {
      static_assert(
          sizeof(tempComparand) == 256,
          "The format string here assumes 256. Fix the format string and this assert if tempComparand changes.");

      struct OperandTypePair {
        const char* str;
        AllocMetadataFlags value;
      } operandTypeMap[] = {{"File", AMFFile},
                            {"Line", AMFLine},
                            {"Time", AMFTime},
                            {"Count", AMFCount},
                            {"Size", AMFAllocSize},
                            {"AllocSize", AMFAllocSize},
                            {"BlockSize", AMFBlockSize},
                            {"Tag", AMFTag},
                            {"ThreadId", AMFThreadId},
                            {"ThreadName", AMFThreadName}};

      // Read the operand (e.g. size)
      for (i = 0;
           i < OVR_ARRAY_COUNT(operandTypeMap) && (instruction.operand.metadataType == AMFNone);
           ++i) {
        if (OVR_stricmp(tempDataType, operandTypeMap[i].str) == 0)
          instruction.operand.metadataType = operandTypeMap[i].value;
      }
      success = success &&
          (instruction.operand.metadataType != AMFNone); // Successful if a match was found.

      // Read the compare type (e.g. >=)
      struct CompareTypePair {
        const char* str;
        Comparison value;
      } compareTypeMap[] = {
          {"==", CmpE}, {"<", CmpL}, {"<=", CmpLE}, {">", CmpG}, {">=", CmpGE}, {"has", CmpHas}};

      for (i = 0;
           i < OVR_ARRAY_COUNT(compareTypeMap) && (instruction.operand.comparison == CmpNone);
           ++i) {
        if (OVR_stricmp(tempCompare, compareTypeMap[i].str) == 0)
          instruction.operand.comparison = compareTypeMap[i].value;
      }
      success = success &&
          (instruction.operand.comparison != CmpNone); // Successful if a match was found.

      // Read the comparand (e.g. 4096)
      instruction.operand.numValue =
          strtoll(tempComparand, &nextChar, 10); // Just read it as both types here; we'll
      strcpy(instruction.operand.strValue, tempComparand); // decide at execution time which to use.

      if (instruction.operand.metadataType == AMFTime) {
        if (*nextChar ==
            's') // If the filter is specifying time in seconds instead of nanoseconds...
          instruction.operand.numValue *= 1000000000; //     convert numValue from seconds to
        //     nanoseconds (which is what we
        //     internally use).
        if (instruction.operand.numValue <
            0) // Handle the case that a negative time was passed, which
          instruction.operand.numValue +=
              CurrentHeapTimeNs; //     means to refer to time relative to current time.
      } else if (
          (instruction.operand.metadataType == AMFCount) &&
          (instruction.operand.numValue <
           0)) // Handle the case that a negative count was passed, which
        instruction.operand.numValue +=
            AllocatorInstance->GetCounter(); //     means to refer to the last N allocations.
    }
#if defined(_MSC_VER)
    else if (
        sscanf_s(filter, "%7[^;]s", tempOperation, (unsigned)sizeof(tempOperation)) ==
        1) // If this line looks like an operation (e.g. And or Or).
#else
    else if (sscanf(filter, "%7[^;]s", tempOperation) == 1) // If this line looks like an operation
// (e.g. And or Or).
#endif
    {
      if (OVR_stricmp(tempOperation, "and") == 0)
        instruction.operation = OpAnd;
      else if (OVR_stricmp(tempOperation, "or") == 0)
        instruction.operation = OpOr;
      else
        success = false;
    } else
      success = false;

    if (success) {
      if (instructionCount < OVR_ARRAY_COUNT(Instructions))
        Instructions[instructionCount] = instruction;
      else
        success = false; // Out of space.
    }

    // Move to the start of the next statement (delimited by ; or \n)
    filter = strpbrk(filter, ";\n");
    if (filter)
      filter++;
    else
      break;
  }

  return success;
}

// Evaluates an individual operand, such as (AllocSize < 32).
bool HeapIterationFilterRPN::EvaluateOperand(const Operand& operand, const AllocMetadata* amd)
    const {
  switch ((int)operand.metadataType) // Cast to int in order to avoid compiler warnings about
  // unhandled enumerants.
  {
    case AMFFile:
    case AMFTag:
    case AMFThreadName: // String-based operands
    {
      const char* p;

      switch ((int)operand.metadataType) {
        default:
        case AMFFile:
          p = amd->File;
          break;
        case AMFTag:
          p = amd->Tag;
          break;
        case AMFThreadName:
          p = amd->ThreadName;
          break;
      }

      switch ((int)operand.comparison) {
        case CmpE:
          return (OVR_stricmp(p, operand.strValue) == 0);
        case CmpHas:
          return (OVR_stristr(p, operand.strValue) != nullptr);
      }
    }

    case AMFLine:
    case AMFTime:
    case AMFCount:
    case AMFAllocSize:
    case AMFBlockSize:
    case AMFThreadId: // Integer-based operands
    {
      int64_t n;

      switch ((int)operand.metadataType) {
        default:
        case AMFLine:
          n = amd->Line;
          break;
        case AMFTime:
          n = amd->TimeNs;
          break;
        case AMFCount:
          n = amd->Count;
          break;
        case AMFAllocSize:
          n = amd->AllocSize;
          break;
        case AMFBlockSize:
          n = amd->BlockSize;
          break;
        case AMFThreadId:
          n = amd->ThreadId;
          break;
      }

      switch ((int)operand.comparison) {
        case CmpE:
          return (n == operand.numValue);
        case CmpL:
          return (n < operand.numValue);
        case CmpLE:
          return (n <= operand.numValue);
        case CmpG:
          return (n > operand.numValue);
        case CmpGE:
          return (n >= operand.numValue);
      }
    }
  }

  return false;
}

bool HeapIterationFilterRPN::Evaluate(const AllocMetadata* amd) {
  // We execute an RPN (a.k.a. postfix) stack here. Because our language here involves
  // only logical operations, our stack need be only a stack of bool.
  struct Stack {
    bool data[32];
    size_t size;

    void PopAndSet(bool value) {
      memmove(&data[1], &data[2], sizeof(data) - (2 * sizeof(bool)));
      data[0] = value;
      size -= 1;
    }
    void Push(bool value) {
      memmove(&data[1], &data[0], sizeof(data) - (1 * sizeof(bool)));
      data[0] = value;
      size += 1;
    }
  } stack{{true}, 1}; // By default the state is true. An empty instruction set evaluates as true.

  for (size_t i = 0; (i < OVR_ARRAY_COUNT(Instructions)) &&
       ((Instructions[i].operation != OpNone) || (Instructions[i].operand.comparison != CmpNone));
       ++i) {
    if (Instructions[i].operation != OpNone) // if this is an operation...
    {
      if (Instructions[i].operation == OpAnd)
        stack.PopAndSet(stack.data[0] && stack.data[1]);
      else
        stack.PopAndSet(stack.data[0] || stack.data[1]);
    } else // Else this is an operand push.
      stack.Push(EvaluateOperand(Instructions[i].operand, amd));
  }

  return stack.data[0];
}

void HeapIterationFilterRPN::TraceTrackedAllocations(
    Allocator* allocator,
    const char* filter,
    Allocator::AllocationTraceCallback callback,
    uintptr_t context) {
  OVR::HeapIterationFilterRPN hifRPN;

  hifRPN.SetFilter(allocator, filter);

  for (const OVR::AllocMetadata* amd = hifRPN.IterateHeapBegin(); amd;
       amd = hifRPN.IterateHeapNext()) {
    char description[16384];
    allocator->DescribeAllocation(amd, 0xffff, description, sizeof(description), 0);
    callback(context, description);
  }

  hifRPN.IterateHeapEnd();
}

//------------------------------------------------------------------------
// ***** DefaultHeap
//

bool DefaultHeap::Init() {
  // Nothing to do.
  return true;
}

void DefaultHeap::Shutdown() {
  // Nothing to do.
}

void* DefaultHeap::Alloc(size_t size) {
  void* p = malloc(size);

  return p;
}

void* DefaultHeap::AllocAligned(size_t size, size_t align) {
#if defined(_MSC_VER)
  void* p = _aligned_malloc(size, align);
#else
  void* p;
  int result = posix_memalign(&p, align, size);
  (void)result; // To do.
#endif

  return p;
}

size_t DefaultHeap::GetAllocSize(const void* p) const {
#if defined(_MSC_VER)
  return _msize(const_cast<void*>(p));
#else
  return malloc_size(p);
#endif
}

size_t DefaultHeap::GetAllocAlignedSize(const void* p, size_t align) const {
#if defined(_MSC_VER)
  return _aligned_msize(const_cast<void*>(p), align, 0);
#else
  OVR_UNUSED(align);
  return malloc_size(p);
#endif
}

void DefaultHeap::Free(void* p) {
  free(p);
}

void DefaultHeap::FreeAligned(void* p) {
#if defined(_MSC_VER)
  _aligned_free(p);
#else
  free(p); // No special function required.
#endif
}

void* DefaultHeap::Realloc(void* p, size_t newSize) {
  void* newP = realloc(p, newSize);

  return newP;
}

void* DefaultHeap::ReallocAligned(void* p, size_t newSize, size_t newAlign) {
#if defined(_MSC_VER)
  void* pNew = _aligned_realloc(p, newSize, newAlign);
#else
  OVR_UNUSED(newAlign);
  void* pNew = realloc(p, newSize); // We expect the implementation to know its alignment. There is
// no standard posix_memalign_realloc.
#endif

  return pNew;
}

//------------------------------------------------------------------------
// ***** OSHeap
//

OSHeap::OSHeap() : Heap(nullptr) {}

OSHeap::~OSHeap() {
  OSHeap::Shutdown();
}

bool OSHeap::Init() {
#if defined(_WIN32)
  Heap = GetProcessHeap(); // This never fails.
#endif
  return true;
}

void OSHeap::Shutdown() {
  // Do not free this heap. Its lifetime is maintained by the OS.
  Heap = nullptr;
}

void* OSHeap::Alloc(size_t size) {
#if defined(_WIN32)
  return HeapAlloc(Heap, 0, size);
#else
  return malloc(size);
#endif
}

void* OSHeap::AllocAligned(size_t size, size_t align) {
#if defined(_WIN32)
  (void)align;
  // We need to solve this if we are to support aligned memory. We'll need to allocate exta memory
  // up front, return an internal pointer, and store info to find the base pointer.
  OVR_FAIL_M("OSHeap::AllocAligned not yet supported.");
  return HeapAlloc(Heap, 0, size);
#else
  void* p;
  int result = posix_memalign(&p, align, size);
  (void)result;
  return p;
#endif
}

size_t OSHeap::GetAllocSize(const void* p) const {
#if defined(_WIN32)
  return HeapSize(Heap, 0, p);
#else
  return malloc_size(p);
#endif
}

size_t OSHeap::GetAllocAlignedSize(const void* p, size_t /*align*/) const {
#if defined(_WIN32)
  // We need to solve this if we are to support aligned memory.
  OVR_FAIL_M("OSHeap::AllocAligned not yet supported.");
  return HeapSize(Heap, 0, p);
#else
  return malloc_size(p);
#endif
}

void OSHeap::Free(void* p) {
#if defined(_WIN32)
  BOOL result = HeapFree(Heap, 0, p);
  OVR_ASSERT_AND_UNUSED(result, result);
#else
  free(p);
#endif
}

void OSHeap::FreeAligned(void* p) {
#if defined(_WIN32)
  OVR_FAIL_M("OSHeap::AllocAligned not yet supported.");
  BOOL result = HeapFree(Heap, 0, p);
  OVR_ASSERT_AND_UNUSED(result, result);
#else
  free(p);
#endif
}

void* OSHeap::Realloc(void* p, size_t newSize) {
#if defined(_WIN32)
  return HeapReAlloc(Heap, 0, p, newSize);
#else
  return realloc(p, newSize);
#endif
}

void* OSHeap::ReallocAligned(void* p, size_t newSize, size_t /*newAlign*/) {
#if defined(_WIN32)
  // We need to solve this if we are to support aligned memory.
  return HeapReAlloc(Heap, 0, p, newSize);
#else
  OVR_FAIL(); // This isn't supported properly currently.
  return realloc(p, newSize);
#endif
}

//------------------------------------------------------------------------
// ***** SafeMMapAlloc / SafeMMapFree
//

void* SafeMMapAlloc(size_t size) {
#if defined(_WIN32)
  return VirtualAlloc(
      nullptr,
      size,
      MEM_RESERVE | MEM_COMMIT,
      PAGE_READWRITE); // size is rounded up to a page. // Returned memory is 0-filled.

#elif defined(OVR_OS_MAC) || defined(OVR_OS_UNIX)
#if !defined(MAP_FAILED)
#define MAP_FAILED ((void*)-1)
#endif

  void* result = mmap(
      nullptr,
      size,
      PROT_READ | PROT_WRITE,
      MAP_PRIVATE | MAP_ANON,
      -1,
      0); // Returned memory is 0-filled.
  if (result == MAP_FAILED) // mmap returns MAP_FAILED (-1) upon failure.
    result = nullptr;
  return result;
#endif
}

void SafeMMapFree(const void* memory, size_t size) {
#if defined(_WIN32)
  OVR_UNUSED(size);
  VirtualFree(const_cast<void*>(memory), 0, MEM_RELEASE);

#elif defined(OVR_OS_MAC) || defined(OVR_OS_UNIX)
  size_t pageSize = getpagesize();
  size = (((size + (pageSize - 1)) / pageSize) * pageSize);
  munmap(const_cast<void*>(memory), size); // Must supply the size to munmap.
#endif
}

//------------------------------------------------------------------------
// ***** DebugPageHeap

static size_t AlignSizeUp(size_t value, size_t alignment) {
  return ((value + (alignment - 1)) & ~(alignment - 1));
}

static size_t AlignSizeDown(size_t value, size_t alignment) {
  return (value & ~(alignment - 1));
}

template <typename Pointer>
Pointer AlignPointerUp(Pointer p, size_t alignment) {
  return reinterpret_cast<Pointer>(
      ((reinterpret_cast<size_t>(p) + (alignment - 1)) & ~(alignment - 1)));
}

template <typename Pointer>
Pointer AlignPointerDown(Pointer p, size_t alignment) {
  return reinterpret_cast<Pointer>(reinterpret_cast<size_t>(p) & ~(alignment - 1));
}

const size_t kFreedBlockArrayMaxSizeDefault = 16384;

DebugPageHeap::DebugPageHeap()
    : FreedBlockArray(nullptr),
      FreedBlockArrayMaxSize(0),
      FreedBlockArraySize(0),
      FreedBlockArrayOldest(0),
      AllocationCount(0),
      OverrunPageEnabled(true)
#if defined(OVR_BUILD_DEBUG)
      ,
      OverrunGuardBytesEnabled(true)
#else
      ,
      OverrunGuardBytesEnabled(false)
#endif
      // PageSize(0)
      ,
      Lock() {
#if OVR_HUNT_UNTRACKED_ALLOCS
  _CrtSetAllocHook(HuntUntrackedAllocHook);
#endif

#if defined(_WIN32)
  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);
  PageSize = (size_t)systemInfo.dwPageSize;
#else
  PageSize = 4096;
#endif

  SetMaxDelayedFreeCount(kFreedBlockArrayMaxSizeDefault);
}

DebugPageHeap::~DebugPageHeap() {
  Shutdown();
}

bool DebugPageHeap::Init() {
  // Nothing to do.
  return true;
}

void DebugPageHeap::Shutdown() {
  Lock::Locker autoLock(&Lock);

  for (size_t i = 0; i < FreedBlockArraySize; i++) {
    if (FreedBlockArray[i].BlockPtr) {
      FreePageMemory(FreedBlockArray[i].BlockPtr, FreedBlockArray[i].BlockSize);
      FreedBlockArray[i].Clear();
    }
  }

  SetMaxDelayedFreeCount(0);
  FreedBlockArraySize = 0;
  FreedBlockArrayOldest = 0;
}

void DebugPageHeap::EnableOverrunDetection(
    bool enableOverrunDetection,
    bool enableOverrunGuardBytes) {
  // Assert that no allocations have been made, which is a requirement for changing these
  // properties. Otherwise future deallocations of these allocations can fail to work properly
  // because these settings have changed behind their back.
  OVR_ASSERT_M(
      AllocationCount == 0,
      "DebugPageHeap::EnableOverrunDetection called when DebugPageHeap is not in a newly initialized state.");

  OverrunPageEnabled = enableOverrunDetection;
  OverrunGuardBytesEnabled =
      (enableOverrunDetection && enableOverrunGuardBytes); // Set OverrunGuardBytesEnabled to false
  // if enableOverrunDetection is false.
}

void DebugPageHeap::SetMaxDelayedFreeCount(size_t maxDelayedFreeCount) {
  if (FreedBlockArray) {
    SafeMMapFree(FreedBlockArray, FreedBlockArrayMaxSize * sizeof(Block));
    FreedBlockArrayMaxSize = 0;
  }

  if (maxDelayedFreeCount) {
    FreedBlockArray = (Block*)SafeMMapAlloc(maxDelayedFreeCount * sizeof(Block));
    OVR_ASSERT(FreedBlockArray);

    if (FreedBlockArray) {
      FreedBlockArrayMaxSize = maxDelayedFreeCount;
#if defined(OVR_BUILD_DEBUG)
      memset(FreedBlockArray, 0, maxDelayedFreeCount * sizeof(Block));
#endif
    }
  }
}

size_t DebugPageHeap::GetMaxDelayedFreeCount() const {
  return FreedBlockArrayMaxSize;
}

void* DebugPageHeap::Alloc(size_t size) {
#if defined(_WIN32)
  return AllocAligned(size, DefaultAlignment);
#else
  return malloc(size);
#endif
}

void* DebugPageHeap::AllocAligned(size_t size, size_t align) {
#if defined(_WIN32)
  OVR_ASSERT(align <= PageSize);

  Lock::Locker autoLock(&Lock);

  if (align < DefaultAlignment)
    align = DefaultAlignment;

  // The actual needed size may be a little less than this, but it's hard to tell how the size and
  // alignments will play out.
  size_t maxRequiredSize = AlignSizeUp(size, align) + SizeStorageSize;

  if (align > SizeStorageSize) {
    // Must do: more sophisticated fitting, as maxRequiredSize is potentially too small.
    OVR_ASSERT(SizeStorageSize <= align);
  }

  size_t blockSize = AlignSizeUp(maxRequiredSize, PageSize);

  if (OverrunPageEnabled)
    blockSize += PageSize; // We add another page which will be uncommitted, so any read or write
  // with it will except.

  void* pBlockPtr;

  if ((FreedBlockArraySize == FreedBlockArrayMaxSize) &&
      FreedBlockArrayMaxSize && // If there is an old block we can recycle...
      (FreedBlockArray[FreedBlockArrayOldest].BlockSize == blockSize)) // We require it to be the
  // exact size, as there would
  // be some headaches for us
  // if it was over-sized.
  {
    pBlockPtr = EnablePageMemory(
        FreedBlockArray[FreedBlockArrayOldest].BlockPtr,
        blockSize); // Convert this memory from PAGE_NOACCESS back to PAGE_READWRITE.
    FreedBlockArray[FreedBlockArrayOldest].Clear();

    if (++FreedBlockArrayOldest == FreedBlockArrayMaxSize)
      FreedBlockArrayOldest = 0;
  } else {
    pBlockPtr = AllocCommittedPageMemory(
        blockSize); // Allocate a new block of one or more pages (via VirtualAlloc).
  }

  if (pBlockPtr) {
    void* pUserPtr = GetUserPosition(pBlockPtr, blockSize, size, align);
    size_t* pSizePos = GetSizePosition(pUserPtr);

    pSizePos[UserSizeIndex] = size;
    pSizePos[BlockSizeIndex] = blockSize;
    AllocationCount++;

    return pUserPtr;
  }

  return nullptr;
#else
  OVR_ASSERT_AND_UNUSED(align <= DefaultAlignment, align);
  return DebugPageHeap::Alloc(size);
#endif
}

size_t DebugPageHeap::GetUserSize(const void* p) {
#if defined(_WIN32)
  return GetSizePosition(p)[UserSizeIndex];
#elif defined(__APPLE__)
  return malloc_size(p);
#else
  return malloc_usable_size(const_cast<void*>(p));
#endif
}

size_t DebugPageHeap::GetBlockSize(const void* p) {
#if defined(_WIN32)
  return GetSizePosition(p)[BlockSizeIndex];
#else
  OVR_UNUSED(p);
  return 0;
#endif
}

size_t* DebugPageHeap::GetSizePosition(const void* p) {
  // No thread safety required as per our design, as we assume that anybody
  // who owns a pointer returned by Alloc cannot have another thread take it away.

  // We assume the pointer is a valid pointer allocated by this allocator.
  // We store some size values into the memory returned to the user, a few bytes before it.
  size_t value = reinterpret_cast<size_t>(p);
  size_t valuePos = (value - SizeStorageSize);
  size_t* pSize = reinterpret_cast<size_t*>(valuePos);

  return pSize;
}

void* DebugPageHeap::Realloc(void* p, size_t newSize) {
#if defined(_WIN32)
  return ReallocAligned(p, newSize, DefaultAlignment);
#else
  return realloc(p, newSize);
#endif
}

void* DebugPageHeap::ReallocAligned(void* p, size_t newSize, size_t newAlign) {
#if defined(_WIN32)
  // The ISO C99 standard states:
  //     The realloc function deallocates the old object pointed to by ptr and
  //     returns a pointer to a new object that has the size specified by size.
  //     The contents of the new object shall be the same as that of the old
  //     object prior to deallocation, up to the lesser of the new and old sizes.
  //     Any bytes in the new object beyond the size of the old object have
  //     indeterminate values.
  //
  //     If ptr is a null pointer, the realloc function behaves like the malloc
  //     function for the specified size. Otherwise, if ptr does not match a
  //     pointer earlier returned by the calloc, malloc, or realloc function,
  //     or if the space has been deallocated by a call to the free or realloc
  //     function, the behavior is undefined. If memory for the new object
  //     cannot be allocated, the old object is not deallocated and its value
  //     is unchanged.
  //
  //     The realloc function returns a pointer to the new object (which may have
  //     the same value as a pointer to the old object), or a null pointer if
  //     the new object could not be allocated.

  // A mutex lock isn't required, as the functions below will handle it internally.
  // But having it here is a little more efficient because it woudl otherwise be
  // locked and unlocked multiple times below, with possible context switches in between.
  Lock::Locker autoLock(&Lock);

  void* pReturn = nullptr;

  if (p) {
    if (newSize) {
      pReturn = AllocAligned(newSize, newAlign);

      if (pReturn) {
        size_t prevSize = GetUserSize(p);

        if (newSize > prevSize)
          newSize = prevSize;

        memcpy(pReturn, p, newSize);
        Free(p);
      } // Else fall through, leaving p's memory unmodified and returning nullptr.
    } else {
      Free(p);
    }
  } else if (newSize) {
    pReturn = AllocAligned(newSize, newAlign);
  }

  return pReturn;
#else
  OVR_ASSERT_AND_UNUSED(newAlign <= DefaultAlignment, newAlign);
  return DebugPageHeap::Realloc(p, newSize);
#endif
}

void DebugPageHeap::Free(void* p) {
#if defined(_WIN32)
  if (p) {
    // Creating a scope for the lock
    {
      Lock::Locker autoLock(&Lock);

      if (FreedBlockArrayMaxSize) // If we have a delayed free list...
      {
        // We don't free the page(s) associated with this but rather put them in the FreedBlockArray
        // in an inaccessible state for later freeing. We do this because we don't want those pages
        // to be available again in the near future, so we can detect use-after-free misakes.
        Block* pBlockNew;

        if (FreedBlockArraySize == FreedBlockArrayMaxSize) // If we have reached freed block
        // capacity... we can start purging old
        // elements from it as a circular queue.
        {
          pBlockNew = &FreedBlockArray[FreedBlockArrayOldest];

          // The oldest element in the container is FreedBlockArrayOldest.
          if (pBlockNew->BlockPtr) // Currently this should always be true.
          {
            FreePageMemory(pBlockNew->BlockPtr, pBlockNew->BlockSize);
            pBlockNew->Clear();
          }

          if (++FreedBlockArrayOldest == FreedBlockArrayMaxSize)
            FreedBlockArrayOldest = 0;
        } else // Else we are still building the container and not yet treating it a circular.
        {
          pBlockNew = &FreedBlockArray[FreedBlockArraySize++];
        }

        pBlockNew->BlockPtr = GetBlockPtr(p);
        pBlockNew->BlockSize = GetBlockSize(p);

#if defined(OVR_BUILD_DEBUG)
        if (OverrunGuardBytesEnabled) // If we have extra bytes at the end of the user's allocation
        // between it and an inaccessible guard page...
        {
          const size_t userSize = GetUserSize(p);
          const uint8_t* pUserEnd = (static_cast<uint8_t*>(p) + userSize);
          const uint8_t* pPageEnd = AlignPointerUp(pUserEnd, PageSize);

          while (pUserEnd != pPageEnd) {
            if (*pUserEnd++ != GuardFillByte) {
              OVR_FAIL();
              break;
            }
          }
        }
#endif

        DisablePageMemory(pBlockNew->BlockPtr, pBlockNew->BlockSize); // Make it so that future
        // attempts to use this memory
        // result in an exception.
      } else {
        FreePageMemory(GetBlockPtr(p), GetBlockSize(p));
      }

      AllocationCount--;
    }
  }
#else
  return free(p);
#endif
}

void DebugPageHeap::FreeAligned(void* p) {
  return Free(p);
}

// Converts a user pointer to the beginning of its page.
void* DebugPageHeap::GetBlockPtr(void* p) {
  // We store size info before p in memory, and this will, by design, be always somewhere within
  // the first page of a block of pages. So just align down to the beginning of its page.
  return AlignPointerDown(GetSizePosition(p), PageSize);
}

void* DebugPageHeap::GetUserPosition(
    void* pPageMemory,
    size_t blockSize,
    size_t userSize,
    size_t userAlignment) {
  uint8_t* pUserPosition;

  if (OverrunPageEnabled) {
    // We need to return the highest position within the page memory that fits the user size while
    // being aligned to userAlignment.
    const size_t pageEnd = reinterpret_cast<size_t>(pPageMemory) +
        (blockSize - PageSize); // pageEnd points to the beginning of the final guard page.
    const size_t userPosition = AlignSizeDown(pageEnd - userSize, userAlignment);
    pUserPosition = reinterpret_cast<uint8_t*>(userPosition);
    OVR_ASSERT((userPosition + userSize) <= pageEnd);

// If userSize is not a multiple of userAlignment then there will be (userAlignment - userSize)
// bytes of unused memory between the user allocated space and the end of the page. There is no way
// around having this. For example, a user allocation of 3 bytes with 8 byte alignment will leave 5
// unused bytes at the end of the page. We optionally fill those unused bytes with a pattern and
// upon Free verify that the pattern is undisturbed. This won't detect reads or writes in that area
// immediately as with reads or writes beyond that, but it will at least detect them at some point
// (e.g. upon Free).
#if defined(OVR_BUILD_DEBUG)
    if (OverrunGuardBytesEnabled) {
      uint8_t* const pUserEnd = (pUserPosition + userSize);
      const size_t remainingByteCount = (reinterpret_cast<uint8_t*>(pageEnd) - pUserEnd);
      if (remainingByteCount) // If there are any left-over bytes...
        memset(pUserEnd, GuardFillByte, remainingByteCount);
    }
#endif
  } else {
    // We need to return the first position in the first page after SizeStorageSize bytes which is
    // aligned to userAlignment.
    const size_t lowestPossiblePos = reinterpret_cast<size_t>(pPageMemory) + SizeStorageSize;
    const size_t userPosition = AlignSizeUp(lowestPossiblePos, userAlignment);
    pUserPosition = reinterpret_cast<uint8_t*>(userPosition);
    OVR_ASSERT((userPosition + userSize) <= (reinterpret_cast<size_t>(pPageMemory) + blockSize));
  }

  // Assert that the returned user pointer (actually the size info before it) will be within the
  // first page. This is important because it verifieds that we haven't wasted memory and because
  // our functionality for telling the start of the page block depends on it.
  OVR_ASSERT(AlignPointerDown(GetSizePosition(pUserPosition), PageSize) == pPageMemory);

  return pUserPosition;
}

void* DebugPageHeap::AllocCommittedPageMemory(size_t blockSize) {
#if defined(_WIN32)
  void* p;

  if (OverrunPageEnabled) {
    // We need to make it so that last page is MEM_RESERVE and the previous pages are MEM_COMMIT +
    // PAGE_READWRITE.
    OVR_ASSERT(blockSize > PageSize); // There should always be at least one extra page.

    // Reserve blockSize amount of pages.
    // We could possibly use PAGE_GUARD here for the last page. This differs from simply leaving it
    // reserved because the OS will generate a one-time-only gaurd page exception. We probabl don't
    // want this, as it's more useful for maintaining your own stack than for catching unintended
    // overruns.
    p = VirtualAlloc(nullptr, blockSize, MEM_RESERVE, PAGE_READWRITE);

    if (p) {
      // Commit all but the last page. Leave the last page as merely reserved so that reads from or
      // writes to it result in an immediate exception.
      p = VirtualAlloc(p, blockSize - PageSize, MEM_COMMIT, PAGE_READWRITE);
    }
  } else {
    // We need to make it so that all pages are MEM_COMMIT + PAGE_READWRITE.
    p = VirtualAlloc(nullptr, blockSize, MEM_COMMIT, PAGE_READWRITE);
  }

#if defined(OVR_BUILD_DEBUG)
  if (!p) {
    // To consider: Make a generic OVRKernel function for formatting system errors. We could move
    // the OVRError GetSysErrorCodeString from LibOVR/OVRError.h to LibOVRKernel/OVR_DebugHelp.h
    DWORD dwLastError = GetLastError();
    WCHAR osError[256];
    DWORD osErrorBufferCapacity = OVR_ARRAY_COUNT(osError);
    CHAR reportedError[384];
    DWORD length = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        (DWORD)dwLastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        osError,
        osErrorBufferCapacity,
        nullptr);

    if (length) {
      std::string errorBuff = UCSStringToUTF8String(osError, length + 1);
      snprintf(
          reportedError,
          OVR_ARRAY_COUNT(reportedError),
          "DebugPageHeap: VirtualAlloc failed with error: %s",
          errorBuff.c_str());
    } else {
      snprintf(
          reportedError,
          OVR_ARRAY_COUNT(reportedError),
          "DebugPageHeap: VirtualAlloc failed with error: %d.",
          (int)dwLastError);
    }

    // LogError("%s", reportedError); Disabled because this call turns around and allocates memory,
    // yet we may be in a broken or exhausted memory situation.
    OVR_FAIL_M(reportedError);
  }
#endif

  return p;
#else
  OVR_UNUSED2(blockSize, OverrunPageEnabled);
  return nullptr;
#endif
}

// We convert disabled page memory (see DisablePageMemory) to enabled page memory. The output is the
// same as with AllocPageMemory.
void* DebugPageHeap::EnablePageMemory(void* pPageMemory, size_t blockSize) {
#if defined(_WIN32)
  // Make sure the entire range of memory is of type PAGE_READWRITE.
  DWORD dwPrevAccess = 0;
  BOOL result = VirtualProtect(
      pPageMemory,
      OverrunPageEnabled ? (blockSize - PageSize) : blockSize,
      PAGE_READWRITE,
      &dwPrevAccess);
  OVR_ASSERT_AND_UNUSED(result, result);
#else
  OVR_UNUSED3(pPageMemory, blockSize, OverrunPageEnabled);
#endif

  return pPageMemory;
}

void DebugPageHeap::DisablePageMemory(void* pPageMemory, size_t blockSize) {
#if defined(_WIN32)
  // Disable access to the page(s). It's faster for us to change the page access than it is to
  // decommit or free the pages. However, this results in more committed physical memory usage than
  // we would need if we instead decommitted the memory.
  DWORD dwPrevAccesss = 0;
  BOOL result = VirtualProtect(
      pPageMemory,
      OverrunPageEnabled ? (blockSize - PageSize) : blockSize,
      PAGE_NOACCESS,
      &dwPrevAccesss);
  OVR_ASSERT_AND_UNUSED(result, result);
#else
  OVR_UNUSED2(pPageMemory, blockSize);
#endif
}

void DebugPageHeap::FreePageMemory(void* pPageMemory, size_t /*blockSize*/) {
#if defined(_WIN32)
  BOOL result = VirtualFree(pPageMemory, 0, MEM_RELEASE);
  OVR_ASSERT_AND_UNUSED(result, result);
#else
  OVR_UNUSED(pPageMemory);
#endif
}

//------------------------------------------------------------------------
// ***** Allocator debug commands
//
//------------------------------------------------------------------------

// AllocatorTraceDbgCmd
const char* allocatorTraceDbgCmdName = "Allocator.Trace";
const char* allocatorTraceDbgCmdUsage = "<filepath> [filter specification]";
const char* allocatorTraceDbgCmdDesc =
    "Triggers the logging of the default allocator heap, with an optional filter.";
const char* allocatorTraceDbgCmdDoc =
    "Triggers the logging of the default allocator heap, with an optional filter.\n"
    "The filter is a string which accepts a set of comparisons expressed in postfix (RPN).\n"
    "Stack traces will be symbolized if the symbol files are present; otherwise they will be missing.\n"
    "The ability to trace heaps can be enabled in release builds by setting the appropriate\n"
    "registry key before starting OVRServer:\n"
    "    HKEY_LOCAL_MACHINE\\SOFTWARE\\Oculus\\HeapTrackingEnabled, REG_DWORD of 0 or 1.\n"
    "The enabling of the debug page heap can be enabled in release builds by setting:\n"
    "    HKEY_LOCAL_MACHINE\\SOFTWARE\\Oculus\\DebugPageHeapEnabled, REG_DWORD of 0 or 1.\n"
    "Allocation tracking and tracing will have more results if malloc tracking is enabled.\n"
    "    HKEY_LOCAL_MACHINE\\SOFTWARE\\Oculus\\MallocRedirectEnabled, REG_DWORD of 0 or 1.\n"
    "Use Allocator.ReportState to tell what the current settings are.\n"
    "\n"
    "Example usage:\n"
    "    Allocator.Trace C:\\temp\\trace.txt                                   Trace the entire heap\n"
    "    Allocator.Trace C:\\temp\\trace.txt \"size > 1024\"                   Trace only allocations > 1024 bytes\n"
    "    Allocator.Trace C:\\temp\\trace.txt \"size > 1024; size < 2048; and\" Trace allocations between 1024 and 2048\n"
    "    Allocator.Trace C:\\temp\\trace.txt \"time > -10s\"                   Trace allocations done in the last 10 seconds\n"
    "    Allocator.Trace C:\\temp\\trace.txt \"time > 50000\"                  Trace allocations done in the first 50000 nanoseconds\n"
    "    Allocator.Trace C:\\temp\\trace.txt \"tag == geometry\"              Trace only allocations tagged as geometry\n"
    "    Allocator.Trace C:\\temp\\trace.txt \"ThreadName has vision\"         Trace only allocations from threads with \"vision\" in the name\n";
int AllocatorTraceDbgCmd(const std::vector<std::string>& args, std::string* output) {
  OVR_DISABLE_MSVC_WARNING(4996) // 4996: This function or variable may be unsafe.

  OVR::Allocator* allocator = OVR::Allocator::GetInstance(false);
  if (allocator) {
    std::stringstream strStream;

    if (!allocator->IsTrackingEnabled()) {
      output->append(
          "Allocator tracking is not enabled. To enable, use the Allocator.EnableTracking command or set the DWORD HKEY_LOCAL_MACHINE\\SOFTWARE\\Oculus\\HeapTrackingEnabled reg key before starting the application.");
      return -1; // Exit because even if tracking was enabled at some point earlier, all records
      // would have been cleared with it was disabled.
    }

    if (args.size() < 2) {
      output->append(
          "Filepath first argument is required but was not supplied. See example usage.");
      return -1;
    }

    std::string filePath = args[1];
    FILE* file;
    errno_t err = 0;

#if defined(_WIN32)
    err = fopen_s(&file, filePath.c_str(), "w");
#else
    file = fopen(filePath.c_str(), "w");
    err = file ? 0 : -1;
#endif

    if (err || !file) {
      strStream << "Failed to open " << filePath;
      *output = strStream.str();
      return -1;
    }

    struct Context {
      FILE* file;
      uint64_t allocationCount;
    } context = {file, 0};

    OVR::HeapIterationFilterRPN::TraceTrackedAllocations(
        allocator,
        (args.size() >= 3) ? args[2].c_str() : "",
        [](uintptr_t contextStruct, const char* text) -> void {
          Context* pContext = reinterpret_cast<Context*>(contextStruct);
          pContext->allocationCount++;
          fwrite(text, 1, strlen(text), pContext->file);
          fwrite("\n\n", 1, 2, pContext->file);
        },
        (uintptr_t)&context);
    strStream << context.allocationCount << " allocations reported to " << filePath;

    std::string str = strStream.str();
    output->append(str.data(), str.length()); // We don't directly assign string objects because
    // currently we are crossing a DLL boundary between
    // these two strings.

    fclose(file);
    return 0;
  }

  output->append("Allocator not found.");
  return -1;

  OVR_RESTORE_MSVC_WARNING()
}

// AllocatorEnableTrackingDbgCmd
const char* allocatorEnableTrackingDbgCmdName = "Allocator.EnableTracking";
const char* allocatorEnableTrackingDbgCmdUsage = "(no arguments)";
const char* allocatorEnableTrackingDbgCmdDesc = "Enables heap tracking.";
const char* allocatorEnableTrackingDbgCmdDoc =
    "Enables heap tracking, in any build. This allows for heap tracing (e.g. Allocator.Trace cmd)\n"
    "Has no effect and reports success if there is no change.\n"
    "Debug builds by default already have tracking enabled. Use Allocator.ReportState to tell.\n"
    "Currently the enabling of tracking results in the recording only allocations made after tracking is started.\n"
    "Example usage:\n"
    "    Allocator.EnableTracking\n"
    "    ... (wait for some time)\n"
    "    Allocator.Trace C:\\temp\\trace.txt\n";

int AllocatorEnableTrackingDbgCmd(const std::vector<std::string>&, std::string* output) {
  OVR::Allocator* allocator = OVR::Allocator::GetInstance(false);
  if (allocator) {
    if (allocator->EnableTracking(true))
      output->append("Allocator tracking enabled.");
    else
      output->append(
          "Allocator tracking couldn't be enabled due to an allocator settings conflict.");
  } else
    output->append("Allocator not found.");

  return (allocator ? 0 : -1);
}

// AllocatorDisableTrackingDbgCmd
const char* allocatorDisableTrackingDbgCmdName = "Allocator.DisableTracking";
const char* allocatorDisableTrackingDbgCmdUsage = "(no arguments)";
const char* allocatorDisableTrackingDbgCmdDesc = "Disables heap tracking.";
const char* allocatorDisableTrackingDbgCmdDoc =
    "Disables heap tracking, in any build.\n"
    "Has no effect and reports success if there is no change.\n"
    "Example usage:\n"
    "    Allocator.DisableTracking\n";
int AllocatorDisableTrackingDbgCmd(const std::vector<std::string>&, std::string* output) {
  OVR::Allocator* allocator = OVR::Allocator::GetInstance(false);

  if (allocator) {
    if (allocator->EnableTracking(false))
      output->append("Allocator tracking disabled.");
    else
      output->append(
          "Allocator tracking couldn't be disabled due to an allocator settings conflict.");
  } else
    output->append("Allocator not found.");

  return (allocator ? 0 : -1);
}

// AllocatorReportStateDbgCmd
const char* allocatorReportStateDbgCmdName = "Allocator.ReportState";
const char* allocatorReportStateDbgCmdUsage = "(no arguments)";
const char* allocatorReportStateDbgCmdDesc =
    "Reports the general state and settings of the global Allocator.";
const char* allocatorReportStateDbgCmdDoc =
    "Reports the general state and settings of the global Allocator.\n"
    "Example usage:\n"
    "    Allocator.ReportState\n";
int AllocatorReportStateDbgCmd(const std::vector<std::string>&, std::string* output) {
  OVR::Allocator* allocator = OVR::Allocator::GetInstance(false);

  if (allocator) {
    bool trackingEnabled = allocator->IsTrackingEnabled();
    bool debugPageHeapEnabled = allocator->IsDebugPageHeapEnabled();
    bool osHeapEnabled = allocator->IsOSHeapEnabled();
    bool mallocRedirectEnabled = allocator->IsMallocRedirectEnabled();
    bool traceOnShutdownEnabled = allocator->IsAllocationTraceOnShutdownEnabled();
    uint64_t heapTimeNs = allocator->GetCurrentHeapTimeNs();
    uint64_t heapCounter = allocator->GetCounter();
    uint64_t heapTrackedCount = 0;
    uint64_t heapTrackedVolume = 0;

    // We could report more detail that the following if desired. The following blocks the heap
    // briefly for other threads.
    for (const OVR::AllocMetadata* amd = allocator->IterateHeapBegin(); amd;
         amd = allocator->IterateHeapNext()) {
      heapTrackedCount++;
      heapTrackedVolume += amd->BlockSize;
    }
    allocator->IterateHeapEnd();
#if defined(_DLL)
    const char* crtName = "DLL CRT";
#else
    const char* crtName = "static CRT";
#endif

#if defined(_DEBUG)
    const char* buildName = "debug build";
#else
    const char* buildName = "release build";
#endif

    std::stringstream strStream;

    strStream << "Memory tracking: " << (trackingEnabled ? "enabled." : "disabled.") << std::endl;
    strStream << "Underlying heap: "
              << (debugPageHeapEnabled ? "debug page heap."
                                       : (osHeapEnabled ? "os heap." : "malloc-based heap."))
              << std::endl;
    strStream << "malloc redirection: " << (mallocRedirectEnabled ? "" : "not ") << "enabled."
              << std::endl;
    strStream << "Shutdown trace: " << (traceOnShutdownEnabled ? "" : "not ") << "enabled."
              << std::endl;
    strStream << "Heap time (ns): " << heapTimeNs << std::endl;
    strStream << "Heap counter: " << heapCounter << std::endl;
    strStream << "Heap allocated count: " << heapTrackedCount << std::endl;
    strStream << "Heap allocated volume: " << heapTrackedVolume << std::endl;
    strStream << "CRT type: " << crtName << std::endl;
    strStream << "Build type: " << buildName << std::endl;

    std::string str = strStream.str();
    output->append(str.data(), str.length()); // We don't directly assign string objects because
    // currently we are crossing a DLL boundary between
    // these two strings.
  } else
    output->append("Allocator not found.");

  return (allocator ? 0 : -1);
}

} // namespace OVR
