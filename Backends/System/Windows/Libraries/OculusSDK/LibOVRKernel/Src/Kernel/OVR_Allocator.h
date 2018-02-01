/************************************************************************************

Filename    :   OVR_Allocator.h
Content     :   Installable memory allocator
Created     :   September 19, 2012
Notes       :

Copyright   :   Copyright 2014-2016 Oculus VR, LLC All Rights reserved.

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

#ifndef OVR_Allocator_h
#define OVR_Allocator_h

#include "OVR_Types.h"
#include "OVR_Atomic.h"
#include "OVR_Std.h"
#include "stdlib.h"
#include "stdint.h"

OVR_DISABLE_ALL_MSVC_WARNINGS()
#include <string.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <exception>
#include <new>
#include <atomic>
OVR_RESTORE_ALL_MSVC_WARNINGS()
#if defined(_WIN32)
#include "OVR_Win32_IncludeWindows.h"
#endif

//-----------------------------------------------------------------------------------
// ***** Disable template-unfriendly MS VC++ warnings
//
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4503) // Pragma to prevent long name warnings in VC++
#pragma warning(disable : 4786)
#pragma warning(disable : 4345) // In MSVC 7.1, warning about placement new POD default initializer
#pragma warning(disable : 4351) // new behavior: elements of array will be default initialized
#endif

// Un-define new so that placement new works
#undef new

//------------------------------------------------------------------------
// ***** Macros to redefine class new/delete operators
//
// Types specifically declared to allow disambiguation of address in
// class member operator new. This is intended to be used with a
// macro like OVR_CHECK_DELETE(class_name, p) in the example below.
//
// Example usage:
//    class Widget
//    {
//    public:
//        Widget();
//
//        #ifdef OVR_BUILD_DEBUG
//            #define OVR_MEMORY_CHECK_DELETE(class_name, p)   \
//                do { if (p) checkInvalidDelete((class_name*)p); } while(0)
//        #else
//            #define OVR_MEMORY_CHECK_DELETE(class_name, p)
//        #endif
//
//        OVR_MEMORY_REDEFINE_NEW_IMPL(Widget, OVR_MEMORY_CHECK_DELETE)
//    };
//

#define OVR_MEMORY_REDEFINE_NEW_IMPL(class_name, check_delete) \
  void* operator new(size_t sz) {                              \
    void* p = OVR_ALLOC_DEBUG(sz, __FILE__, __LINE__);         \
    if (!p)                                                    \
      throw OVR::bad_alloc();                                  \
    return p;                                                  \
  }                                                            \
                                                               \
  void* operator new(size_t sz, const char* file, int line) {  \
    OVR_UNUSED2(file, line);                                   \
    void* p = OVR_ALLOC_DEBUG(sz, file, line);                 \
    if (!p)                                                    \
      throw OVR::bad_alloc();                                  \
    return p;                                                  \
  }                                                            \
                                                               \
  void operator delete(void* p) {                              \
    if (p) {                                                   \
      check_delete(class_name, p);                             \
      OVR_FREE(p);                                             \
    }                                                          \
  }                                                            \
                                                               \
  void operator delete(void* p, const char*, int) {            \
    if (p) {                                                   \
      check_delete(class_name, p);                             \
      OVR_FREE(p);                                             \
    }                                                          \
  }

// Used by OVR_MEMORY_REDEFINE_NEW
#define OVR_MEMORY_CHECK_DELETE_NONE(class_name, p)

// Redefine all delete/new operators in a class without custom memory initialization.
#define OVR_MEMORY_REDEFINE_NEW(class_name) \
  OVR_MEMORY_REDEFINE_NEW_IMPL(class_name, OVR_MEMORY_CHECK_DELETE_NONE)

namespace OVR {

// Our thread identifier type. May not be identical to OVR::ThreadId from OVR_Threads.h
#if defined(_WIN32)
typedef uint32_t AllocatorThreadId; // Same as DWORD
#else
typedef uintptr_t AllocatorThreadId; // Same as void*
#endif

// We subclass std::bad_alloc for the purpose of overriding the 'what' function
// to provide additional information about the exception, such as context about
// how or where the exception occurred in our code. We subclass std::bad_alloc
// instead of creating a new type because it's intended to override std::bad_alloc
// and be caught by code that uses catch(std::bad_alloc&){}. Also, the std::bad_alloc
// constructor actually attempts to allocate memory!

struct bad_alloc : public std::bad_alloc {
  bad_alloc(const char* description = "OVR::bad_alloc") OVR_NOEXCEPT;

  bad_alloc(const bad_alloc& oba) OVR_NOEXCEPT {
    OVR_strlcpy(Description, oba.Description, sizeof(Description));
  }

  bad_alloc& operator=(const bad_alloc& oba) OVR_NOEXCEPT {
    OVR_strlcpy(Description, oba.Description, sizeof(Description));
    return *this;
  }

  virtual const char* what() const OVR_NOEXCEPT {
    return Description;
  }

  char Description[256]; // Fixed size because we cannot allocate memory.
};

//-----------------------------------------------------------------------------------
// ***** Construct / Destruct

// Construct/Destruct functions are useful when new is redefined, as they can
// be called instead of placement new constructors.

template <class T>
OVR_FORCE_INLINE T* Construct(void* p) {
  return ::new (p) T();
}

template <class T>
OVR_FORCE_INLINE T* Construct(void* p, const T& source) {
  return ::new (p) T(source);
}

// Same as above, but allows for a different type of constructor.
template <class T, class S>
OVR_FORCE_INLINE T* ConstructAlt(void* p, const S& source) {
  return ::new (p) T(source);
}

template <class T, class S1, class S2>
OVR_FORCE_INLINE T* ConstructAlt(void* p, const S1& src1, const S2& src2) {
  return ::new (p) T(src1, src2);
}

// Note: These ConstructArray functions don't properly support the case of a C++ exception occurring
// midway during construction, as they don't deconstruct the successfully constructed array elements
// before returning.
template <class T>
OVR_FORCE_INLINE void ConstructArray(void* p, size_t count) {
  uint8_t* pdata = (uint8_t*)p;
  for (size_t i = 0; i < count; ++i, pdata += sizeof(T)) {
    Construct<T>(pdata);
  }
}

template <class T>
OVR_FORCE_INLINE void ConstructArray(void* p, size_t count, const T& source) {
  uint8_t* pdata = (uint8_t*)p;
  for (size_t i = 0; i < count; ++i, pdata += sizeof(T)) {
    Construct<T>(pdata, source);
  }
}

template <class T>
OVR_FORCE_INLINE void Destruct(T* pobj) {
  pobj->~T();
  OVR_UNUSED1(pobj); // Fix incorrect 'unused variable' MSVC warning.
}

template <class T>
OVR_FORCE_INLINE void DestructArray(T* pobj, size_t count) {
  for (size_t i = 0; i < count; ++i, ++pobj)
    pobj->~T();
}

//-----------------------------------------------------------------------------------
// ***** SysMemAlloc / SysMemFree
//
// System memory allocation functions. Cannot use the C runtime library. Must be able
// to execute before the C runtime library has initialized or after it has shut down.
//
void* SysMemAlloc(size_t n);
void SysMemFree(void* p, size_t n);

//-----------------------------------------------------------------------------------
// ***** StdAllocatorSysMem
//
// Defines a C++ std allocator, suitable for using with C++ containers.
// This version by design uses system memory APIs instead of C++ memory APIs.
//
// Example usage:
//     using namespace std;
//     typedef vector<int, StdAllocatorSysMem<int>> IntArray;
//     typedef basic_string<char, char_traits<char>, StdAllocatorSysMem<char>> CharString;
//     typedef list<int, StdAllocatorSysMem<int>> IntList;
//     typedef map<int, long, less<int>, StdAllocatorSysMem<int>> IntMap;
//     typedef multimap<int, long, less<int>, StdAllocatorSysMem<int>> IntMultiMap;
//     typedef set<int, less<int>, StdAllocatorSysMem<int>> IntSet;
//     typedef multiset<int, less<int>, StdAllocatorSysMem<int>> IntMultiSet;
//     typedef unordered_map<int, long, hash<int>, equal_to<int>, StdAllocatorSysMem<int>>
//     IntHashMap;
//     typedef unordered_multimap<int, long, hash<int>, equal_to<int>, StdAllocatorSysMem<int>>
//     IntHashMultiMap;
//     typedef unordered_set<int, hash<int>, equal_to<int>, StdAllocatorSysMem<int>> IntHashSet;
//     typedef unordered_multiset<int, hash<int>, equal_to<int>, StdAllocatorSysMem<int>>
//     IntHashMultiSet;
//     typedef deque<int, StdAllocatorSysMem<int>> IntDequeue;
//     typedef queue<int, deque<int, StdAllocatorSysMem<int>> > IntQueue;
//
template <class T>
class StdAllocatorSysMem {
 public:
  typedef StdAllocatorSysMem<T> this_type;
  typedef T value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef void* void_pointer;
  typedef const void* const_void_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef std::false_type propagate_on_container_copy_assignment;
  typedef std::false_type propagate_on_container_move_assignment;
  typedef std::false_type propagate_on_container_swap;

  this_type select_on_container_copy_construction() const {
    return *this;
  }

  template <class Other>
  struct rebind {
    typedef StdAllocatorSysMem<Other> other;
  };

  pointer address(reference ref) const {
    return std::addressof(ref);
  }

  const_pointer address(const_reference ref) const {
    return std::addressof(ref);
  }

  StdAllocatorSysMem() {}

  StdAllocatorSysMem(const this_type&) {}

  template <class Other>
  StdAllocatorSysMem(const StdAllocatorSysMem<Other>&) {}

  template <class Other>
  this_type& operator=(const StdAllocatorSysMem<Other>&) {
    return *this;
  }

  bool operator==(const this_type&) const {
    return true;
  }

  bool operator!=(const this_type&) const {
    return false;
  }

  void deallocate(pointer p, size_type n) const {
    if (p)
      SysMemFree(p, n);
  }

  pointer allocate(size_type n) const {
    void* pVoid = SysMemAlloc(n * sizeof(T));
    if (!pVoid)
      throw ::std::bad_alloc();
    return (pointer)pVoid;
  }

  pointer allocate(size_type n, const void*) const {
    return allocate(n);
  }

  void construct(T* p) const {
    ::new ((void*)p) T();
  }

  void construct(T* p, const T& value) const {
    ::new ((void*)p) T(value);
  }

  template <class U, class... Types>
  void construct(U* pU, Types&&... args) const {
    ::new ((void*)pU) U(std::forward<Types>(args)...);
  }

  template <class U>
  void destroy(U* pU) const {
    pU->~U();
    OVR_UNUSED(pU); // VC++ mistakenly claims it's unused unless we do this.
  }

  size_t max_size() const {
    return ((size_t)(-1) / sizeof(T));
  }
};

//------------------------------------------------------------------------
// ***** Heap
//
// Declares a minimal interface for a memory heap.
// Implementations of this Heap interface are expected to handle thread safety
// themselves.
//
class Heap {
 public:
  virtual ~Heap() {}

  virtual bool Init() = 0;
  virtual void Shutdown() = 0;

  virtual void* Alloc(size_t size) = 0;
  virtual void* AllocAligned(size_t size, size_t align) = 0;
  virtual size_t GetAllocSize(const void* p) const = 0;
  virtual size_t GetAllocAlignedSize(const void* p, size_t align) const = 0;
  virtual void Free(void* p) = 0;
  virtual void FreeAligned(void* p) = 0;
  virtual void* Realloc(void* p, size_t newSize) = 0;
  virtual void* ReallocAligned(void* p, size_t newSize, size_t newAlign) = 0;
};

class InterceptCRTMalloc;

//------------------------------------------------------------------------
// ***** SysAllocatedPointerVector, etc.
//
// We use these in our allocator functionality here.
//
typedef std::vector<void*, StdAllocatorSysMem<void*>> SysAllocatedPointerVector;
typedef std::basic_string<char, std::char_traits<char>, StdAllocatorSysMem<char>>
    SysAllocatedString;
typedef std::vector<SysAllocatedString, StdAllocatorSysMem<SysAllocatedString>>
    SysAllocatedStringVector;

//-----------------------------------------------------------------------------------
// ***** AllocMetadata
//
// Stores info about an allocation.
//
struct AllocMetadata {
  const void* Alloc; // The allocation itself.
  SysAllocatedPointerVector Backtrace; // Array of void*
  SysAllocatedStringVector BacktraceSymbols; // Array of string.
  const char* File; // __FILE__ of application allocation site.
  int Line; // __LINE__ of application allocation site.
  uint64_t TimeNs; // Allocator time in Nanoseconds. See GetCurrentHeapTimeNs.
  uint64_t Count; // Nth allocation in this heap. Ever increasing. Starts with 0, so the first
  // allocation is the 0th allocation.
  uint64_t AllocSize; // Size the user requested.
  uint64_t BlockSize; // Size that was actually needed from the underlying memory.
  const char* Tag; // Should this instead be a char array?
  AllocatorThreadId ThreadId; // ThreadId at the time of the allocation. May potentially be stale if
  // the thread has exited.
  char ThreadName[32]; // Thread name at the time of the allocation.

  AllocMetadata()
      : Alloc(nullptr),
        Backtrace(),
        File(nullptr),
        Line(0),
        TimeNs(0),
        AllocSize(0),
        BlockSize(0),
        Tag(nullptr),
        ThreadId(0) {
    ThreadName[0] = '\0';
  }
};

// This is used to identify fields from AllocMetadata. For example, when printing out
// AllocMetadata you can use these flags to specify which fields you are interested in.
enum AllocMetadataFlags {
  AMFNone = 0x0000,
  AMFAlloc = 0x0001,
  AMFBacktrace = 0x0002,
  AMFBacktraceSymbols = 0x0004,
  AMFFile = 0x0008,
  AMFLine = 0x0010,
  AMFTime = 0x0020,
  AMFCount = 0x0040,
  AMFAllocSize = 0x0080,
  AMFBlockSize = 0x0100,
  AMFTag = 0x0200,
  AMFThreadId = 0x0400,
  AMFThreadName = 0x0800
};

//-----------------------------------------------------------------------------------
// ***** Allocator
//
class Allocator {
 public:
  // Returns the pointer to the default Allocator instance.
  // This pointer is used for most of the memory allocations.
  // If the instance is not yet created, this function isn't thread safe
  // about creating it.
  static Allocator* GetInstance(bool create = true);

  // Explicitly destroys the default Allocator instance.
  static void DestroyInstance();

 public:
  Allocator(const char* allocatorName = nullptr);
  ~Allocator();

  bool Init();
  void Shutdown();

  // Allocate memory of specified size with default alignment.
  // Alloc of size==0 will allocate a tiny block & return a valid pointer;
  // this makes it suitable for new operator.
  void* Alloc(size_t size, const char* tag);

  // Allocate zero-initialized memory of specified count and size with default alignment.
  // Alloc of size==0 will allocate a tiny block & return a valid pointer;
  // this makes it suitable for new operator.
  void* Calloc(size_t count, size_t size, const char* tag);

  // Allocate memory of specified alignment.
  // Memory allocated with AllocAligned MUST be freed with FreeAligned.
  void* AllocAligned(size_t size, size_t align, const char* tag);

  // Same as Alloc, but provides an option of passing file/line data.
  void* AllocDebug(size_t size, const char* tag, const char* file, unsigned line);

  // Same as Calloc, but provides an option of passing file/line data.
  void* CallocDebug(size_t count, size_t size, const char* tag, const char* file, unsigned line);

  // Same as Alloc, but provides an option of passing file/line data.
  void*
  AllocAlignedDebug(size_t size, size_t align, const char* tag, const char* file, unsigned line);

  // Returns the size of the allocation.
  size_t GetAllocSize(const void* p) const;

  // Returns the size of the aligned allocation.
  size_t GetAllocAlignedSize(const void* p, size_t align) const;

  // Frees memory allocated by Alloc/Realloc.
  // Free of null pointer is valid and will do nothing.
  void Free(void* p);

  // Frees memory allocated with AllocAligned.
  void FreeAligned(void* p);

  // Reallocate memory block to a new size, copying data if necessary. Returns the pointer to
  // new memory block, which may be the same as original pointer. Will return 0 if reallocation
  // failed, in which case previous memory is still valid (as per the C99 Standard).
  // Realloc to decrease size will never fail.
  // Realloc of pointer == NULL is equivalent to Alloc
  // Realloc to size == 0, shrinks to the minimal size, pointer remains valid and requires Free.
  void* Realloc(void* p, size_t newSize);

  // Like realloc but also zero-initializes the newly added space.
  void* Recalloc(void* p, size_t count, size_t size);

  // Reallocates memory allocated with AllocAligned.
  void* ReallocAligned(void* p, size_t newSize, size_t newAlign);

  void* RecallocAligned(void* p, size_t count, size_t newSize, size_t newAlign);

  // Same as Realloc, but provides an option of passing file/line data.
  void* ReallocDebug(void* p, size_t newSize, const char* file, unsigned line);

  // Like ReallocDebug but also zero-initializes the newly added space.
  void* RecallocDebug(void* p, size_t count, size_t newSize, const char* file, unsigned line);

  // Same as ReallocAligned, but provides an option of passing file/line data.
  void*
  ReallocAlignedDebug(void* p, size_t newSize, size_t newAlign, const char* file, unsigned line);

  void* RecallocAlignedDebug(
      void* p,
      size_t count,
      size_t newSize,
      size_t newAlign,
      const char* file,
      unsigned line);

  // Returns the underlying Heap implementation.
  const Heap* GetHeap() const {
    return Heap;
  }

 public:
  // Names the Allocator. Useful for identifying one of multiple Allocators within a process.
  // The name is copied from the allocatorName argument.
  void SetAllocatorName(const char* allocatorName);

  const char* GetAllocatorName() const;

  // If enabled then allocation tracking is done, which enables leak reports, double-free detection,
  // etc. Must be called before the Init function.
  bool EnableTracking(bool enable);

  bool IsTrackingEnabled() const {
    return TrackingEnabled;
  }

  // If enabled then the debug page is used.
  // Must be called before the Init function.
  bool EnableDebugPageHeap(bool enable);

  bool IsDebugPageHeapEnabled() const {
    return DebugPageHeapEnabled;
  }

  bool IsOSHeapEnabled() const {
    return OSHeapEnabled;
  }

  // If enabled then a debug trace of existing allocations occurs on destruction of this Allocator.
  bool EnableAllocationTraceOnShutdown(bool enable) {
    TraceAllocationsOnShutdown = enable;
    return true;
  }

  bool IsAllocationTraceOnShutdownEnabled() const {
    return TraceAllocationsOnShutdown;
  }

  bool EnableMallocRedirect();

  bool IsMallocRedirectEnabled() const {
    return MallocRedirectEnabled;
  }

  static bool IsHeapTrackingRegKeyEnabled(bool defaultValue);

  // IterateHeapBegin succeeds only if tracking is enabled.
  // Once IterateHeapBegin is called, the heap is thread-locked until IterateHeapEnd is called.
  // If the heap has no allocations, IterateHeapBegin returns nullptr.
  // You must always call IterateHeapEnd if you call IterateHeapBegin, regardless of the return
  // value of IterateHeapBegin. For a given thread, IterataHeapBegin must be followed by
  // IterateHeapEnd before any new call to IterateHeapBegin.
  //
  // Example usage:
  //     for(const OVR::AllocMetadata* amd = allocator->IterateHeapBegin(); amd; amd =
  //     allocator->IterateHeapNext())
  //         { ... }
  //     allocator->IterateHeapEnd();
  const AllocMetadata* IterateHeapBegin();
  const AllocMetadata* IterateHeapNext();
  void IterateHeapEnd();

  // Given an AllocationMetaData, this function writes it to a string description.
  // For amdFlags, see AllocMetadataFlags.
  // Returns the required strlen of the description (like the strlcpy function, etc.)
  size_t DescribeAllocation(
      const AllocMetadata* amd,
      int amdFlags,
      char* description,
      size_t descriptionCapacity,
      size_t appendedNewlineCount);

  // Displays information about outstanding allocations, typically for the
  // purpose of reporting leaked memory on application or module shutdown.
  // This should be used instead of, for example, VC++ _CrtDumpMemoryLeaks
  // because it allows us to dump additional information about our allocations.
  // Returns the number of currently outstanding heap allocations.
  // If the callback is valid, this function iteratively calls the callback with
  // output. If the callback is nullptr then this function debug-traces the output.
  // The callback context is an arbitrary user-supplied value.
  //
  // Example usage:
  //     void AllocationTraceCallback(uintptr_t context, const char* text)
  //         { printf("%s", text); }
  //     allocator->TraceTrackedAllocations(AllocationTraceCallback, 0);
  //
  typedef void (*AllocationTraceCallback)(uintptr_t context, const char* text);
  size_t TraceTrackedAllocations(AllocationTraceCallback callback, uintptr_t context);

 public:
  // Returns the current heap time in nanoseconds. The returned time is with respect
  // to first heap startup, which in practice equates to the application start time.
  static uint64_t GetCurrentHeapTimeNs();

  // Returns the allocation counter, which indicates the historical count of calls
  // to allocation functions. This is an ever-increasing number and not the count
  // of outstanding allocations. To get the count of outstanding allocations, use the
  // heap iteration functionality.
  uint64_t GetCounter() const {
    return CurrentCounter;
  }

  uint64_t GetAndUpdateCounter() {
    return CurrentCounter++;
  } // Post-increment here is by design.

 protected:
  void SetNewBlockMetadata(
      Allocator* allocator,
      AllocMetadata& amd,
      const void* alloc,
      uint64_t allocSize,
      uint64_t blockSize,
      const char* file,
      int line,
      const char* tag,
      void** backtraceArray,
      size_t backtraceArraySize);

  // Add the allocation & the callstack to the tracking database.
  void TrackAlloc(const void* p, size_t size, const char* tag, const char* file, int line);

  // Remove the allocation from the tracking database (if tracking is enabled).
  // Returns true if p appears to be valid or if tracking is disabled.
  // Returns false for NULL and for values not found in our tracking map.
  bool UntrackAlloc(const void* p);

  // Returns true if the allocation is being tracked.
  // Returns true if p appears to be valid or if tracking is disabled.
  // Returns false for NULL and for values not found in our tracking map.
  bool IsAllocTracked(const void* p);

  // Returns a copy of the AllocMetadata.
  bool GetAllocMetadata(const void* p, AllocMetadata& metadata);

 public:
  // Tag push/pop API

  // Every PushTag must be matched by a PopTag. It's easiest to do this via the AllocatorTagScope
  // utility class.
  void PushTag(const char* tag);

  // Matches a PushTag.
  void PopTag();

  // Gets the current tag for the current thread.
  // Returns a default string if there is no current tag set for the current thread.
  const char* GetTag(const char* defaultTag = nullptr);

 protected:
  // This is called periodically to purge the map of elements that correspond to threads that no
  // longer exist.
  void PurgeTagMap();

 protected:
// Tracked allocations
#if defined(OVR_BUILD_DEBUG)
  typedef std::map<
      const void*,
      AllocMetadata,
      std::less<const void*>, // map is slower but in debug builds we can read its sorted contents
      // easier.
      StdAllocatorSysMem<std::pair<const void* const, AllocMetadata>>>
      TrackedAllocMap;
#else
  typedef std::unordered_map<
      const void*,
      AllocMetadata,
      std::hash<const void*>,
      std::equal_to<const void*>,
      StdAllocatorSysMem<std::pair<const void* const, AllocMetadata>>>
      TrackedAllocMap;
#endif

  // Per-thread tag stack
  typedef std::vector<const char*, StdAllocatorSysMem<const char*>> ConstCharVector;
#if defined(OVR_BUILD_DEBUG)
  typedef std::map<
      AllocatorThreadId,
      ConstCharVector,
      std::less<AllocatorThreadId>,
      StdAllocatorSysMem<std::pair<const AllocatorThreadId, ConstCharVector>>>
      ThreadIdToTagVectorMap;
#else
  typedef std::unordered_map<
      AllocatorThreadId,
      ConstCharVector,
      std::hash<AllocatorThreadId>,
      std::equal_to<AllocatorThreadId>,
      StdAllocatorSysMem<std::pair<const AllocatorThreadId, ConstCharVector>>>
      ThreadIdToTagVectorMap;
#endif

  char AllocatorName[64]; // The name of this allocator. Useful because we could have multiple
  // instances within a process.
  Heap* Heap; // The underlying heap we are using.
  bool DebugPageHeapEnabled; // If enabled then we use our DebugPageHeap instead of DefaultHeap or
  // OSheap.
  bool OSHeapEnabled; // If enabled then we use our OSHeap instead of DebugPageHeap or DefaultHeap.
  bool MallocRedirectEnabled; // If enabled then we redirect CRT malloc to ourself (only if we are
  // the default global allocator).
  InterceptCRTMalloc* MallocRedirect; //
  bool TrackingEnabled; //
  bool TraceAllocationsOnShutdown; // If true then we do a debug trace of allocations on our
  // shutdown.
  OVR::Lock TrackLock; // Thread-exclusive access to AllocationMap.
  TrackedAllocMap::const_iterator
      TrackIterator; // Valid only between IterateHeapBegin and IterateHeapEnd.
  TrackedAllocMap AllocationMap; //
  SysAllocatedPointerVector DelayedFreeList; // Used when we are overriding CRT malloc and need to
  // call CRT free on some pointers after we've restored
  // it.
  SysAllocatedPointerVector DelayedAlignedFreeList; // "
  std::atomic_ullong CurrentCounter; // Ever-increasing count of allocation requests.
  bool SymbolLookupEnabled; //
  ThreadIdToTagVectorMap TagMap; //
  OVR::Lock TagMapLock; // Thread-exclusive access to TagMap.
  static Allocator* DefaultAllocator; // Default instance.
  static uint64_t ReferenceHeapTimeNs; // The time that GetCurrentHeapTimeNs reports relative to. In
  // practice this is the time of application startup.

 public:
  // The following functions are deprecated and will be removed in the future. Use the member
  // functions instead.
  static void SetLeakTracking(bool enabled) {
    GetInstance()->EnableTracking(enabled);
  }

  static bool IsTrackingLeaks() {
    return GetInstance()->IsTrackingEnabled();
  }

  static int DumpMemory() {
    return (int)GetInstance()->TraceTrackedAllocations(nullptr, 0);
  }
};

// This allows for providing an intelligent filter for heap iteration.
// It provides an RPN (a.k.a. postfix) language for executing a filter of ORs and ANDs.
// We have this RPN language because implementing a string parsing system
// is currently outside the complexity scope of this module. An RPN scheme
// can be implemented in just 100 lines or code or so.
//
// An RPN language works by having a list of instructions which cause pushes
// and pops of data on a stack. In our case the stack consists only of bools.
// We want to evaluate expressions such as the following:
//     (Size > 10)
//     (Size > 10) && (Size < 32)
//     ((Size > 10) && (Size < 32)) || (Time > 100)
//     ((Size > 10) && (Size < 32)) || ((Time > 100) && (Time < 200))
//
// The user specifies the instructions via newline-delimited strings. Here are
// examples for each of the above.
// Example 1:
//    (Size > 10)
//        ->
//    "Size > 10"                   Push the result of this on the stack.
//
// Example 2:
//    (Size > 10) && (Size < 32)
//        ->
//    "Size > 10\n                  Push the result of this on the stack.
//     Size < 32\n                  Push the result of this on the stack.
//     and"                         Pop the two (AllocSize results), push the result the the AND of
//     the two.
//
// Example 3:
//    ((Size > 10) && (Size < 32)) || (Time > 100)
//        ->
//    "Size > 10\n                  Push the result of this on the stack.
//     Size < 32\n                  Push the result of this on the stack.
//     and\n                        Pop the two (AllocSize results), push the result the the AND of
//     the two. Time > 100s\n                Push the result of this on the stack. or"
//     Pop the two (AllocSize AND result and Time result), push the result of the OR of the two.
//
// Example 4:
//    ((Size > 10) && (Size < 32)) || ((Time > 100) && (Time < 200))
//        ->
//    "Size > 10\n                  Push the result of this on the stack.
//     Size < 32\n                  Push the result of this on the stack.
//     and\n                        Pop the two (AllocSize results), push the result the the AND of
//     the two. Time > 100s\n                Push the result of this on the stack. Time < 200s\n
//     Push the result of this on the stack. and\n                        Pop the two (Time
//     results), push the result the the AND of the two. or"                          Pop the two
//     (AllocSize AND result and Time AND result), push the result of the OR of the two.
//
// The operands available and their forms are:
//      Operand                  Compare         Comparand       Notes
//      ------------------------------------------------------------------------
//      File                     ==,has          <string>        case-insensitive. has means
//      substring check. Line                     ==,<,<=,>,>=    <integer> Time
//      ==,<,<=,>,>=    <integer>[s]    time in nanoseconds (or seconds if followed by s). A
//      negative time means relative to now. Count                    ==,<,<=,>,>=    <integer>
//      A negative count means to return the last nth allocation(s). AllocSize (or just Size)
//      ==,<,<=,>,>=    <integer> BlockSize                ==,<,<=,>,>=    <integer> Tag
//      ==,has          <string>        case insensitive. has means substring check. ThreadId
//      ==,<,<=,>,>=    <integer>       <,<=,>,>= are usually useless but provided for consistency
//      with other integer types. ThreadName               ==,has          <string>        case
//      insensitive. has means substring check.
//
struct HeapIterationFilterRPN {
  HeapIterationFilterRPN();

  bool SetFilter(Allocator* allocator, const char* filter);

  // This is the same as Allocator::IterateHeapBegin, except it returns only
  // values that match the filter specification.
  const AllocMetadata* IterateHeapBegin();
  const AllocMetadata* IterateHeapNext();
  void IterateHeapEnd();

  // This is a one-shot filtered tracing function.
  // Example usage:
  //     auto printfCallback = [](uintptr_t, const char* text)->void{ printf("%s\n", text); };
  //     HeapIterationFilterRPN::TraceTrackedAllocations(&allocator, "Count < 400", printfCallback,
  //     0);
  static void TraceTrackedAllocations(
      Allocator* allocator,
      const char* filter,
      Allocator::AllocationTraceCallback callback,
      uintptr_t context);

 protected:
  enum Operation { OpNone, OpAnd, OpOr };
  enum Comparison { CmpNone, CmpE, CmpL, CmpLE, CmpG, CmpGE, CmpHas };

  struct Operand {
    AllocMetadataFlags metadataType; // e.g. AMFAllocSize
    Comparison comparison; // e.g. CmpLE
    int64_t numValue; // Applies to numeric AllocMetadataFlags types.
    char strValue[256]; // Applies to string AllocMetadataFlags types.

    Operand() : metadataType(AMFNone), comparison(CmpNone), numValue(0), strValue{} {}
  };

  // An instruction is either an operation or an operand. We could use a union to represent
  // that but it would complicate our declarations here. Instead we declare both types one
  // after the other. If the operation type is none, then this entry is an operand instead
  // of an operation.
  struct Instruction {
    Operation operation;
    Operand operand;
  };

  bool Compile(const char* filter); // Returns false upon syntax error.
  bool Evaluate(const AllocMetadata* amd); // Returns true if amd matches the filter.
  bool EvaluateOperand(const Operand& operand, const AllocMetadata* amd) const;

 protected:
  Allocator* AllocatorInstance; // The Allocator we execute the filter against.
  const char* Filter; // The string-based filter gets converted into the Instructions, which can be
  // executed per alloc.
  Instruction Instructions[32]; // Array of instructions to execute. 0-terminated.
  uint64_t CurrentHeapTimeNs; // The time at the start of evaluation. Used for time comparisons.
};

//------------------------------------------------------------------------
// ***** DefaultHeap
//
// Delegates to malloc.
// This heap is created and used if no other heap is installed.
//
class DefaultHeap : public Heap {
 public:
  virtual bool Init();
  virtual void Shutdown();

  virtual void* Alloc(size_t size);
  virtual void* AllocAligned(size_t size, size_t align);
  virtual size_t GetAllocSize(const void* p) const;
  virtual size_t GetAllocAlignedSize(const void* p, size_t align) const;
  virtual void Free(void* p);
  virtual void FreeAligned(void* p);
  virtual void* Realloc(void* p, size_t newSize);
  virtual void* ReallocAligned(void* p, size_t newSize, size_t newAlign);
};

//------------------------------------------------------------------------
// ***** OSHeap
//
// Delegates to OS heap functions instead of malloc/free/new/delete.
//
class OSHeap : public Heap {
 public:
  OSHeap();
  ~OSHeap();

  virtual bool Init();
  virtual void Shutdown();

  virtual void* Alloc(size_t size);
  virtual void* AllocAligned(size_t size, size_t align);
  virtual size_t GetAllocSize(const void* p) const;
  virtual size_t GetAllocAlignedSize(const void* p, size_t align) const;
  virtual void Free(void* p);
  virtual void FreeAligned(void* p);
  virtual void* Realloc(void* p, size_t newSize);
  virtual void* ReallocAligned(void* p, size_t newSize, size_t newAlign);

 protected:
#if defined(_WIN32)
  HANDLE Heap; // Windows heap handle.
#else
  void* Heap;
#endif
};

//------------------------------------------------------------------------
// ***** DebugPageHeap
//
// Implements a page-protected heap:
//   Detects use-after-free and memory overrun bugs immediately at the time of usage via an
//   exception. Can detect a memory read or write beyond the valid memory immediately at the
//       time of usage via an exception (if EnableOverrunDetection is enabled).
//       This doesn't replace valgrind but implements a subset of its functionality
//       in a way that performs well enough to avoid interfering with app execution.
//   The point of this is that immediately detects these two classes of errors while
//       being much faster than external tools such as valgrind, etc. This is at a cost of
//       as much as a page of extra bytes per allocation (two if EnableOverrunDetection is enabled).
//   On Windows the Alloc and Free functions average about 12000 cycles each. This isn't small but
//       it should be low enough for many testing circumstances with apps that are prudent with
//       memory allocation volume.
//   The amount of system memory needed for this isn't as high as one might initially guess, as it
//       takes hundreds of thousands of memory allocations in order to make a dent in the gigabytes
//       of memory most computers have.
//
//
// Technical design for the Windows platform:
//   Every Alloc is satisfied via a VirtualAlloc return of a memory block of one or more pages;
//       the minimum needed to satisy the user's size and alignment requirements.
//   Upon Free the memory block (which is one or more pages) is not passed to VirtualFree but rather
//       is converted to PAGE_NOACCESS and put into a delayed free list (FreedBlockArray) to be
//       passed to VirtualFree later. The result of this is that any further attempts to read or
//       write the memory will result in an exception.
//   The delayed-free list increases each time Free is called until it reached maximum capacity,
//       at which point the oldest memory block in the list is passed to VirtualFree and its
//       entry in the list is filled with this newly Freed (PAGE_NOACCESS) memory block.
//   Once the delayed-free list reaches maximum capacity it thus acts as a ring buffer of blocks.
//       The maximum size of this list is currently determined at compile time as a constant.
//   The EnableOverrunDetection is an additional feature which allows reads or writes beyond valid
//       memory to be detected as they occur. This is implemented by adding an allocating an
//       additional page of memory at the end of the usual pages and leaving it uncommitted
//       (MEM_RESERVE). When this option is used, we return a pointer to the user that's at the end
//       of the valid memory block as opposed to at the beginning. This is so that the space right
//       after the user space is invalid. If there are some odd bytes remaining between the end of
//       the user's space and the page (due to alignment requirements), we optionally fill these
//       with guard bytes. We do not currently support memory underrun detection, which could be
//       implemented via an extra un-accessible page before the user page(s). In practice this is
//       rarely needed.
//   Currently the choice to use EnableOverrunDetection must be done before any calls to Alloc, etc.
//       as the logic is simpler and faster if we don't have to dynamically handle either case at
//       runtime.
//   We store within the memory block the size of the block and the size of the original user Alloc
//       request. This is done as two size_t values written before the memory returned to the user.
//       Thus the pointer returned to the user will never be at the very beginning of the memory
//       block, because there will be two size_t's before it.
//   This class itself allocates no memory, as that could interfere with its ability to supply
//       memory, especially if the global malloc and new functions are replaced with this class.
//       We could in fact support this class allocating memory as long as it used a system allocator
//       and not malloc, new, etc.
//   As of this writing we don't do debug fill patterns in the returned memory, because we mostly
//       don't need it because memory exceptions take the place of unexpected fill value validation.
//       However, there is some value in doing a small debug fill of the last few bytes after the
//       user's bytes but before the next page, which will happen for odd sizes passed to Alloc.
//
// Technical design for Mac and Linux platforms:
//   Apple's XCode malloc functionality includes something called MallocGuardEdges which is similar
//       to DebugPageHeap, though it protects only larger sized allocations and not smaller ones.
//   Our approach for this on Mac and Linux is to use mmap and mprotect in a way similar to
//   VirtualAlloc and
//       VirtualProtect. Unix doesn't have the concept of Windows MEM_RESERVE vs. MEM_COMMIT, but we
//       can simulate MEM_RESERVE by having an extra page that's PROT_NONE instead of MEM_RESERVE.
//       Since Unix platforms don't commit pages pages to physical memory until they are first
//       accessed, this extra page will in practice act similar to Windows MEM_RESERVE at runtime.
//
// Allocation inteface:
//   Alloc sizes can be any size_t >= 0.
//   An alloc size of 0 returns a non-nullptr.
//   Alloc functions may fail (usually due to insufficent memory), in which case they return
//   nullptr. All returned allocations are aligned on a power-of-two boundary of at least
//   DebugPageHeap::DefaultAlignment. AllocAligned supports any alignment power-of-two value from 1
//   to 256. Other values result in undefined behavior. AllocAligned may return a pointer that's
//   aligned greater than the requested alignment. Realloc acts as per the C99 Standard realloc.
//   Free requires the supplied pointer to be a valid pointer returned by this allocator's Alloc
//   functions, else the behavior is undefined. You may not Free a pointer a second time, else the
//   behavior is undefined. Free otherwise always succeeds. Allocations made with AllocAligned or
//   ReallocAligned must be Freed via FreeAligned, as per the base class requirement.
//

class DebugPageHeap : public Heap {
 public:
  DebugPageHeap();
  virtual ~DebugPageHeap();

  bool Init();
  void Shutdown();

  void SetMaxDelayedFreeCount(size_t delayedFreeCount); // Sets how many freed blocks we should save
  // before purging the oldest of them.
  size_t GetMaxDelayedFreeCount() const; // Returns the max number of delayed free allocations
  // before the oldest ones are purged (finally freed).
  void EnableOverrunDetection(
      bool enableOverrunDetection,
      bool enableOverrunGuardBytes); // enableOverrunDetection is by default.
  // enableOverrunGuardBytes is enabled by default in debug
  // builds.

  void* Alloc(size_t size);
  void* AllocAligned(size_t size, size_t align);
  size_t GetAllocSize(const void* p) const {
    return GetUserSize(p);
  }
  size_t GetAllocAlignedSize(const void* p, size_t /*align*/) const {
    return GetUserSize(p);
  }
  void* Realloc(void* p, size_t newSize);
  void* ReallocAligned(void* p, size_t newSize, size_t newAlign);
  void Free(void* p);
  void FreeAligned(void* p);
  size_t GetPageSize() const {
    return PageSize;
  }

 protected:
  struct Block {
    void* BlockPtr; // The pointer to the first page of the contiguous set of pages that make up
    // this block.
    size_t BlockSize; // (page size) * (page count). Will be >= (SizeStorageSize + UserSize).

    void Clear() {
      BlockPtr = nullptr;
      BlockSize = 0;
    }
  };

  Block* FreedBlockArray; // Currently a very simple array-like container that acts as a ring buffer
  // of delay-freed (but inaccessible) blocks.
  size_t FreedBlockArrayMaxSize; // The max number of Freed blocks to put into FreedBlockArray
  // before they start getting purged. Must be <=
  // kFreedBlockArrayCapacity.
  size_t FreedBlockArraySize; // The amount of valid elements within FreedBlockArray. Increases as
  // elements are added until it reaches kFreedBlockArrayCapacity. Then
  // stays that way until Shutdown.
  size_t FreedBlockArrayOldest; // The oldest entry in the FreedBlockArray ring buffer.
  size_t AllocationCount; // Number of currently live Allocations. Incremented by successful calls
  // to Alloc (etc.)  Decremented by successful calss to Free.
  bool OverrunPageEnabled; // If true then we implement memory overrun detection, at the cost of an
  // extra page per user allocation.
  bool OverrunGuardBytesEnabled; // If true then any remaining bytes between the end of the user's
  // allocation and the end of the page are filled with guard bytes
  // and verified upon Free. Valid only if OverrunPageEnabled is
  // true.
  size_t PageSize; // The current default platform memory page size (e.g. 4096). We allocated blocks
  // in multiples of pages.
  OVR::Lock Lock; // Mutex which allows an instance of this class to be used by multiple threads
  // simultaneously.

 public:
#if defined(_WIN64) || defined(_M_IA64) || defined(__LP64__) || defined(__LP64__) || \
    defined(__arch64__) || defined(__APPLE__)
  static const size_t DefaultAlignment = 16; // 64 bit platforms and all Apple platforms.
#else
  static const size_t DefaultAlignment = 8; // 32 bit platforms. We want DefaultAlignment as low as
// possible because that means less unused bytes between
// a user allocation and the end of the page.
#endif
#if defined(_WIN32)
  static const size_t MaxAlignment = 2048; // Half a page size.
#else
  static const size_t MaxAlignment = DefaultAlignment; // Currently a low limit because we don't
// have full page allocator support yet.
#endif

 protected:
  static const size_t SizeStorageSize = DefaultAlignment; // Where the user size and block size is
  // stored. Needs to be at least 2 *
  // sizeof(size_t).
  static const size_t UserSizeIndex =
      0; // We store block sizes within the memory itself, and this serves to identify it.
  static const size_t BlockSizeIndex = 1;
  static const uint8_t GuardFillByte = 0xfd; // Same value VC++ uses for heap guard bytes.

  static size_t GetUserSize(
      const void* p); // Returns the size that the user requested in Alloc, etc.
  static size_t GetBlockSize(const void* p); // Returns the actual number of bytes in the returned
  // block. Will be a multiple of PageSize.
  static size_t* GetSizePosition(const void* p); // We store the user and block size as two size_t
  // values within the returned memory to the user,
  // before the user pointer. This gets that
  // location.

  void* GetBlockPtr(void* p);
  void* GetUserPosition(void* pPageMemory, size_t blockSize, size_t userSize, size_t userAlignment);
  void* AllocCommittedPageMemory(size_t blockSize);
  void* EnablePageMemory(void* pPageMemory, size_t blockSize);
  void DisablePageMemory(void* pPageMemory, size_t blockSize);
  void FreePageMemory(void* pPageMemory, size_t blockSize);
};

///------------------------------------------------------------------------
/// ***** AllocatorTagScope
///
/// Implements automated setting of a current tag for an allocator.
/// This allows for easier tagging of memory by subsystem usage.
///
/// Example usage:
///    void Geometry::GenerateGeometry()
///    {
///        AllocatorTagScope tagScope("geometry"); // Registers a tag with the default allocator.
///        Will unregister it at function exit.
///
///        Indices.push_back(1234);         // The allocation that results from each of these
///        MemberPtr = OVR_ALLOC(1234);     // calls will be tagged with "geometry".
///    }
///
class AllocatorTagScope {
 public:
  AllocatorTagScope(const char* tag = nullptr, Allocator* allocator = nullptr) {
    Tag = tag;

    if (!allocator)
      allocator = Allocator::GetInstance(false);

    Allocator = allocator;

    if (Allocator)
      Allocator->PushTag(tag);
  }

  ~AllocatorTagScope() {
    if (Allocator)
      Allocator->PopTag();
  }

  const char* GetTag() const {
    return Tag;
  }

 protected:
  const char* Tag;
  Allocator* Allocator;
};

// AllocatorTraceDbgCmd
//
// This is a debug command that lets you dump a heap trace into a file, with a given filter
// specification.
//
extern const char* allocatorTraceDbgCmdName;
extern const char* allocatorTraceDbgCmdUsage;
extern const char* allocatorTraceDbgCmdDesc;
extern const char* allocatorTraceDbgCmdDoc;
extern int AllocatorTraceDbgCmd(const std::vector<std::string>& args, std::string* output);

// AllocatorEnableTrackingDbgCmd
//
// This is a debug command that lets you enable tracing in the Allocator.
//
extern const char* allocatorEnableTrackingDbgCmdName;
extern const char* allocatorEnableTrackingDbgCmdUsage;
extern const char* allocatorEnableTrackingDbgCmdDesc;
extern const char* allocatorEnableTrackingDbgCmdDoc;
extern int AllocatorEnableTrackingDbgCmd(const std::vector<std::string>&, std::string* output);

// AllocatorDisableTrackingDbgCmd
//
// This is a debug command that lets you disable tracing in the Allocator.
//
extern const char* allocatorDisableTrackingDbgCmdName;
extern const char* allocatorDisableTrackingDbgCmdUsage;
extern const char* allocatorDisableTrackingDbgCmdDesc;
extern const char* allocatorDisableTrackingDbgCmdDoc;
extern int AllocatorDisableTrackingDbgCmd(const std::vector<std::string>&, std::string* output);

// AllocatorReportStateDbgCmd
//
// This is a debug command that lets you report the current general state of the default allocator.
//
extern const char* allocatorReportStateDbgCmdName;
extern const char* allocatorReportStateDbgCmdUsage;
extern const char* allocatorReportStateDbgCmdDesc;
extern const char* allocatorReportStateDbgCmdDoc;
extern int AllocatorReportStateDbgCmd(const std::vector<std::string>&, std::string* output);

///------------------------------------------------------------------------
/// ***** OVR_malloca / OVR_freea
///
/// Implements a safer version of alloca. However, see notes below.
///
/// Allocates memory from the stack via alloca (or similar) for smaller
/// allocation sizes, else falls back to operator new. This is very similar
/// to the Microsoft _malloca and _freea functions, and the implementation
/// is nearly the same aside from using operator new instead of malloc.
///
/// Unlike alloca, calls to OVR_malloca must be matched by calls to OVR_freea,
/// and the OVR_freea call must be in the same function scope as the original
/// call to OVR_malloca.
///
/// Note:
/// While this function reduces the likelihood of a stack overflow exception,
/// it cannot guarantee it, as even small allocation sizes done by alloca
/// can exhaust the stack when it is nearly full. However, the majority of
/// stack overflows due to alloca usage are due to large allocation size
/// requests.
///
/// Declarations:
///     void* OVR_malloca(size_t size);
///     void  OVR_freea(void* p);
///
/// Example usage:
///     void TestMalloca()
///     {
///         char* charArray = (char*)OVR_malloca(37000);
///
///         if(charArray)
///         {
///             // <use charArray>
///             OVR_freea(charArray);
///         }
///     }
///
#if !defined(OVR_malloca)
#define OVR_MALLOCA_ALLOCA_ID UINT32_C(0xcccccccc)
#define OVR_MALLOCA_MALLOC_ID UINT32_C(0xdddddddd)
#define OVR_MALLOCA_ID_SIZE \
  16 // Needs to be at least 2 * sizeof(uint32_t) and at least the minimum alignment for malloc on
// the platform. 16 works for all platforms.
#if defined(_MSC_VER)
#define OVR_MALLOCA_SIZE_THRESHOLD 8192
#else
#define OVR_MALLOCA_SIZE_THRESHOLD \
  1024 // Non-Microsoft platforms tend to exhaust stack space sooner due to non-automatic stack
// expansion.
#endif

#define OVR_malloca(size)                                                                     \
  ((((size) + OVR_MALLOCA_ID_SIZE) < OVR_MALLOCA_SIZE_THRESHOLD)                              \
       ? OVR::malloca_SetId(                                                                  \
             static_cast<char*>(alloca((size) + OVR_MALLOCA_ID_SIZE)), OVR_MALLOCA_ALLOCA_ID) \
       : OVR::malloca_SetId(                                                                  \
             static_cast<char*>(new char[(size) + OVR_MALLOCA_ID_SIZE]), OVR_MALLOCA_MALLOC_ID))

inline void* malloca_SetId(char* p, uint32_t id) {
  if (p) {
    *reinterpret_cast<uint32_t*>(p) = id;
    p = reinterpret_cast<char*>(p) + OVR_MALLOCA_ID_SIZE;
  }

  return p;
}
#endif

#if !defined(OVR_freea)
#define OVR_freea(p) OVR::freea_Impl(reinterpret_cast<char*>(p))

inline void freea_Impl(char* p) {
  if (p) {
    // We store the allocation type id at the first uint32_t in the returned memory.
    static_assert(
        OVR_MALLOCA_ID_SIZE >= sizeof(uint32_t), "Insufficient OVR_MALLOCA_ID_SIZE size.");
    p -= OVR_MALLOCA_ID_SIZE;
    uint32_t id = *reinterpret_cast<uint32_t*>(p);

    if (id == OVR_MALLOCA_MALLOC_ID)
      delete[] p;
#if defined(OVR_BUILD_DEBUG)
    else if (id != OVR_MALLOCA_ALLOCA_ID)
      OVR_FAIL_M("OVR_freea memory corrupt or not allocated by OVR_alloca.");
#endif
  }
}
#endif

///------------------------------------------------------------------------
/// ***** OVR_newa / OVR_deletea
///
/// Implements a C++ array version of OVR_malloca/OVR_freea.
/// Expresses failure via a nullptr return value and not via a C++ exception.
/// If a handled C++ exception occurs midway during construction in OVR_newa,
/// there is no automatic destruction of the successfully constructed elements.
///
/// Declarations:
///     T*   OVR_newa(T, size_t count);
///     void OVR_deletea(T, T* pTArray);
///
/// Example usage:
///     void TestNewa()
///     {
///         Widget* pWidgetArray = OVR_newa(Widget, 37000);
///
///         if(pWidgetArray)
///         {
///             // <use pWidgetArray>
///             OVR_deletea(Widget, pWidgetArray);
///         }
///     }
///
#if !defined(OVR_newa)
#define OVR_newa(T, count) \
  OVR::newa_Impl<T>(static_cast<char*>(OVR_malloca(count * sizeof(T))), count)
#endif

template <class T>
T* newa_Impl(char* pTArray, size_t count) {
  if (pTArray) {
    OVR::ConstructArray<T>(pTArray, count);

    // We store the count at the second uint32_t in the returned memory.
    static_assert(
        OVR_MALLOCA_ID_SIZE >= (2 * sizeof(uint32_t)), "Insufficient OVR_MALLOCA_ID_SIZE size.");
    reinterpret_cast<uint32_t*>((reinterpret_cast<char*>(pTArray) - OVR_MALLOCA_ID_SIZE))[1] =
        (uint32_t)count;
  }
  return reinterpret_cast<T*>(pTArray);
}

#if !defined(OVR_deletea)
#define OVR_deletea(T, pTArray) OVR::deletea_Impl<T>(pTArray)
#endif

template <class T>
void deletea_Impl(T* pTArray) {
  if (pTArray) {
    uint32_t count =
        reinterpret_cast<uint32_t*>((reinterpret_cast<char*>(pTArray) - OVR_MALLOCA_ID_SIZE))[1];
    OVR::DestructArray<T>(pTArray, count);
    OVR_freea(pTArray);
  }
}

//------------------------------------------------------------------------
// ***** OVR_DEBUG_HEAP_FILE_LINE_ENABLED
//
// Defined as 0 or 1.
// This is controlled by a #define because if we always compiled this code
// then release builds would have lots of file names in the binaries.
//
#ifndef OVR_DEBUG_HEAP_FILE_LINE_ENABLED
#if defined(OVR_BUILD_DEBUG)
#define OVR_DEBUG_HEAP_FILE_LINE_ENABLED 1
#else
#define OVR_DEBUG_HEAP_FILE_LINE_ENABLED 0
#endif
#endif

//------------------------------------------------------------------------
// ***** Memory Allocation Macros
//
// These macros should be used for global allocation. In the future, these
// macros will allows allocation to be extended with debug file/line information
// if necessary.

#define OVR_REALLOC(p, size) OVR::Allocator::GetInstance()->Realloc((p), (size))
#define OVR_FREE(p) OVR::Allocator::GetInstance()->Free((p))
#define OVR_FREE_ALIGNED(p) OVR::Allocator::GetInstance()->FreeAligned((p))

#if OVR_DEBUG_HEAP_FILE_LINE_ENABLED
#define OVR_ALLOC_TAGGED(size, tag) \
  OVR::Allocator::GetInstance()->AllocDebug((size), (tag), __FILE__, __LINE__)
#define OVR_ALLOC(size) \
  OVR::Allocator::GetInstance()->AllocDebug((size), nullptr, __FILE__, __LINE__)
#define OVR_ALLOC_ALIGNED_TAGGED(size, align, tag) \
  OVR::Allocator::GetInstance()->AllocAlignedDebug((size), (align), (tag), __FILE__, __LINE__)
#define OVR_ALLOC_ALIGNED(size, align) \
  OVR::Allocator::GetInstance()->AllocAlignedDebug((size), (align), nullptr, __FILE__, __LINE__)
#define OVR_ALLOC_DEBUG(size, file, line) \
  OVR::Allocator::GetInstance()->AllocDebug((size), nullptr, (file), (line))
#define OVR_ALLOC_DEBUG_TAGGED(size, tag, file, line) \
  OVR::Allocator::GetInstance()->AllocDebug((size), (tag), (file), (line))
#else
#define OVR_ALLOC_TAGGED(size, tag) OVR::Allocator::GetInstance()->Alloc((size), (tag))
#define OVR_ALLOC(size) OVR::Allocator::GetInstance()->Alloc((size), nullptr)
#define OVR_ALLOC_ALIGNED_TAGGED(size, align, tag) \
  OVR::Allocator::GetInstance()->AllocAligned((size), (align), (tag))
#define OVR_ALLOC_ALIGNED(size, align) \
  OVR::Allocator::GetInstance()->AllocAligned((size), (align), nullptr)
#define OVR_ALLOC_DEBUG(size, file, line) OVR::Allocator::GetInstance()->Alloc((size), nullptr)
#define OVR_ALLOC_DEBUG_TAGGED(size, tag, file, line) \
  OVR::Allocator::GetInstance()->Alloc((size), (tag))
#endif

//------------------------------------------------------------------------
// ***** NewOverrideBase
//
// Base class that overrides the new and delete operators.
// Deriving from this class, even as a multiple base, incurs no space overhead.
class NewOverrideBase {
 public:
  // Redefine all new & delete operators.
  OVR_MEMORY_REDEFINE_NEW(NewOverrideBase)
};

//------------------------------------------------------------------------
// ***** Mapped memory allocation
//
// Equates to VirtualAlloc/VirtualFree on Windows, mmap/munmap on Unix.
// These are useful for when you need system-supplied memory pages.
// These are also useful for when you need to allocate memory in a way
// that doesn't affect the application heap.

void* SafeMMapAlloc(size_t size);
void SafeMMapFree(const void* memory, size_t size);

} // namespace OVR

//------------------------------------------------------------------------
// ***** OVR_DEFINE_NEW
//
// Redefine operator 'new' if necessary.
// This allows us to remap all usage of new to something different.
//
#if defined(OVR_DEFINE_NEW)
#define new OVR_DEFINE_NEW
#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // OVR_Allocator_h
