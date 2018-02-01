/************************************************************************************

Filename    :   ExceptionHandler.cpp
Content     :   Platform-independent exception handling interface
Created     :   October 6, 2014

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#include "OVR_DebugHelp.h"
#include "OVR_Types.h"
#include "OVR_UTF8Util.h"
#include "OVR_Atomic.h"
#include "OVR_SysFile.h"
#include "OVR_Log.h"
#include "OVR_Std.h"
#include "Util/Util_SystemGUI.h"

#include <stdlib.h>
#include <time.h>

#if defined(OVR_OS_WIN32) || defined(OVR_OS_WIN64)
#pragma warning(push, 0)
OVR_DISABLE_MSVC_WARNING(4091) // 'keyword' : ignored on left of 'type' when no variable is declared
#include "OVR_Win32_IncludeWindows.h"
#include <ShlObj.h>
#include <WinNT.h>
#include <DbgHelp.h>
#include <WinVer.h>
#include <share.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <comutil.h>
#include <Wbemcli.h>
#include <Wbemidl.h>
#include <ObjBase.h>
#include <process.h>
#pragma warning(pop)

#pragma comment( \
    lib, "Psapi.lib") // To consider: It may be a problem to statically link to these libraries if
// the application at runtime intends to dynamically
#pragma comment( \
    lib, "ole32.lib") // link to a different version of the same library, and we are statically
// linked into that application instead of being a DLL.
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Version.lib")

// NtQueryInformation and THREAD_BASIC_INFORMATION are undocumented but frequently needed for
// digging into thread information.
typedef LONG(WINAPI* NtQueryInformationThreadFunc)(HANDLE, int, PVOID, ULONG, PULONG);

struct THREAD_BASIC_INFORMATION {
  LONG ExitStatus;
  PVOID TebBaseAddress;
  PVOID UniqueProcessId;
  PVOID UniqueThreadId;
  PVOID Priority;
  PVOID BasePriority;
};

#ifndef UNW_FLAG_NHANDLER // Older Windows SDKs don't support this.
#define UNW_FLAG_NHANDLER 0
#endif

#elif defined(OVR_OS_MAC)
#include <unistd.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/thread_status.h>
#include <mach/exception.h>
#include <mach/task.h>
#include <mach/thread_act.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>
#include <libproc.h>
#include <libgen.h>
#include <execinfo.h>
#include <cxxabi.h>
#include "OVR_mach_exc_OSX.h"

#if defined(__LP64__)
typedef struct mach_header_64 MachHeader;
typedef struct segment_command_64 SegmentCommand;
typedef struct section_64 Section;
#define kLCSegment LC_SEGMENT_64
#else
typedef struct mach_header MachHeader;
typedef struct segment_command SegmentCommand;
typedef struct section Section;
#define kLCSegment LC_SEGMENT
#endif

extern "C" const struct dyld_all_image_infos* _dyld_get_all_image_infos(); // From libdyld.dylib

#elif defined(OVR_OS_UNIX)
#include <unistd.h>
#include "sys/stat.h"
#if defined(OVR_OS_ANDROID)
#include <linux/sysctl.h>
#else
#include <sys/sysctl.h>
#endif
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <libgen.h>
#if !defined(OVR_OS_ANDROID)
#include <execinfo.h>
#endif
#include <cxxabi.h>
//#include <libunwind.h> // Can't use this until we can ensure that we have an installed version of
// it.
#endif

#if !defined(MIN)
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#endif

OVR_DISABLE_MSVC_WARNING(4351) // new behavior: elements of array will be default initialized
OVR_DISABLE_MSVC_WARNING(4996) // This function or variable may be unsafe

#if defined(OVR_OS_APPLE)
static OVR::ExceptionHandler* sExceptionHandler = nullptr;
const uint32_t sMachCancelMessageType = 0x0ca9ce11; // This is a made-up value of our own choice.

extern "C" {
kern_return_t catch_mach_exception_raise_OVR(
    mach_port_t /*exceptionPort*/,
    mach_port_t /*threadSysId*/,
    mach_port_t /*machTask*/,
    exception_type_t /*machExceptionType*/,
    mach_exception_data_t /*machExceptionData*/,
    mach_msg_type_number_t /*machExceptionDataCount*/) {
  return KERN_FAILURE;
}

kern_return_t catch_mach_exception_raise_state_OVR(
    mach_port_t /*exceptionPort*/,
    exception_type_t /*exceptionType*/,
    const mach_exception_data_t /*machExceptionData*/,
    mach_msg_type_number_t /*machExceptionDataCount*/,
    int* /*pMachExceptionFlavor*/,
    const thread_state_t /*threadStatePrev*/,
    mach_msg_type_number_t /*threaStatePrevCount*/,
    thread_state_t /*threadStateNew*/,
    mach_msg_type_number_t* /*pThreadStateNewCount*/) {
  return KERN_FAILURE;
}

kern_return_t catch_mach_exception_raise_state_identity_OVR(
    mach_port_t exceptionPort,
    mach_port_t threadSysId,
    mach_port_t machTask,
    exception_type_t exceptionType,
    mach_exception_data_type_t* pMachExceptionData,
    mach_msg_type_number_t machExceptionDataCount,
    int* pMachExceptionFlavor,
    thread_state_t threadStatePrev,
    mach_msg_type_number_t threadStatePrevCount,
    thread_state_t threadStateNew,
    mach_msg_type_number_t* pThreadStateNewCount) {
  return sExceptionHandler->HandleMachException(
      exceptionPort,
      threadSysId,
      machTask,
      exceptionType,
      pMachExceptionData,
      machExceptionDataCount,
      pMachExceptionFlavor,
      threadStatePrev,
      threadStatePrevCount,
      threadStateNew,
      pThreadStateNewCount);
}

void* MachHandlerThreadFunctionStatic(void* pExceptionHandlerVoid) {
  return static_cast<OVR::ExceptionHandler*>(pExceptionHandlerVoid)->MachHandlerThreadFunction();
}

} // extern "C"
#endif

namespace OVR {

void GetInstructionPointer(void*& pInstruction) {
#if defined(OVR_CC_MSVC)
  pInstruction = _ReturnAddress();
#else // GCC, clang
  pInstruction = __builtin_return_address(0);
#endif
}

// addressStrCapacity should be at least 2+16+1 = 19 characters.
static size_t SprintfAddress(char* addressStr, size_t addressStrCapacity, const void* pAddress) {
#if defined(OVR_CC_MSVC)
#if (OVR_PTR_SIZE >= 8)
  return snprintf(
      addressStr, addressStrCapacity, "0x%016I64x", pAddress); // e.g. 0x0123456789abcdef
#else
  return snprintf(addressStr, addressStrCapacity, "0x%08x", pAddress); // e.g. 0x89abcdef
#endif
#else
#if (OVR_PTR_SIZE >= 8)
  return snprintf(
      addressStr, addressStrCapacity, "%016lx", (uintptr_t)pAddress); // e.g. 0x0123456789abcdef
#else
  return snprintf(addressStr, addressStrCapacity, "%08lx", (uintptr_t)pAddress); // e.g. 0x89abcdef
#endif
#endif
}

///////////////////////////////////////////////////////////////////////////////
// GetMemoryAccess
//
// Returns MemoryAccess flags. Returns kMAUnknown for unknown access.
//
int GetMemoryAccess(const void* p) {
  int memoryAccessFlags = 0;

#if defined(_WIN32)
  MEMORY_BASIC_INFORMATION mbi;

  if (VirtualQuery(p, &mbi, sizeof(mbi))) {
    if (mbi.State == MEM_COMMIT) // If the memory page has been committed for use....
    {
      if (mbi.Protect & PAGE_READONLY)
        memoryAccessFlags |= kMARead;
      if (mbi.Protect & PAGE_READWRITE)
        memoryAccessFlags |= (kMARead | kMAWrite);
      if (mbi.Protect & PAGE_EXECUTE)
        memoryAccessFlags |= kMAExecute;
      if (mbi.Protect & PAGE_EXECUTE_READ)
        memoryAccessFlags |= (kMARead | kMAExecute);
      if (mbi.Protect & PAGE_EXECUTE_READWRITE)
        memoryAccessFlags |= (kMARead | kMAWrite | kMAExecute);
      if (mbi.Protect & PAGE_EXECUTE_WRITECOPY)
        memoryAccessFlags |= (kMAWrite | kMAExecute);
      if (mbi.Protect & PAGE_WRITECOPY)
        memoryAccessFlags |= kMAWrite;
    }
  }
#else
  OVR_UNUSED(p);
#endif

  return memoryAccessFlags;
}

/* Disabled until we can complete this, but leaving as a placeholder.

/// Adds the given memory access flags to the existing access.
/// Size doesn't have to be in page-sized increments.
///
bool AugmentMemoryAccess(const void* p, size_t size, int memoryAccessFlags)
{
    bool success = false;

    #ifdef _WIN32
        // This is tedious on Windows because it doesn't implement the memory access types as simple
flags.
        // We have to deal with each of the types individually:
        //     0, PAGE_NOACCESS, PAGE_READONLY, PAGE_READWRITE, PAGE_EXECUTE, PAGE_EXECUTE_READ,
        //     PAGE_EXECUTE_READWRITE, PAGE_EXECUTE_WRITECOPY, PAGE_WRITECOPY

        MEMORY_BASIC_INFORMATION mbi;

        if(VirtualQuery(p, &mbi, sizeof(mbi)))
        {
            DWORD dwOldProtect = 0;
            DWORD dwNewProtect = 0;

            (void)memoryAccessFlags;

            switch (mbi.Protect)
            {
                case 0:
                    break;
                case PAGE_NOACCESS:
                    break;
                case PAGE_READONLY:
                    break;
                case PAGE_READWRITE:
                    break;
                case PAGE_EXECUTE:
                    break;
                case PAGE_EXECUTE_READ:
                    break;
                case PAGE_EXECUTE_READWRITE:
                    break;
                case PAGE_EXECUTE_WRITECOPY:
                    break;
                case PAGE_WRITECOPY:
                    break;
            }

            if(VirtualProtect(const_cast<void*>(p), size, dwNewProtect, &dwOldProtect))
            {
                success = true;
            }
        }
    #endif

    return success;
}
*/

// threadHandleStrCapacity should be at least 2+16+1 = 19 characters.
static size_t SprintfThreadHandle(
    char* threadHandleStr,
    size_t threadHandleStrCapacity,
    const ThreadHandle& threadHandle) {
  return SprintfAddress(threadHandleStr, threadHandleStrCapacity, threadHandle);
}

// threadSysIdStrCapacity should be at least 20+4+16+2 = 42 characters.
static size_t SprintfThreadSysId(
    char* threadSysIdStr,
    size_t threadSysIdStrCapacity,
    const ThreadSysId& threadSysId) {
#if defined(OVR_CC_MSVC) // Somebody could conceivably use VC++ with a different standard library
  // that supports %ll. And VS2012+ also support %ll.
  return snprintf(
      threadSysIdStr,
      threadSysIdStrCapacity,
      "%I64u (0x%I64x)",
      (uint64_t)threadSysId,
      (uint64_t)threadSysId); // e.g. 5642 (0x160a)
#else
  return snprintf(
      threadSysIdStr,
      threadSysIdStrCapacity,
      "%llu (0x%llx)",
      (uint64_t)threadSysId,
      (uint64_t)threadSysId);
#endif
}

void GetThreadStackBounds(void*& pStackBase, void*& pStackLimit, ThreadHandle threadHandle) {
#if defined(OVR_OS_WIN64) || defined(OVR_OS_WIN32)
  ThreadSysId threadSysIdCurrent = (ThreadSysId)GetCurrentThreadId();
  ThreadSysId threadSysId;
  NT_TIB* pTIB = nullptr;

  if (threadHandle == OVR_THREADHANDLE_INVALID)
    threadSysId = threadSysIdCurrent;
  else
    threadSysId = ConvertThreadHandleToThreadSysId(threadHandle);

  if (threadSysId == threadSysIdCurrent) {
#if (OVR_PTR_SIZE == 4)
    // Need to use __asm__("movl %%fs:0x18, %0" : "=r" (pTIB) : : ); under gcc/clang.
    __asm {
                    mov eax, fs:[18h]
                    mov pTIB, eax
    }
#else
    pTIB = (NT_TIB*)NtCurrentTeb();
#endif
  } else {
#if (OVR_PTR_SIZE == 4)
    // It turns out we don't need to suspend the thread when getting SegFs/SegGS, as that's
    // constant per thread and doesn't require the thread to be suspended.
    // SuspendThread((HANDLE)threadHandle);

    CONTEXT context;
    memset(&context, 0, sizeof(context));
    context.ContextFlags = CONTEXT_SEGMENTS;

    if (::GetThreadContext(
            (HANDLE)threadHandle, &context)) // Requires THREAD_QUERY_INFORMATION privileges.
    {
      LDT_ENTRY ldtEntry;
      if (GetThreadSelectorEntry(
              threadHandle, context.SegFs, &ldtEntry)) // Requires THREAD_QUERY_INFORMATION
        pTIB = (NT_TIB*)((ldtEntry.HighWord.Bits.BaseHi << 24 ) | (ldtEntry.HighWord.Bits.BaseMid << 16) | ldtEntry.BaseLow);
    }

// ResumeThread((HANDLE)threadHandle);
#else
    // We cannot use GetThreadSelectorEntry or Wow64GetThreadSelectorEntry on Win64.
    // We need to read the SegGs qword at offset 0x30. We can't use pTIB =
    // (NT_TIB*)__readgsqword(0x30) because that reads only the current setGs offset.
    //    mov rax, qword ptr gs:[30h]
    //    mov qword ptr [pTIB],rax
    // In the meantime we rely on the NtQueryInformationThread function.

    static NtQueryInformationThreadFunc spNtQueryInformationThread = nullptr;

    if (!spNtQueryInformationThread) {
      HMODULE hNTDLL = GetModuleHandleW(L"ntdll.dll");
      spNtQueryInformationThread = (NtQueryInformationThreadFunc)(uintptr_t)GetProcAddress(
          hNTDLL, "NtQueryInformationThread");
    }

    if (spNtQueryInformationThread) {
      THREAD_BASIC_INFORMATION tbi;

      memset(&tbi, 0, sizeof(tbi));
      LONG result = spNtQueryInformationThread(
          threadHandle,
          0,
          &tbi,
          sizeof(tbi),
          nullptr); // Requires THREAD_QUERY_INFORMATION privileges
      if (result == 0)
        pTIB = (NT_TIB*)tbi.TebBaseAddress;
    }
#endif
  }

  if (pTIB && (GetMemoryAccess(pTIB) & kMARead)) {
    pStackBase = (void*)pTIB->StackBase;
    pStackLimit = (void*)pTIB->StackLimit;
  } else {
    pStackBase = nullptr;
    pStackLimit = nullptr;
  }

#elif defined(OVR_OS_APPLE)
  if (!threadHandle)
    threadHandle = pthread_self();

  pStackBase = pthread_get_stackaddr_np((pthread_t)threadHandle);
  size_t stackSize = pthread_get_stacksize_np((pthread_t)threadHandle);
  pStackLimit = (void*)((size_t)pStackBase - stackSize);

#elif defined(OVR_OS_UNIX)
  pStackBase = nullptr;
  pStackLimit = nullptr;

  pthread_attr_t threadAttr;
  pthread_attr_init(&threadAttr);

#if defined(OVR_OS_LINUX)
  int result = pthread_getattr_np((pthread_t)threadHandle, &threadAttr);
#else
  int result = pthread_attr_get_np((pthread_t)threadHandle, &threadAttr);
#endif

  if (result == 0) {
    size_t stackSize = 0;
    result = pthread_attr_getstack(&threadAttr, &pStackLimit, &stackSize);

    if (result == 0)
      pStackBase =
          (void*)((uintptr_t)pStackLimit + stackSize); // We assume the stack grows downward.
  }

#endif
}

bool KillCdeclFunction(
    void* pFunction,
    int32_t functionReturnValue,
    SavedFunction* pSavedFunction) {
#if defined(OVR_OS_MS)
  // The same implementation works for both 32 bit x86 and 64 bit x64.
  DWORD dwOldProtect;
  const uint8_t size =
      ((functionReturnValue == 0)
           ? 3
           : ((functionReturnValue == 1)
                  ? 5
                  : 6)); // This is the number of instruction bytes we overwrite below.

  if (VirtualProtect(pFunction, size, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
    if (pSavedFunction) // If the user wants to save the implementation for later restoration...
    {
      pSavedFunction->Function = pFunction;
      pSavedFunction->Size = size;
      memcpy(pSavedFunction->Data, pFunction, pSavedFunction->Size);

      const uint8_t opCode = *reinterpret_cast<uint8_t*>(pFunction);
      if (opCode == 0xe9) // If the function was a 32 bit relative jump to the real function (which
      // is a common thing)...
      {
        int32_t jumpOffset;
        memcpy(&jumpOffset, reinterpret_cast<uint8_t*>(pFunction) + 1, sizeof(int32_t));
        pSavedFunction->FunctionImplementation =
            reinterpret_cast<uint8_t*>(pFunction) + 5 + jumpOffset;
      } else
        pSavedFunction->FunctionImplementation = nullptr;
    }

    if (functionReturnValue == 0) // We write 3 bytes.
    {
      const uint8_t instructionBytes[] = {0x33, 0xc0, 0xc3}; // xor eax, eax; ret
      memcpy(pFunction, instructionBytes, sizeof(instructionBytes));
    } else if (functionReturnValue == 1) // We write 5 bytes.
    {
      uint8_t instructionBytes[] = {0x33, 0xc0, 0xff, 0xc0, 0xc3}; // xor eax, eax; inc eax; ret
      // -- note that this is smaller
      // than (mov eax 0x00000001;
      // ret), which takes 6 bytes.
      memcpy(pFunction, instructionBytes, sizeof(instructionBytes));
    } else // We write 6 bytes.
    {
      uint8_t instructionBytes[] = {0xb8, 0x00, 0x00, 0x00, 0x00, 0xc3}; // mov eax, 0x00000000; ret
      memcpy(
          instructionBytes + 1,
          &functionReturnValue,
          sizeof(int32_t)); // mov eax, functionReturnValue; ret
      memcpy(pFunction, instructionBytes, sizeof(instructionBytes));
    }

    VirtualProtect(pFunction, size, dwOldProtect, &dwOldProtect);
    return true;
  }
#else
  OVR_UNUSED3(pFunction, functionReturnValue, pSavedFunction);
#endif

  return false;
}

bool KillCdeclFunction(void* pFunction, SavedFunction* pSavedFunction) {
#if defined(OVR_OS_MS)
  // The same implementation works for both 32 bit x86 and 64 bit x64.
  DWORD dwOldProtect;
  const uint8_t size = 1;

  if (VirtualProtect(pFunction, size, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
    if (pSavedFunction) // If the user wants to save the implementation for later restoration...
    {
      pSavedFunction->Function = pFunction;
      pSavedFunction->Size = size;
      memcpy(pSavedFunction->Data, pFunction, pSavedFunction->Size);

      const uint8_t opCode = *reinterpret_cast<uint8_t*>(pFunction);
      if (opCode == 0xe9) // If the function was a 32 bit relative jump to the real function (which
      // is a common thing)...
      {
        int32_t jumpOffset;
        memcpy(&jumpOffset, reinterpret_cast<uint8_t*>(pFunction) + 1, sizeof(int32_t));
        pSavedFunction->FunctionImplementation =
            reinterpret_cast<uint8_t*>(pFunction) + 5 + jumpOffset;
      } else
        pSavedFunction->FunctionImplementation = nullptr;
    }

    const uint8_t instructionBytes[] = {0xc3}; // asm ret
    memcpy(pFunction, instructionBytes, sizeof(instructionBytes));
    VirtualProtect(pFunction, size, dwOldProtect, &dwOldProtect);
    return true;
  }

#else
  OVR_UNUSED2(pFunction, pSavedFunction);
#endif

  return false;
}

bool RedirectCdeclFunction(
    void* pFunction,
    const void* pDestFunction,
    OVR::SavedFunction* pSavedFunction) {
#if defined(_WIN32)
  // The same implementation works for both 32 bit x86 and 64 bit x64.
  // We implement this as a 32 bit relative jump from pFunction to pDestFunction.
  // This takes five bytes and is of the form:
  //    E9 <32bit offset>
  // This can work only when the pDestFunction is within 32 bits of pFunction. That will always be
  // the case when redirecting to a new location within the same module. But on 64 bit Windows, it
  // may be that pFunction is in one module and pDestFunction is in another module (e.g. DLL) with
  // an address that is farther than 32 bits away. In that case we need to instead do a 64 bit
  // absolute jump or if there isn't enough space for those instruction bytes then we need to do a
  // near jump to some nearby location where we can have a full 64 bit absolute jump. It turns out
  // that in the case of calling DLL functions the absolute-jump-through-64bit-data 0xff instruction
  // is used. We could change that 64 bit data.

#if defined(_WIN64)
  if (abs((intptr_t)pDestFunction - (intptr_t)pFunction) >= ((intptr_t)1 << 31)) {
    // A 64 bit jump would be required in this case, which we currently don't support, but could
    // with some effort.
    return false;
  }
#endif

  DWORD dwOldProtect;
  const uint8_t size = 5;

  if (VirtualProtect(pFunction, size, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
    if (pSavedFunction) // If the user wants to save the implementation for later restoration...
    {
      pSavedFunction->Function = pFunction;
      pSavedFunction->Size = size;
      memcpy(pSavedFunction->Data, pFunction, pSavedFunction->Size);

      const uint8_t opCode = *reinterpret_cast<uint8_t*>(pFunction);
      if (opCode == 0xe9) // If the function was a 32 bit relative jump to the real function (which
      // is a common thing)...
      {
        int32_t jumpOffset;
        memcpy(&jumpOffset, reinterpret_cast<uint8_t*>(pFunction) + 1, sizeof(int32_t));
        pSavedFunction->FunctionImplementation =
            reinterpret_cast<uint8_t*>(pFunction) + 5 + jumpOffset;
      } else
        pSavedFunction->FunctionImplementation = nullptr;
    }

    union Rel32Bytes {
      int32_t rel32;
      uint8_t bytes[4];
    } rel32Bytes = {
        ((int32_t)pDestFunction - (int32_t)pFunction) -
        size}; // -size because the offset is relative to after the 5 byte opcode sequence.

    uint8_t instructionBytes[] = {0xe9,
                                  rel32Bytes.bytes[0],
                                  rel32Bytes.bytes[1],
                                  rel32Bytes.bytes[2],
                                  rel32Bytes.bytes[3]}; // asm jmp <rel32>
    memcpy(pFunction, instructionBytes, sizeof(instructionBytes));
    VirtualProtect(pFunction, size, dwOldProtect, &dwOldProtect);
    return true;
  }

#else
  OVR_UNUSED3(pFunction, pDestFunction, pSavedFunction);
#endif

  return false;
}

bool RestoreCdeclFunction(SavedFunction* pSavedFunction) {
  if (pSavedFunction && pSavedFunction->Size) {
#if defined(OVR_OS_MS)
    DWORD dwOldProtect;

    if (VirtualProtect(
            pSavedFunction->Function,
            pSavedFunction->Size,
            PAGE_EXECUTE_READWRITE,
            &dwOldProtect)) {
      memcpy(pSavedFunction->Function, pSavedFunction->Data, pSavedFunction->Size);
      VirtualProtect(pSavedFunction->Function, pSavedFunction->Size, dwOldProtect, &dwOldProtect);
      return true;
    }
#else
    OVR_UNUSED(pSavedFunction);
#endif
  }

  return false;
}

CopiedFunction::CopiedFunction(const void* pFunction, size_t size) : Function(nullptr) {
  if (pFunction)
    Copy(pFunction, size);
}

CopiedFunction::~CopiedFunction() {
  // To consider: We may have use cases in which we want to intentionally not free the
  // memory and instead let it live beyond our lifetime so that it can still be called.
  Free();
}

const void* CopiedFunction::GetRealFunctionLocation(const void* pFunction) {
  // It turns out that many functions are really jumps to the actual function code.
  // These jumps are typically implemented with the E9 machine opcode, followed by
  // an int32 relative jump distance. We need to handle that case.

  // If the code is executable but not readable, we'll need to make it readable.
  bool readable = (pFunction && (GetMemoryAccess(pFunction) & OVR::kMARead) != 0);

  if (!readable) {
    return nullptr;

    // To do: Implement this:
    // readable = AugmentMemoryAccess(opCode, 1, OVR::kMARead);
  }

  const uint8_t* opCode = static_cast<const uint8_t*>(pFunction);

  if (*opCode == 0xE9) // If this is the E9 relative jmp instuction (which happens to be always used
  // for local-module trampolining by VC++)...
  {
    int32_t jumpDistance;
    memcpy(&jumpDistance, opCode + 1, sizeof(jumpDistance));

    pFunction = (opCode + 5 + jumpDistance); // +5 because the jmp is relative to the end of the
    // five byte 0xE9 instruction.
    // Is it possible that pFunction points to another trampoline? I haven't seen such a thing,
    // but it could be handled by a loop here or simply goto the opCode assignment line above.
  }

  return pFunction;
}

const void* CopiedFunction::Copy(const void* pFunction, size_t size) {
  Free();

#if defined(_WIN32)
  if (size == 0) // If we don't know the size...
  {
    // When debug symbols are present, we could look up the function size.
    // On x64 we can possibly look up the size via the stack unwind data.
    // On x86 and x64 we could possibly look for return statements while
    // also checking if pFunction is simply a short trampoline.
    size = 4096; // For our current uses this is good enough. But it's not general.
  }

  void* pNewFunction =
      VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

  if (pNewFunction) {
    // It turns out that (especially in debug builds), pFunction may be a
    // trampoline (unilateral jump) to the real function implementation elsewhere.
    // We need to copy the real implementation and not the jump, at least if the
    // jump is a relative jump (usually the case) and not an absolute jump.
    const void* pRealFunction = GetRealFunctionLocation(pFunction);

    memcpy(pNewFunction, pRealFunction, size);
    Function = pNewFunction;
  }
#else
  OVR_UNUSED2(pFunction, size);
#endif

  return Function;
}

void CopiedFunction::Free() {
  if (Function) {
#if defined(_WIN32)
    VirtualFree(Function, 0, MEM_RELEASE);
#endif

    Function = nullptr;
  }
}

static bool DebuggerForcedNotPresent = false;

// Force OVRIsDebuggerPresent to return false
void ForceDebuggerNotPresent() {
  DebuggerForcedNotPresent = true;
}

// Allow debugger check to proceded as normal
void ClearDebuggerNotPresent() {
  DebuggerForcedNotPresent = false;
}

bool OVRIsDebuggerPresent() {
  if (DebuggerForcedNotPresent) {
    return false;
  }
#if defined(OVR_OS_MS)
  return ::IsDebuggerPresent() != 0;

#elif defined(OVR_OS_APPLE)
  int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};
  struct kinfo_proc info;
  size_t size = sizeof(info);

  info.kp_proc.p_flag = 0;
  sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0);

  return ((info.kp_proc.p_flag & P_TRACED) != 0);

#elif defined(PT_TRACE_ME) && !defined(OVR_OS_ANDROID)
  return (ptrace(PT_TRACE_ME, 0, 1, 0) < 0);

#elif (defined(OVR_OS_LINUX) || defined(OVR_OS_BSD)) && !defined(OVR_OS_ANDROID)
  // This works better than the PT_TRACE_ME approach, but causes the debugger to get
  // confused when executed under a debugger.
  // It also presents this problem:
  //     http://pubs.opengroup.org/onlinepubs/009695399/functions/fork.html
  //     When the application calls fork() from a signal handler and any of the
  //     fork handlers registered by pthread_atfork() calls a function that is
  //     not asynch-signal-safe, the behavior is undefined.
  // We may need to provide two pathways within this function, one of which
  // doesn't fork and instead uses PT_TRACE_ME.
  int pid = fork();
  int status;
  bool present = false;

  if (pid == -1) // If fork failed...
  {
    // perror("fork");
  } else if (pid == 0) // If we are the child process...
  {
    int ppid = getppid();

#if defined(OVR_OS_LINUX)
    if (ptrace(PTRACE_ATTACH, ppid, nullptr, nullptr) == 0)
#else
    if (ptrace(PT_ATTACH, ppid, nullptr, nullptr) == 0)
#endif
    {
      waitpid(ppid, nullptr, 0);

#if defined(OVR_OS_LINUX)
      ptrace(PTRACE_CONT, getppid(), nullptr, nullptr);
      ptrace(PTRACE_DETACH, getppid(), nullptr, nullptr);
#else
      ptrace(PT_CONTINUE, getppid(), nullptr, nullptr);
      ptrace(PT_DETACH, getppid(), nullptr, nullptr);
#endif
    } else {
      // ptrace failed so the debugger is present.
      present = true;
    }

    exit(present ? 1 : 0); // The WEXITSTATUS call below will read this exit value.
  } else // Else we are the original process.
  {
    waitpid(pid, &status, 0);
    present =
        WEXITSTATUS(status) ? true : false; // Read the exit value from the child's call to exit.
  }

  return present;

#else
  return false;
#endif
}

// Exits the process with the given exit code.
void ExitProcess(intptr_t processReturnValue) {
  exit((int)processReturnValue);
}

// Note that we can't just return sizeof(void*) == 8, as we may have the case of a
// 32 bit app running on a 64 bit operating system.
static bool Is64BitOS() {
#if (OVR_PTR_SIZE >= 8)
  return true;

#elif defined(OVR_OS_WIN32) || defined(OVR_OS_WIN64)
  BOOL is64BitOS = FALSE;
  bool IsWow64ProcessPresent =
      (GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "IsWow64Process") != nullptr);
  return (IsWow64ProcessPresent && IsWow64Process(GetCurrentProcess(), &is64BitOS) && is64BitOS);

#elif defined(OVR_OS_MAC) || defined(OVR_OS_UNIX)
  utsname utsName;
  memset(&utsName, 0, sizeof(utsName));
  return (uname(&utsName) == 0) && (strcmp(utsName.machine, "x86_64") == 0);

#else
  return false;
#endif
}

// The output will always be 0-terminated.
// Returns the required strlen of the output.
// Returns (size_t)-1 on failure.
size_t SpawnShellCommand(const char* shellCommand, char* output, size_t outputCapacity) {
#if defined(OVR_OS_UNIX) || defined(OVR_OS_APPLE)
  FILE* pFile = popen(shellCommand, "r");

  if (pFile) {
    size_t requiredLength = 0;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), pFile)) // fgets 0-terminates the buffer.
    {
      size_t length = OVR_strlen(buffer);
      requiredLength += length;

      if (outputCapacity) {
        OVR_strlcpy(output, buffer, outputCapacity);
        length = MIN(outputCapacity, length);
      }

      output += length;
      outputCapacity -= length;
    }

    pclose(pFile);
    return requiredLength;
  }
#else
  // To do. Properly solving this on Windows requires a bit of code.
  OVR_UNUSED(shellCommand);
  OVR_UNUSED(output);
  OVR_UNUSED(outputCapacity);
#endif

  return (size_t)-1;
}

// Retrieves the name of the given thread.
// To do: Move this to OVR_Threads.h
bool GetThreadName(OVR::ThreadHandle threadHandle, char* threadName, size_t threadNameCapacity) {
#if (defined(OVR_OS_APPLE) || defined(OVR_OS_LINUX)) && !defined(OVR_OS_ANDROID)
  int result = pthread_getname_np((pthread_t)threadHandle, threadName, threadNameCapacity);
  if (result == 0)
    return true;
#else
  // This is not currently possible on Windows, as only the debugger stores the thread name. We
  // could potentially use a vectored
  // exception handler to catch all thread name exceptions (0x406d1388) and record them in a static
  // list at runtime. To detect
  // thread exit we could use WMI Win32_ThreadStopTrace. Maintain a list of thread names between
  // these two events.
  OVR_UNUSED(threadHandle);
  OVR_UNUSED(threadNameCapacity);
#endif

  if (threadNameCapacity)
    threadName[0] = 0;

  return false;
}

OVR::ThreadSysId ConvertThreadHandleToThreadSysId(OVR::ThreadHandle threadHandle) {
#if defined(OVR_OS_WIN64)
  return (OVR::ThreadSysId)::GetThreadId(
      threadHandle); // Requires THREAD_QUERY_INFORMATION privileges.

#elif defined(OVR_OS_WIN32)
  typedef DWORD(WINAPI * GetThreadIdFunc)(HANDLE);

  static volatile bool sInitialized = false;
  static GetThreadIdFunc spGetThreadIdFunc = nullptr;
  static NtQueryInformationThreadFunc spNtQueryInformationThread = nullptr;

  if (!sInitialized) {
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (hKernel32)
      spGetThreadIdFunc = (GetThreadIdFunc)(uintptr_t)GetProcAddress(hKernel32, "GetThreadId");

    if (!spGetThreadIdFunc) {
      HMODULE hNTDLL = GetModuleHandleA("ntdll.dll");

      if (hNTDLL)
        spNtQueryInformationThread = (NtQueryInformationThreadFunc)(uintptr_t)GetProcAddress(
            hNTDLL, "NtQueryInformationThread");
    }

    sInitialized = true;
  }

  if (spGetThreadIdFunc)
    return (OVR::ThreadSysId)spGetThreadIdFunc(threadHandle);

  if (spNtQueryInformationThread) {
    THREAD_BASIC_INFORMATION tbi;

    if (spNtQueryInformationThread(threadHandle, 0, &tbi, sizeof(tbi), nullptr) == 0)
      return (OVR::ThreadSysId)tbi.UniqueThreadId;
  }

  return OVR_THREADSYSID_INVALID;

#elif defined(OVR_OS_APPLE)
  mach_port_t threadSysId = pthread_mach_thread_np((pthread_t)threadHandle); // OS 10.4 and later.
  return (ThreadSysId)threadSysId;

#elif defined(OVR_OS_LINUX)

  // I believe we can usually (though not portably) intepret the pthread_t as a pointer to a struct
  // whose first member is a lwp id.
  OVR_UNUSED(threadHandle);
  return OVR_THREADSYSID_INVALID;

#else
  OVR_UNUSED(threadHandle);
  return OVR_THREADSYSID_INVALID;
#endif
}

OVR::ThreadHandle ConvertThreadSysIdToThreadHandle(OVR::ThreadSysId threadSysId) {
  if (threadSysId == OVR_THREADSYSID_INVALID)
    return OVR_THREADHANDLE_INVALID;

#if defined(OVR_OS_MS)
  // We currently request the given rights because that's what users of this function typically need
  // it for. Ideally there would
  // be a way to specify the requested rights in order to avoid the problem if we need only a subset
  // of them but can't get it.
  // The solution we use below to try opening with successively reduced rights will work for our
  // uses here but isn't a good general solution to this.
  OVR::ThreadHandle threadHandle = ::OpenThread(
      THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
      TRUE,
      (DWORD)threadSysId);

  if (threadHandle == OVR_THREADHANDLE_INVALID) {
    threadHandle =
        ::OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, TRUE, (DWORD)threadSysId);

    if (threadHandle == OVR_THREADHANDLE_INVALID)
      threadHandle = ::OpenThread(THREAD_QUERY_INFORMATION, TRUE, (DWORD)threadSysId);
  }

  return threadHandle;
#elif defined(OVR_OS_MAC)
  return (ThreadHandle)pthread_from_mach_thread_np((mach_port_t)threadSysId);
#else
  return (ThreadHandle)threadSysId;
#endif
}

void FreeThreadHandle(OVR::ThreadHandle threadHandle) {
#if defined(OVR_OS_MS)
  if (threadHandle != OVR_THREADHANDLE_INVALID)
    ::CloseHandle(threadHandle);
#else
  OVR_UNUSED(threadHandle);
#endif
}

OVR::ThreadSysId GetCurrentThreadSysId() {
#if defined(OVR_OS_MS)
  return ::GetCurrentThreadId();
#elif defined(OVR_OS_APPLE)
  return (ThreadSysId)mach_thread_self();
#else
  return (ThreadSysId)pthread_self();
#endif
}

static void GetCurrentProcessFilePath(char* appPath, size_t appPathCapacity) {
  appPath[0] = 0;

#if defined(OVR_OS_MS)
  wchar_t pathW[OVR_MAX_PATH];
  GetModuleFileNameW(0, pathW, (DWORD)OVR_ARRAY_COUNT(pathW));

  auto requiredUTF8Length = OVR::UTF8Util::Strlcpy(appPath, appPathCapacity, pathW);
  if (requiredUTF8Length >= appPathCapacity) {
    appPath[0] = 0;
  }
#elif defined(OVR_OS_APPLE)
  struct BunderFolder {
    // Returns true if pStr ends with pFind, case insensitively.
    // To do: Move OVR_striend to OVRKernel/Std.h
    static bool OVR_striend(
        const char* pStr,
        const char* pFind,
        size_t strLength = (size_t)-1,
        size_t findLength = (size_t)-1) {
      if (strLength == (size_t)-1)
        strLength = OVR_strlen(pStr);
      if (findLength == (size_t)-1)
        findLength = OVR_strlen(pFind);
      if (strLength >= findLength)
        return (OVR_stricmp(pStr + strLength - findLength, pFind) == 0);
      return false;
    }

    static bool IsBundleFolder(const char* filePath) {
      // https://developer.apple.com/library/mac/documentation/CoreFoundation/Conceptual/CFBundles/AboutBundles/AboutBundles.html#//apple_ref/doc/uid/10000123i-CH100-SW1
      static const char* extensionArray[] = {".app", ".bundle", ".framework", ".plugin", ".kext"};

      for (size_t i = 0; i < OVR_ARRAY_COUNT(extensionArray); i++) {
        if (OVR_striend(filePath, extensionArray[i]))
          return true;
      }

      return false;
    }
  };

  char appPathTemp[PATH_MAX];
  uint32_t appPathTempCapacity32 = PATH_MAX;
  size_t requiredStrlen = appPathCapacity;

  if (_NSGetExecutablePath(appPathTemp, &appPathTempCapacity32) == 0) {
    char appPathTempReal[PATH_MAX];

    if (realpath(
            appPathTemp,
            appPathTempReal)) // If the path is a symbolic link, this converts it to the real path.
    {
      // To consider: Enable reading the internal bundle executable path. An application on Mac may
      // in
      // fact be within a file bundle, which is an private file system within a file. With Objective
      // C
      // we could use: [[NSWorkspace sharedWorkspace] isFilePackageAtPath:fullPath];
      bool shouldReadTheBunderPath = false;

      if (shouldReadTheBunderPath) {
        // We recursively call dirname() until we find .app/.bundle/.plugin as a directory name.
        OVR_strlcpy(appPathTemp, appPathTempReal, OVR_ARRAY_COUNT(appPathTemp));
        bool found = BunderFolder::IsBundleFolder(appPathTemp);

        while (!found && OVR_strcmp(appPathTemp, ".") && OVR_strcmp(appPathTemp, "/")) {
          OVR_strlcpy(appPathTemp, dirname(appPathTemp), OVR_ARRAY_COUNT(appPathTemp));
          found = BunderFolder::IsBundleFolder(appPathTemp);
        }

        if (found) // If somewhere above we found a parent bundle container...
          requiredStrlen = OVR_strlcpy(appPath, appPathTemp, appPathCapacity);
        else
          requiredStrlen = OVR_strlcpy(appPath, appPathTempReal, appPathCapacity);
      } else {
        requiredStrlen = OVR_strlcpy(appPath, appPathTempReal, appPathCapacity);
      }
    }
  }

  if (requiredStrlen >= appPathCapacity)
    appPath[0] = '\0';

#elif defined(OVR_OS_LINUX)
  ssize_t length = readlink("/proc/self/exe", appPath, appPathCapacity);

  if ((length != -1) && ((size_t)length < (appPathCapacity - 1))) {
    appPath[length] = '\0';
  }
#endif
}

static const char* GetFileNameFromPath(const char* filePath) {
  const char* lastPathSeparator = strrchr(filePath, '/');

#if defined(OVR_OS_MS)
  // Microsoft APIs are inconsistent with respect to allowing / as a path separator.
  const char* candidate = strrchr(filePath, '\\');

  if (candidate > lastPathSeparator) {
    lastPathSeparator = candidate;
  }
#endif

  if (lastPathSeparator)
    return lastPathSeparator + 1;

  return filePath;
}

static void FormatDateTime(
    char* buffer,
    size_t bufferCapacity,
    time_t timeValue,
    bool getDate,
    bool getTime,
    bool localDateTime,
    bool fileNameSafeCharacters = false) {
  char temp[128];
  const tm* pTime = localDateTime ? localtime(&timeValue) : gmtime(&timeValue);

  if (bufferCapacity)
    buffer[0] = 0;

  if (getDate) {
    const char* format = fileNameSafeCharacters ? "%Y-%m-%d" : "%Y/%m/%d";
    strftime(temp, OVR_ARRAY_COUNT(temp), format, pTime);
    OVR::OVR_strlcpy(buffer, temp, bufferCapacity);
  }

  if (getTime) {
    const char* format = fileNameSafeCharacters ? " %H.%M.%S" : " %H:%M:%S";
    strftime(temp, OVR_ARRAY_COUNT(temp), (getDate ? format : format + 1), pTime);
    OVR::OVR_strlcat(buffer, temp, bufferCapacity);
  }
}

void GetOSVersionName(char* versionName, size_t versionNameCapacity) {
#if defined(OVR_OS_MS)
  const char* name = "unknown";

  RTL_OSVERSIONINFOEXW vi = {};
  vi.dwOSVersionInfoSize = sizeof(vi);

  // We use RtlGetVersion instead of GetVersionExW because the latter doesn't actually return the
  // version of Windows, it returns
  // the highest version of Windows that this application's manifest declared that it was compatible
  // with. We want to know the
  // real version of Windows and so need to fall back to RtlGetVersion.
  LONG(WINAPI * pfnRtlGetVersion)(RTL_OSVERSIONINFOEXW*);
  pfnRtlGetVersion = (decltype(pfnRtlGetVersion))(uintptr_t)GetProcAddress(
      GetModuleHandleW((L"ntdll.dll")), "RtlGetVersion");

  if (pfnRtlGetVersion) // This will virtually always succeed.
  {
    if (pfnRtlGetVersion(&vi) != 0) // pfnRtlGetVersion will virtually always succeed.
      memset(&vi, 0, sizeof(vi));
  }

  if (vi.dwMajorVersion == 10) {
    if (vi.dwMinorVersion == 0) {
      if (vi.dwBuildNumber >= 10586) {
        if (vi.wProductType == VER_NT_WORKSTATION)
          name = "Windows 10 TH2+";
        else
          name = "Windows Server 2016 Technical Preview TH2+";
      } else {
        if (vi.wProductType == VER_NT_WORKSTATION)
          name = "Windows 10";
        else
          name = "Windows Server 2016 Technical Preview";
      }
    } else {
      // Unknown recent version.
      if (vi.wProductType == VER_NT_WORKSTATION)
        name = "Windows 10 Unknown";
      else
        name = "Windows Server 2016 Unknown";
    }
  } else if (vi.dwMajorVersion >= 7) {
    // Unknown recent version.
  } else if (vi.dwMajorVersion >= 6) {
    if (vi.dwMinorVersion >= 4)
      name = "Windows 10 Pre Released";
    else if (vi.dwMinorVersion >= 3) {
      if (vi.wProductType == VER_NT_WORKSTATION)
        name = "Windows 8.1";
      else
        name = "Windows Server 2012 R2";
    } else if (vi.dwMinorVersion >= 2) {
      if (vi.wProductType == VER_NT_WORKSTATION)
        name = "Windows 8";
      else
        name = "Windows Server 2012";
    } else if (vi.dwMinorVersion >= 1) {
      if (vi.wProductType == VER_NT_WORKSTATION)
        name = "Windows 7";
      else
        name = "Windows Server 2008 R2";
    } else {
      if (vi.wProductType == VER_NT_WORKSTATION)
        name = "Windows Vista";
      else
        name = "Windows Server 2008";
    }
  } else if (vi.dwMajorVersion >= 5) {
    if (vi.dwMinorVersion == 0)
      name = "Windows 2000";
    else if (vi.dwMinorVersion == 1)
      name = "Windows XP";
    else // vi.dwMinorVersion == 2
    {
      if (GetSystemMetrics(SM_SERVERR2) != 0)
        name = "Windows Server 2003 R2";
      else if (vi.wSuiteMask & VER_SUITE_WH_SERVER)
        name = "Windows Home Server";
      if (GetSystemMetrics(SM_SERVERR2) == 0)
        name = "Windows Server 2003";
      else
        name = "Windows XP Professional x64 Edition";
    }
  } else
    name = "Windows 98 or earlier";

  OVR_strlcpy(versionName, name, versionNameCapacity);

  if (vi.szCSDVersion[0]) // If the version is reporting a service pack string...
  {
    OVR_strlcat(versionName, ", ", versionNameCapacity);

    char servicePackname8[128];
    OVR::UTF8Util::Strlcpy(servicePackname8, sizeof(servicePackname8), vi.szCSDVersion);
    OVR_strlcat(versionName, servicePackname8, versionNameCapacity);
  }
#elif defined(OVR_OS_UNIX) || defined(OVR_OS_APPLE)
  utsname utsName;
  memset(&utsName, 0, sizeof(utsName));

  if (uname(&utsName) == 0)
    snprintf(
        versionName,
        versionNameCapacity,
        "%s %s %s %s",
        utsName.sysname,
        utsName.release,
        utsName.version,
        utsName.machine);
  else
    snprintf(versionName, versionNameCapacity, "Unix");
#endif
}

void CreateException(CreateExceptionType exceptionType) {
  char buffer[1024] = {};

  switch (exceptionType) {
    case kCETAccessViolation: {
      int* pNullPtr = reinterpret_cast<int*>((rand() / 2) / RAND_MAX);
      pNullPtr[0] = 0; // This line should generate an exception.
      sprintf(buffer, "%p", pNullPtr);
      break;
    }

    case kCETDivideByZero: {
      int smallValue = 1;
      int largeValue = (1000 * exceptionType);
      int divByZero = (smallValue / largeValue); // This line should generate a div/0 exception.
      sprintf(buffer, "%d", divByZero);
      break;
    }

    case kCETIllegalInstruction: {
#if defined(OVR_CPU_X86) ||     \
    (defined(OVR_CPU_X86_64) && \
     !defined(OVR_CC_MSVC)) // (if x86) or (if x64 and any computer but VC++)...
#if defined(OVR_CC_MSVC)
      __asm ud2
#else // e.g. GCC
      asm volatile("ud2");
#endif

#elif defined(OVR_CPU_X86_64) && (defined(OVR_OS_MS) && defined(PAGE_EXECUTE_READWRITE))
      // VC++ for x64 doesn't support inline asm.
      void* pVoid = _AddressOfReturnAddress();
      void** ppVoid = reinterpret_cast<void**>(pVoid);
      void* pReturnAddress = *ppVoid;
      DWORD dwProtectPrev = 0;

      if (VirtualProtect(
              pReturnAddress,
              2,
              PAGE_EXECUTE_READWRITE,
              &dwProtectPrev)) // If we can set the memory to be executable...
      {
        // Modify the code we return to.
        uint8_t asm_ud2[] = {0x0f, 0x0b};
        memcpy(pReturnAddress, asm_ud2, sizeof(asm_ud2));
        VirtualProtect(pReturnAddress, 2, dwProtectPrev, &dwProtectPrev);
      } else {
        // To do: Fix this.
      }

#else
// To do: Fix this.
#endif

          break;
    }

    case kCETStackCorruption: {
      size_t size = (sizeof(buffer) * 16) - (rand() % 16);
      char* pOutsizeStack = buffer - ((sizeof(buffer) * 16) + (rand() % 16));

      memset(buffer, 0, size);
      memset(pOutsizeStack, 0, size); // This line should generate an exception, or an exception
      // will be generated upon return from this function.
      break;
    }

    case kCETStackOverflow: {
      CreateException(exceptionType); // Call ourselves recursively. This line should generate a
      // div/0 exception.
      sprintf(buffer, "%d", exceptionType);
      break;
    }

    case kCETAlignment: {
      // Not all platforms generate alignment exceptions. Some internally handle it.
      void* pAligned = malloc(16);
      char* pMisaligned = (char*)pAligned + 1;
      uint64_t* pMisaligned64 = reinterpret_cast<uint64_t*>(pMisaligned);

      *pMisaligned64 = 0; // This line should generate an exception.
      free(pAligned);
      break;
    }

    case kCETFPU:
      // Platforms usually have FPU exceptions disabled. In order to test FPU exceptions we will
      // need to at least
      // temporarily disable them before executing code here to generate such exceptions.
      // To do.
      break;

    case kCETTrap:
      // To do. This is hardware-specific.
      break;
  }
}

//-----------------------------------------------------------------------------
// SymbolLookup

#if defined(OVR_OS_MS)
#if defined(OVR_CC_MSVC)
// The Lock below is one that we want to keep around as long as possible, as there may be
// application code that
// needs to do symbol lookups during process teardown after main has returned. The init_seg(lib)
// statement
// below makes it so that this module's globals are initialized right after the C standard library
// has initialized,
// and are destroyed right before the C standard library is destroyed (after after all other app
// globals are destroyed).
#pragma warning(disable : 4073) // warning C4073: initializers put in library initialization area.
#pragma warning( \
    disable : 4075) // warning C4075: initializers put in unrecognized initialization area.
#pragma init_seg(lib)
#endif

typedef BOOL(WINAPI* StackWalk64Type)(
    DWORD MachineType,
    HANDLE hProcess,
    HANDLE hThread,
    LPSTACKFRAME64 StackFrame,
    PVOID ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
typedef PVOID(WINAPI* SymFunctionTableAccess64Type)(HANDLE hProcess, DWORD64 dwAddr);
typedef DWORD64(WINAPI* SymGetModuleBase64Type)(HANDLE hProcess, DWORD64 dwAddr);
typedef DWORD(WINAPI* SymSetOptionsType)(DWORD SymOptions);
typedef BOOL(
    WINAPI* SymInitializeWType)(HANDLE hProcess, PCWSTR UserSearchPath, BOOL fInvadeProcess);
typedef BOOL(WINAPI* SymCleanupType)(HANDLE hProcess);
typedef DWORD64(WINAPI* SymLoadModule64Type)(
    HANDLE hProcess,
    HANDLE hFile,
    PCSTR ImageName,
    PCSTR ModuleName,
    DWORD64 BaseOfDll,
    DWORD SizeOfDll);
typedef BOOL(WINAPI* SymFromAddrType)(
    HANDLE hProcess,
    DWORD64 Address,
    PDWORD64 Displacement,
    PSYMBOL_INFO Symbol);
typedef BOOL(WINAPI* SymGetLineFromAddr64Type)(
    HANDLE hProcess,
    DWORD64 qwAddr,
    PDWORD pdwDisplacement,
    PIMAGEHLP_LINE64 Line64);

static StackWalk64Type pStackWalk64 = nullptr;
static SymFunctionTableAccess64Type pSymFunctionTableAccess64 = nullptr;
static SymGetModuleBase64Type pSymGetModuleBase64 = nullptr;
static SymSetOptionsType pSymSetOptions = nullptr;
static SymInitializeWType pSymInitializeW = nullptr;
static SymCleanupType pSymCleanup = nullptr;
static SymLoadModule64Type pSymLoadModule64 = nullptr;
static SymFromAddrType pSymFromAddr = nullptr;
static SymGetLineFromAddr64Type pSymGetLineFromAddr64 = nullptr;
static int32_t sSymUsageCount = 0;
static HMODULE sDbgHelp = nullptr;

static OVR::Lock* GetSymbolLookupLockPtr() {
  static OVR::Lock sDbgHelpLock;
  return &sDbgHelpLock;
}

bool SymbolLookup::Initialize() {
  OVR::Lock::Locker autoLock(GetSymbolLookupLockPtr());

  if (++sSymUsageCount > 1) {
    OVR_ASSERT(
        pSymInitializeW !=
        nullptr); // If it was already initialized then the pointers should be valid.
    return true;
  }

  // http://msdn.microsoft.com/en-us/library/windows/desktop/ms679294%28v=vs.85%29.aspx
  sDbgHelp = LoadLibraryW(
      L"DbgHelp.dll"); // It's best if the application supplies a recent version of this.

  if (sDbgHelp) {
    pStackWalk64 = (StackWalk64Type)(uintptr_t)::GetProcAddress(sDbgHelp, "StackWalk64");
    pSymFunctionTableAccess64 = (SymFunctionTableAccess64Type)(uintptr_t)::GetProcAddress(
        sDbgHelp, "SymFunctionTableAccess64");
    pSymGetModuleBase64 =
        (SymGetModuleBase64Type)(uintptr_t)::GetProcAddress(sDbgHelp, "SymGetModuleBase64");
    pSymSetOptions = (SymSetOptionsType)(uintptr_t)::GetProcAddress(sDbgHelp, "SymSetOptions");
    pSymInitializeW = (SymInitializeWType)(uintptr_t)::GetProcAddress(sDbgHelp, "SymInitializeW");
    pSymCleanup = (SymCleanupType)(uintptr_t)::GetProcAddress(sDbgHelp, "SymCleanup");
    pSymLoadModule64 =
        (SymLoadModule64Type)(uintptr_t)::GetProcAddress(sDbgHelp, "SymLoadModule64");
    pSymFromAddr = (SymFromAddrType)(uintptr_t)::GetProcAddress(sDbgHelp, "SymFromAddr");
    pSymGetLineFromAddr64 =
        (SymGetLineFromAddr64Type)(uintptr_t)::GetProcAddress(sDbgHelp, "SymGetLineFromAddr64");

    // To consider: Use a manually created search path:
    // wchar_t searchPathW[4096]; // Semicolon-separated strings.
    //     The current working directory of the application.
    //     The directory of the application itself (GetModuleFileName).
    //     The _NT_SYMBOL_PATH environment variable.
    //     The _NT_ALTERNATE_SYMBOL_PATH environment variable.

    if (pSymInitializeW) {
      if (pSymInitializeW(GetCurrentProcess(), nullptr /*searchPathW*/, FALSE)) {
        if (pSymSetOptions) {
          pSymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
        }

        return true;
      }
    }
  }

  --sSymUsageCount;
  return false;
}

bool SymbolLookup::IsInitialized() {
  // Note that it's possible that another thread could change the state of this right after the
  // return.
  OVR::Lock::Locker autoLock(GetSymbolLookupLockPtr());
  return (sSymUsageCount != 0);
}

void SymbolLookup::Shutdown() {
  OVR::Lock::Locker autoLock(GetSymbolLookupLockPtr());

  if (sSymUsageCount > 0 && --sSymUsageCount <= 0) {
    pSymCleanup(GetCurrentProcess());

    pStackWalk64 = nullptr;
    pSymFunctionTableAccess64 = nullptr;
    pSymGetModuleBase64 = nullptr;
    pSymSetOptions = nullptr;
    pSymInitializeW = nullptr;
    pSymCleanup = nullptr;
    pSymLoadModule64 = nullptr;
    pSymFromAddr = nullptr;
    pSymGetLineFromAddr64 = nullptr;

    FreeLibrary(sDbgHelp);
    sDbgHelp = nullptr;
  }
}
#else
bool SymbolLookup::Initialize() {
  return true;
}

bool SymbolLookup::IsInitialized() {
  return true;
}

void SymbolLookup::Shutdown() {}
#endif

SymbolLookup::SymbolLookup()
    : AllowMemoryAllocation(true),
      ModuleListUpdated(false),
      ModuleInfoArray(),
      ModuleInfoArraySize(0) {}

SymbolLookup::~SymbolLookup() {}

void SymbolLookup::AddSourceCodeDirectory(const char* pDirectory) {
  OVR_UNUSED(pDirectory);
}

void SymbolLookup::EnableMemoryAllocation(bool enabled) {
  AllowMemoryAllocation = enabled;
}

bool SymbolLookup::Refresh() {
  ModuleListUpdated = false;
  return RefreshModuleList();
}

OVR_DISABLE_MSVC_WARNING(4740) // flow in or out of inline asm code suppresses global optimization
OVR_DISABLE_MSVC_WARNING(4748) // /GS can not protect parameters and local variables from local
// buffer overrun because optimizations are disabled in function

// Requires the thread to be suspended if not the current thread.
size_t SymbolLookup::GetBacktrace(
    void* addressArray[],
    size_t addressArrayCapacity,
    size_t skipCount,
    void* platformThreadContext,
    OVR::ThreadSysId threadSysIdHelp) {
#if defined(OVR_OS_WIN64)
  // The DbgHelp library must be loaded already.
  OVR_ASSERT(sSymUsageCount > 0);

  if (platformThreadContext == nullptr)
    return RtlCaptureStackBackTrace(
        (DWORD)skipCount, (ULONG)addressArrayCapacity, addressArray, nullptr);

  // We need to get the call stack of another thread.
  size_t frameIndex = 0;
  CONTEXT context = {};
  PRUNTIME_FUNCTION pRuntimeFunction;
  ULONG64 imageBase = 0;
  ULONG64 imageBasePrev = 0;
  HANDLE hThread = ConvertThreadSysIdToThreadHandle(threadSysIdHelp);

  if (hThread) {
    // We need to get the full thread context if possible on x64 platforms.
    // See for example https://bugzilla.mozilla.org/show_bug.cgi?id=1120126
    context.ContextFlags = CONTEXT_FULL;

    if (::GetThreadContext(
            hThread,
            &context)) // GetThreadContext will fail if the caller didn't suspend the thread.
    {
      if (context.Rip && (frameIndex < addressArrayCapacity))
        addressArray[frameIndex++] = (void*)(uintptr_t)context.Rip;

      while (context.Rip && (frameIndex < addressArrayCapacity)) {
        imageBasePrev = imageBase;
        pRuntimeFunction =
            (PRUNTIME_FUNCTION)RtlLookupFunctionEntry(context.Rip, &imageBase, nullptr);

        if (pRuntimeFunction) {
          // We have observed a problem in which the thread we are trying to read has exited by the
          // time we get here,
          // despite that we both suspended the thread and successfully read its context. This is
          // contrary to the
          // expectations set here:
          // https://blogs.msdn.microsoft.com/oldnewthing/20150205-00/?p=44743.
          // This code only executes in debugging situations and so should not be an issue for the
          // large majority of
          // end users. It's not safe in general in shipping applications to suspend threads.
          __try {
            VOID* handlerData = nullptr;
            ULONG64 establisherFramePointers[2] = {0, 0};
            RtlVirtualUnwind(
                UNW_FLAG_NHANDLER,
                imageBase,
                context.Rip,
                pRuntimeFunction,
                &context,
                &handlerData,
                establisherFramePointers,
                nullptr);
          } __except (
              GetExceptionCode() == 0x406D1388 /*set thread name*/ ? EXCEPTION_CONTINUE_EXECUTION
                                                                   : EXCEPTION_EXECUTE_HANDLER) {
            break; // If you encounter this under a debugger, just continue and let this code eat
            // the exception.
          }
        } else {
          // This would be viable only if we could rely on the presence of stack frames.
          // context.Rip  = (ULONG64)(*(PULONG64)context.Rsp);
          // context.Rsp += 8;

          // Be safe and just bail.
          context.Rip = 0;
        }

        if (context.Rip && (frameIndex < addressArrayCapacity)) {
          if (skipCount)
            --skipCount;
          else
            addressArray[frameIndex++] = (void*)(uintptr_t)context.Rip;
        }
      }
    }

    FreeThreadHandle(hThread);
  }

  return frameIndex;

#elif defined(OVR_OS_WIN32)
  OVR_UNUSED(threadSysIdHelp);

  OVR::Lock::Locker autoLock(GetSymbolLookupLockPtr());
  size_t frameIndex = 0;

  if (pStackWalk64) {
    CONTEXT context;

    if (platformThreadContext) {
      memcpy(&context, platformThreadContext, sizeof(context));
      context.ContextFlags = CONTEXT_CONTROL;
    } else {
      memset(&context, 0, sizeof(context));
      context.ContextFlags = CONTEXT_CONTROL;

      __asm {
                    mov context.Ebp, EBP
                    mov context.Esp, ESP
                    call GetEIP
                    GetEIP:
                    pop context.Eip
      }
    }

    STACKFRAME64 sf;
    memset(&sf, 0, sizeof(sf));
    sf.AddrPC.Offset = context.Eip;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrStack.Offset = context.Esp;
    sf.AddrStack.Mode = AddrModeFlat;
    sf.AddrFrame.Offset = context.Ebp;
    sf.AddrFrame.Mode = AddrModeFlat;

    const HANDLE hCurrentProcess = ::GetCurrentProcess();
    const HANDLE hCurrentThread = ::GetCurrentThread();

    if (!platformThreadContext) // If we are reading the current thread's call stack then we ignore
      // this current function.
      skipCount++;

    while (frameIndex < addressArrayCapacity) {
      if (!pStackWalk64(
              IMAGE_FILE_MACHINE_I386,
              hCurrentProcess,
              hCurrentThread,
              &sf,
              &context,
              nullptr,
              pSymFunctionTableAccess64,
              pSymGetModuleBase64,
              nullptr))
        break;

      if (sf.AddrFrame.Offset == 0)
        break;

      if (skipCount)
        --skipCount;
      else
        addressArray[frameIndex++] = ((void*)(uintptr_t)sf.AddrPC.Offset);
    }
  }

  return frameIndex;

#elif defined(OVR_OS_APPLE)
  struct StackFrame {
    StackFrame* pParentStackFrame;
    void* pReturnPC;
  };

  void* pInstruction;
  StackFrame* pStackFrame;
  size_t frameIndex = 0;

  if (platformThreadContext) {
#if defined(OVR_CPU_ARM)
    arm_thread_state_t* pThreadState = (arm_thread_state_t*)platformThreadContext;
    pStackFrame = (StackFrame*)pThreadState->__fp;
    pInstruction = (void*)pThreadState->__pc;
#define FrameIsAligned(pStackFrame) ((((uintptr_t)pStackFrame) & 0x1) == 0)
#elif defined(OVR_CPU_X86_64)
    x86_thread_state_t* pThreadState = (x86_thread_state_t*)platformThreadContext;
    pInstruction = (void*)pThreadState->uts.ts64.__rip;
    pStackFrame = (StackFrame*)pThreadState->uts.ts64.__rbp;
#define FrameIsAligned(pStackFrame) ((((uintptr_t)pStackFrame) & 0xf) == 0)
#elif defined(OVR_CPU_X86)
    x86_thread_state_t* pThreadState = (x86_thread_state_t*)platformThreadContext;
    pInstruction = (void*)pThreadState->uts.ts32.__eip;
    pStackFrame = (StackFrame*)pThreadState->uts.ts32.__ebp;
#define FrameIsAligned(pStackFrame) ((((uintptr_t)pStackFrame) & 0xf) == 8)
#endif

    if (frameIndex < addressArrayCapacity)
      addressArray[frameIndex++] = pInstruction;
  } else // Else get the current values...
  {
    pStackFrame = (StackFrame*)__builtin_frame_address(0);
    GetInstructionPointer(pInstruction);
  }

  pthread_t threadSelf = pthread_self();
  void* pCurrentStackBase = pthread_get_stackaddr_np(threadSelf);
  void* pCurrentStackLimit =
      (void*)((uintptr_t)pCurrentStackBase - pthread_get_stacksize_np(threadSelf));
  bool threadIsCurrent = (platformThreadContext == nullptr) ||
      (((void*)pStackFrame > pCurrentStackLimit) && ((void*)pStackFrame <= pCurrentStackBase));
  StackFrame* pStackBase;
  StackFrame* pStackLimit;

  if (threadIsCurrent) {
    pStackBase = (StackFrame*)pCurrentStackBase;
    pStackLimit = (StackFrame*)pCurrentStackLimit;
  } else if (threadSysIdHelp) {
    pthread_t threadHandle = pthread_from_mach_thread_np((mach_port_t)threadSysIdHelp);
    pStackBase = (StackFrame*)pthread_get_stackaddr_np(threadHandle);
    pStackLimit = (StackFrame*)((uintptr_t)pStackBase - pthread_get_stacksize_np(threadHandle));
  } else { // We guess what the limits are.
    pStackBase = pStackFrame + ((384 * 1024) / sizeof(StackFrame));
    pStackLimit = pStackFrame - ((384 * 1024) / sizeof(StackFrame));
  }

  if ((frameIndex < addressArrayCapacity) && pStackFrame && FrameIsAligned(pStackFrame)) {
    addressArray[frameIndex++] = pStackFrame->pReturnPC;

    while (pStackFrame && pStackFrame->pReturnPC && (frameIndex < addressArrayCapacity)) {
      pStackFrame = pStackFrame->pParentStackFrame;

      if (pStackFrame && FrameIsAligned(pStackFrame) && pStackFrame->pReturnPC &&
          (pStackFrame > pStackLimit) && (pStackFrame < pStackBase)) {
        if (skipCount)
          --skipCount;
        else
          addressArray[frameIndex++] = pStackFrame->pReturnPC;
      } else
        break;
    }
  }

  return frameIndex;

#elif defined(OVR_OS_LINUX) && (defined(__LIBUNWIND__) || defined(LIBUNWIND_AVAIL))
  // Libunwind-based solution. Requires installation of libunwind package.
  // Libunwind isn't always safe for threads that are in signal handlers.
  // An approach to get the callstack of another thread is to use signal injection into the target
  // thread.

  OVR_UNUSED(platformThreadContext);
  OVR_UNUSED(threadSysIdHelp);

  size_t frameIndex = 0;
  unw_cursor_t cursor;
  unw_context_t uc;
  unw_word_t ip, sp;

  unw_getcontext(&uc); // This gets the current thread's context. We could alternatively initialize
  // another thread's context with it.
  unw_init_local(&cursor, &uc);

  while ((unw_step(&cursor) > 0) && (frameIndex < addressArrayCapacity)) {
    // We can get the function name here too on some platforms with unw_get_proc_info() and
    // unw_get_proc_name().

    if (skipCount)
      --skipCount;
    else {
      unw_get_reg(&cursor, UNW_REG_IP, &ip);
      addressArray[frameIndex++] = (void*)ip;
    }
  }

  return frameIndex;
#else
  OVR_UNUSED(addressArray);
  OVR_UNUSED(addressArrayCapacity);
  OVR_UNUSED(skipCount);
  OVR_UNUSED(platformThreadContext);
  OVR_UNUSED(threadSysIdHelp);

  return 0;
#endif
}

size_t SymbolLookup::GetBacktraceFromThreadHandle(
    void* addressArray[],
    size_t addressArrayCapacity,
    size_t skipCount,
    OVR::ThreadHandle threadHandle) {
#if defined(OVR_OS_MS)
  size_t count = 0;
  DWORD threadSysId = (DWORD)ConvertThreadHandleToThreadSysId(threadHandle);

  // Compare to 0, compare to the self 'pseudohandle' and compare to the self id.
  if ((threadHandle == OVR_THREADHANDLE_INVALID) || (threadHandle == ::GetCurrentThread()) ||
      (threadSysId == ::GetCurrentThreadId())) // If threadSysId refers to the current thread...
    return GetBacktrace(
        addressArray, addressArrayCapacity, skipCount, nullptr, OVR_THREADSYSID_INVALID);

  // We are working with another thread. We need to suspend it and get its CONTEXT.
  // Suspending other threads is risky, as they may be in some state that cannot be safely blocked.
  DWORD suspendResult =
      ::SuspendThread(threadHandle); // Requires that the handle have THREAD_SUSPEND_RESUME rights.

  if (suspendResult != (DWORD)-1) // Returns previous suspend count, or -1 if failed.
  {
    CONTEXT context = {};
    context.ContextFlags = CONTEXT_CONTROL |
        CONTEXT_INTEGER; // Requires that the handle have THREAD_GET_CONTEXT rights.

    if (::GetThreadContext(
            threadHandle,
            &context)) // This is supposed to ensure that the thread really is stopped.
    {
      count = GetBacktrace(addressArray, addressArrayCapacity, skipCount, &context, threadSysId);
      suspendResult = ::ResumeThread(threadHandle);
      OVR_ASSERT_AND_UNUSED(suspendResult != (DWORD)-1, suspendResult);
    }
  }

  return count;

#elif defined(OVR_OS_APPLE)
  mach_port_t threadSysID =
      pthread_mach_thread_np((pthread_t)threadHandle); // Convert pthread_t to mach thread id.
  return GetBacktraceFromThreadSysId(
      addressArray, addressArrayCapacity, skipCount, (OVR::ThreadSysId)threadSysID);

#elif defined(OVR_OS_LINUX)
  // To do.
  OVR_UNUSED(addressArray);
  OVR_UNUSED(addressArrayCapacity);
  OVR_UNUSED(skipCount);
  OVR_UNUSED(threadHandle);
  return 0;
#endif
}

size_t SymbolLookup::GetBacktraceFromThreadSysId(
    void* addressArray[],
    size_t addressArrayCapacity,
    size_t skipCount,
    OVR::ThreadSysId threadSysId) {
#if defined(OVR_OS_MS)
  OVR::ThreadHandle threadHandle = ConvertThreadSysIdToThreadHandle(threadSysId);
  if (threadHandle) {
    size_t count =
        GetBacktraceFromThreadHandle(addressArray, addressArrayCapacity, skipCount, threadHandle);
    FreeThreadHandle(threadHandle);
    return count;
  }
  return 0;

#elif defined(OVR_OS_APPLE)
  mach_port_t threadCurrent = pthread_mach_thread_np(pthread_self());
  mach_port_t thread = (mach_port_t)threadSysId;

  if (thread == threadCurrent) {
    return GetBacktrace(
        addressArray, addressArrayCapacity, skipCount, nullptr, OVR_THREADSYSID_INVALID);
  } else {
    kern_return_t result = thread_suspend(
        thread); // Do we need to do this if it's an thread who exception is being handled?
    size_t count = 0;

    if (result == KERN_SUCCESS) {
#if defined(OVR_CPU_X86) || defined(OVR_CPU_X86_64)
      x86_thread_state_t threadState;
#elif defined(OVR_CPU_ARM)
      arm_thread_state_t threadState;
#endif
      mach_msg_type_number_t stateCount = MACHINE_THREAD_STATE_COUNT;

      result = thread_get_state(
          thread, MACHINE_THREAD_STATE, (natural_t*)(uintptr_t)&threadState, &stateCount);

      if (result == KERN_SUCCESS)
        count =
            GetBacktrace(addressArray, addressArrayCapacity, skipCount, &threadState, threadSysId);

      thread_resume(thread);

      return count;
    }
  }

  return 0;

#elif defined(OVR_OS_LINUX)
  // To do.
  OVR_UNUSED(addressArray);
  OVR_UNUSED(addressArrayCapacity);
  OVR_UNUSED(skipCount);
  OVR_UNUSED(threadSysId);
  return 0;
#endif
}

// We need to return the required moduleInfoArrayCapacity.
size_t SymbolLookup::GetModuleInfoArray(
    ModuleInfo* pModuleInfoArray,
    size_t moduleInfoArrayCapacity) {
#if defined(OVR_OS_MS)
  size_t moduleCountRequired =
      0; // The count we would copy to pModuleInfoArray if moduleInfoArrayCapacity was enough.
  size_t moduleCount =
      0; // The count we actually copy to pModuleInfoArray. Will be <= moduleInfoArrayCapacity.
  HANDLE hProcess = GetCurrentProcess();
  HMODULE hModuleArray[200];
  DWORD cbNeeded = 0;
  MODULEINFO mi;

  if (EnumProcessModules(hProcess, hModuleArray, sizeof(hModuleArray), &cbNeeded)) {
    moduleCountRequired = ((cbNeeded / sizeof(HMODULE)) < OVR_ARRAY_COUNT(hModuleArray))
        ? (cbNeeded / sizeof(HMODULE))
        : OVR_ARRAY_COUNT(hModuleArray);
    moduleCount = MIN(moduleCountRequired, OVR_ARRAY_COUNT(hModuleArray));
    moduleCount = MIN(moduleCount, moduleInfoArrayCapacity);

    for (size_t i = 0; i < moduleCount; i++) {
      ModuleInfo& moduleInfo = pModuleInfoArray[i];

      memset(&mi, 0, sizeof(mi));
      BOOL result = GetModuleInformation(hProcess, hModuleArray[i], &mi, sizeof(mi));

      if (result) {
        wchar_t pathW[OVR_MAX_PATH];

        moduleInfo.handle = hModuleArray[i];
        moduleInfo.baseAddress = (uintptr_t)mi.lpBaseOfDll;
        moduleInfo.size = mi.SizeOfImage;

        GetModuleFileNameW(hModuleArray[i], pathW, OVR_ARRAY_COUNT(pathW));
        auto requiredUTF8Length = OVR::UTF8Util::Strlcpy(
            moduleInfo.filePath, OVR_ARRAY_COUNT(moduleInfo.filePath), pathW);
        if (requiredUTF8Length < OVR_ARRAY_COUNT(moduleInfo.filePath)) {
          OVR::OVR_strlcpy(
              moduleInfo.name,
              GetFileNameFromPath(moduleInfo.filePath),
              OVR_ARRAY_COUNT(moduleInfo.name));
        } else {
          moduleInfo.filePath[0] = '\0';
          moduleInfo.name[0] = '\0';
        }
      } else {
        moduleInfo.handle = 0;
        moduleInfo.baseAddress = 0;
        moduleInfo.size = 0;
        moduleInfo.filePath[0] = '\0';
        moduleInfo.name[0] = '\0';
      }
    }
  }

  return moduleCountRequired;

#elif defined(OVR_OS_MAC)
  size_t moduleCountRequired = 0;
  size_t moduleCount = 0;

  struct MacModuleInfo // This struct exists solely so we can have a local function within this
  // function..
  {
    static void AddMacModuleInfo(
        ModuleInfo* pModuleInfoArrayL,
        size_t& moduleCountRequiredL,
        size_t& moduleCountL,
        size_t moduleInfoArrayCapacityL,
        const char* pTypeFilterL,
        const char* pModulePath,
        uintptr_t currentSegmentPos,
        const MachHeader* pMachHeader,
        uint64_t offset) {
      for (size_t i = 0; i < pMachHeader->ncmds; i++) {
        const SegmentCommand* pSegmentCommand =
            reinterpret_cast<const SegmentCommand*>(currentSegmentPos);

        if (pSegmentCommand->cmd == kLCSegment) {
          const size_t segnameSize =
              (sizeof(pSegmentCommand->segname) + 1); // +1 so we can have a trailing '\0'.
          char segname[segnameSize];

          memcpy(segname, pSegmentCommand->segname, sizeof(pSegmentCommand->segname));
          segname[segnameSize - 1] = '\0';

          if (!pTypeFilterL || OVR_strncmp(segname, pTypeFilterL, sizeof(segname))) {
            moduleCountRequiredL++;

            if (moduleCountL < moduleInfoArrayCapacityL) {
              ModuleInfo& info = pModuleInfoArrayL[moduleCountL++];

              info.baseAddress = (uint64_t)(pSegmentCommand->vmaddr + offset);
              info.handle = reinterpret_cast<ModuleHandle>((uintptr_t)info.baseAddress);
              info.size = (uint64_t)pSegmentCommand->vmsize;
              OVR_strlcpy(info.filePath, pModulePath, OVR_ARRAY_COUNT(info.filePath));
              OVR_strlcpy(info.name, GetFileNameFromPath(pModulePath), OVR_ARRAY_COUNT(info.name));

              info.permissions[0] = (pSegmentCommand->initprot & VM_PROT_READ) ? 'r' : '-';
              info.permissions[1] = (pSegmentCommand->initprot & VM_PROT_WRITE) ? 'w' : '-';
              info.permissions[2] = (pSegmentCommand->initprot & VM_PROT_EXECUTE) ? 'x' : '-';
              info.permissions[3] = '/';
              info.permissions[4] = (pSegmentCommand->maxprot & VM_PROT_READ) ? 'r' : '-';
              info.permissions[5] = (pSegmentCommand->maxprot & VM_PROT_WRITE) ? 'w' : '-';
              info.permissions[6] = (pSegmentCommand->maxprot & VM_PROT_EXECUTE) ? 'x' : '-';
              info.permissions[7] = '\0';

              OVR_strlcpy(info.type, pSegmentCommand->segname, OVR_ARRAY_COUNT(info.type));
            }
          }
        }

        currentSegmentPos += pSegmentCommand->cmdsize;
      }
    }
  };

  // Iterate dyld_all_image_infos->infoArray
  const struct dyld_all_image_infos* pAllImageInfos = _dyld_get_all_image_infos();

  for (uint32_t i = 0; i < pAllImageInfos->infoArrayCount; i++) {
    const char* pModulePath = pAllImageInfos->infoArray[i].imageFilePath;

    if (pModulePath && *pModulePath) {
      uintptr_t currentSegmentPos = (uintptr_t)pAllImageInfos->infoArray[i].imageLoadAddress;
      const MachHeader* pMachHeader = reinterpret_cast<const MachHeader*>(currentSegmentPos);
      uint64_t offset = (uint64_t)_dyld_get_image_vmaddr_slide(i);

      currentSegmentPos += sizeof(*pMachHeader);

      MacModuleInfo::AddMacModuleInfo(
          pModuleInfoArray,
          moduleCountRequired,
          moduleCount,
          moduleInfoArrayCapacity,
          nullptr /*"__TEXT"*/,
          pModulePath,
          currentSegmentPos,
          pMachHeader,
          offset);
    }
  }

  // In addition to iterating dyld_all_image_infos->infoArray we need to also iterate /usr/lib/dyld
  // entries.
  const MachHeader* pMachHeader = (const MachHeader*)pAllImageInfos->dyldImageLoadAddress;
  uintptr_t currentSegmentPos = (uintptr_t)pMachHeader + sizeof(*pMachHeader);
  char modulePath[OVR_MAX_PATH] = "";
  pid_t pid = getpid();
  int filenameLen =
      proc_regionfilename((int)pid, currentSegmentPos, modulePath, (uint32_t)sizeof(modulePath));

  if (filenameLen > 0)
    MacModuleInfo::AddMacModuleInfo(
        pModuleInfoArray,
        moduleCountRequired,
        moduleCount,
        moduleInfoArrayCapacity,
        "__TEXT",
        modulePath,
        currentSegmentPos,
        pMachHeader,
        0);

  return moduleCountRequired;

#elif defined(OVR_OS_LINUX)
  // One approach is to read /proc/self/maps, which is supported by Linux (though not BSD).
  // Linux glibc dladdr() can tell us what module an arbitrary function address comes from, but
  // can't tell us the list of modules.
  OVR_UNUSED(pModuleInfoArray);
  OVR_UNUSED(moduleInfoArrayCapacity);
  return 0;

#else
  OVR_UNUSED(pModuleInfoArray);
  OVR_UNUSED(moduleInfoArrayCapacity);
  return 0;
#endif
}

size_t SymbolLookup::GetThreadList(
    ThreadHandle* threadHandleArray,
    ThreadSysId* threadSysIdArray,
    size_t threadArrayCapacity) {
  size_t countRequired = 0;
  size_t count = 0;

#if defined(OVR_OS_MS)
  // Print a list of threads.
  DWORD currentProcessId = GetCurrentProcessId();
  HANDLE hThreadSnap = CreateToolhelp32Snapshot(
      TH32CS_SNAPTHREAD,
      currentProcessId); // ICreateToolhelp32Snapshot actually ignores currentProcessId.

  if (hThreadSnap != INVALID_HANDLE_VALUE) {
    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hThreadSnap, &te32)) {
      do {
        if (te32.th32OwnerProcessID == currentProcessId) {
          HANDLE hThread = ConvertThreadSysIdToThreadHandle(te32.th32ThreadID);

          if (hThread) {
            ++countRequired;

            if ((threadHandleArray || threadSysIdArray) && (count < threadArrayCapacity)) {
              if (threadHandleArray)
                threadHandleArray[count] = hThread; // The caller must call CloseHandle on this
              // thread, or call DoneThreadList on the
              // returned array.
              if (threadSysIdArray)
                threadSysIdArray[count] = ConvertThreadHandleToThreadSysId(hThread);
              ++count;
            }

            if (!threadHandleArray) // If we aren't giving this back to the user...
              FreeThreadHandle(hThread);
          }
        }
      } while (Thread32Next(hThreadSnap, &te32));
    }

    CloseHandle(hThreadSnap);
  }

#elif defined(OVR_OS_APPLE)
  mach_port_t taskSelf = mach_task_self();
  thread_act_port_array_t threadArray;
  mach_msg_type_number_t threadCount;

  kern_return_t result = task_threads(taskSelf, &threadArray, &threadCount);

  if (result == KERN_SUCCESS) {
    for (mach_msg_type_number_t i = 0; i < threadCount; i++) {
      ++countRequired;

      if ((threadHandleArray || threadSysIdArray) && (count < threadArrayCapacity)) {
        if (threadHandleArray)
          threadHandleArray[count] = pthread_from_mach_thread_np(threadArray[i]);
        if (threadSysIdArray)
          threadSysIdArray[count] = threadArray[i];
        ++count;
      }
    }

    vm_deallocate(taskSelf, (vm_address_t)threadArray, threadCount * sizeof(thread_act_t));
  }

#elif defined(OVR_OS_LINUX)
  // To do.
  OVR_UNUSED(count);
  OVR_UNUSED(threadHandleArray);
  OVR_UNUSED(threadSysIdArray);
  OVR_UNUSED(threadArrayCapacity);
#endif

  return countRequired;
}

void SymbolLookup::DoneThreadList(
    ThreadHandle* threadHandleArray,
    ThreadSysId* threadSysIdArray,
    size_t threadArrayCount) {
#if defined(OVR_OS_MS)
  for (size_t i = 0; i != threadArrayCount; ++i) {
    if (threadHandleArray[i]) {
      CloseHandle(threadHandleArray[i]);
      threadHandleArray[i] = OVR_THREADHANDLE_INVALID;
    }
  }

  OVR_UNUSED(threadSysIdArray);
#else
  OVR_UNUSED(threadHandleArray);
  OVR_UNUSED(threadSysIdArray);
  OVR_UNUSED(threadArrayCount);
#endif
}

// Writes a given thread's callstack wity symbols to the given output.
// It may not be safe to call this from an exception handler, as sOutput allocates memory.
bool SymbolLookup::ReportThreadCallstack(
    OVR::String& sOutput,
    size_t skipCount,
    ThreadSysId threadSysId) {
  sOutput.Clear();

  if (!threadSysId)
    threadSysId = GetCurrentThreadSysId();

  void* addressArray[64];
  size_t addressCount = GetBacktraceFromThreadSysId(
      addressArray, OVR_ARRAY_COUNT(addressArray), skipCount, threadSysId);

  // Print the header
  char headerBuffer[256];
  char threadSysIdStr[48];
  char stackBaseStr[24];
  char stackLimitStr[24];
  void* pStackBase;
  void* pStackLimit;
  // void*        pStackCurrent;  // Current stack pointer. To do: support reporting this.
  ThreadHandle threadHandle = ConvertThreadSysIdToThreadHandle(threadSysId);
  OVR::GetThreadStackBounds(pStackBase, pStackLimit, threadHandle);

  SprintfThreadSysId(threadSysIdStr, OVR_ARRAY_COUNT(threadSysIdStr), threadSysId);
  SprintfAddress(stackBaseStr, OVR_ARRAY_COUNT(stackBaseStr), pStackBase);
  SprintfAddress(stackLimitStr, OVR_ARRAY_COUNT(stackLimitStr), pStackLimit);

  snprintf(
      headerBuffer,
      OVR_ARRAY_COUNT(headerBuffer),
      "Thread id: %s, stack base: %s, stack limit: %s\n",
      threadSysIdStr,
      stackBaseStr,
      stackLimitStr);

  sOutput += headerBuffer;

  // Print the backtrace info
  char backtraceBuffer[1024]; // Sometimes function symbol names are very long.
  SymbolInfo symbolInfo;
  const char* pModuleName;

  if (addressCount == 0) {
    sOutput += "<Unable to read backtrace>\n";
  } else {
    for (size_t i = 0; i < addressCount; ++i) {
      LookupSymbol((uint64_t)addressArray[i], symbolInfo);

      if (symbolInfo.pModuleInfo && symbolInfo.pModuleInfo->name[0])
        pModuleName = symbolInfo.pModuleInfo->name;
      else
        pModuleName = "(unknown module)";

      char addressStr[24];
      SprintfAddress(addressStr, OVR_ARRAY_COUNT(addressStr), addressArray[i]);

      if (symbolInfo.filePath[0])
        snprintf(
            backtraceBuffer,
            OVR_ARRAY_COUNT(backtraceBuffer),
            "%-2u %-24s %s %s+%d %s:%d\n",
            (unsigned)i,
            pModuleName,
            addressStr,
            symbolInfo.function,
            symbolInfo.functionOffset,
            symbolInfo.filePath,
            symbolInfo.fileLineNumber);
      else
        snprintf(
            backtraceBuffer,
            OVR_ARRAY_COUNT(backtraceBuffer),
            "%-2u %-24s %s %s+%d\n",
            (unsigned)i,
            pModuleName,
            addressStr,
            symbolInfo.function,
            symbolInfo.functionOffset);

      sOutput += backtraceBuffer;
    }
  }

  FreeThreadHandle(threadHandle);

  return (addressCount > 0);
}

// Writes all thread's callstacks with symbols to the given output.
// It may not be safe to call this from an exception handler, as sOutput allocates memory.
bool SymbolLookup::ReportThreadCallstacks(OVR::String& sOutput, size_t skipCount) {
  sOutput.Clear();

  ThreadSysId threadSysIdArray[64];
  size_t threadSysIdCount =
      GetThreadList(nullptr, threadSysIdArray, OVR_ARRAY_COUNT(threadSysIdArray));

  if (threadSysIdCount > OVR_ARRAY_COUNT(threadSysIdArray))
    threadSysIdCount = OVR_ARRAY_COUNT(threadSysIdArray);

  for (size_t i = 0; i < threadSysIdCount; i++) {
    String sTemp;
    ReportThreadCallstack(sTemp, skipCount, threadSysIdArray[i]);
    if (i > 0)
      sOutput += "\n";
    sOutput += sTemp;
  }

  return (threadSysIdCount > 0);
}

bool SymbolLookup::ReportModuleInformation(OVR::String& sOutput) {
  sOutput.Clear();

  RefreshModuleList();

  char backtraceBuffer[1024];

  for (size_t i = 0; i < ModuleInfoArraySize; ++i) {
    snprintf(
        backtraceBuffer,
        OVR_ARRAY_COUNT(backtraceBuffer),
        "Base: 0x%llx Size: 0x%llx Name: '%s' Path: '%s'\n",
        ModuleInfoArray[i].baseAddress,
        ModuleInfoArray[i].size,
        ModuleInfoArray[i].name,
        ModuleInfoArray[i].filePath);
    sOutput += backtraceBuffer;
  }

  return true;
}

bool SymbolLookup::RefreshModuleList() {
  if (!ModuleListUpdated) {
#if defined(OVR_OS_MS)
    OVR::Lock::Locker autoLock(GetSymbolLookupLockPtr());

    // We can't rely on SymRefreshModuleList because it's present in DbgHelp 6.5,
    // which doesn't distribute with Windows 7.

    // Currently we support only refreshing the list once ever. With a little effort we could revise
    // this code to
    // support re-refreshing the list at runtime to account for the possibility that modules have
    // recently been
    // added or removed.
    if (pSymLoadModule64) {
      const size_t requiredCount =
          GetModuleInfoArray(ModuleInfoArray, OVR_ARRAY_COUNT(ModuleInfoArray));
      ModuleInfoArraySize = MIN(requiredCount, OVR_ARRAY_COUNT(ModuleInfoArray));

      HANDLE hProcess = GetCurrentProcess();

      for (size_t i = 0; i < ModuleInfoArraySize; i++)
        pSymLoadModule64(
            hProcess,
            nullptr,
            ModuleInfoArray[i].filePath,
            nullptr,
            ModuleInfoArray[i].baseAddress,
            (DWORD)ModuleInfoArray[i].size);

      ModuleListUpdated = true;
    }
#else
    const size_t requiredCount =
        GetModuleInfoArray(ModuleInfoArray, OVR_ARRAY_COUNT(ModuleInfoArray));
    ModuleInfoArraySize = MIN(requiredCount, OVR_ARRAY_COUNT(ModuleInfoArray));
    ModuleListUpdated = true;
#endif
  }

  return true;
}

bool SymbolLookup::LookupSymbol(uint64_t address, SymbolInfo& symbolInfo) {
  return LookupSymbols(&address, &symbolInfo, 1);
}

bool SymbolLookup::LookupSymbols(
    uint64_t* addressArray,
    SymbolInfo* pSymbolInfoArray,
    size_t arraySize) {
  bool success = false;

  if (!ModuleListUpdated) {
    RefreshModuleList();
  }

#if defined(OVR_OS_MS)
  OVR::Lock::Locker autoLock(GetSymbolLookupLockPtr());

  union SYMBOL_INFO_UNION {
    SYMBOL_INFO msSymbolInfo;
    char suffixPadding[sizeof(SYMBOL_INFO) + 1024];
  };

  for (size_t i = 0; i < arraySize; i++) {
    uint64_t& address = addressArray[i];
    SymbolInfo& symbolInfo = pSymbolInfoArray[i];

    // Copy the address and ModuleInfo
    symbolInfo.address = addressArray[i];
    symbolInfo.pModuleInfo = GetModuleInfoForAddress(
        address); // We could also use siu.msSymbolInfo.ModBase to get the module slightly faster.

    // Get the function/offset.
    SYMBOL_INFO_UNION siu;
    memset(&siu, 0, sizeof(siu));
    siu.msSymbolInfo.SizeOfStruct = sizeof(siu.msSymbolInfo);
    siu.msSymbolInfo.MaxNameLen = sizeof(siu.suffixPadding) - sizeof(SYMBOL_INFO) +
        1; // +1 because SYMBOL_INFO itself has Name[1].

    HANDLE hProcess = GetCurrentProcess();
    DWORD64 displacement64 = 0;
    bool bResult = (pSymFromAddr != nullptr) &&
        (pSymFromAddr(hProcess, address, &displacement64, &siu.msSymbolInfo) != FALSE);

    if (bResult) {
      success = true;
      symbolInfo.size = siu.msSymbolInfo.Size;
      OVR_strlcpy(symbolInfo.function, siu.msSymbolInfo.Name, OVR_ARRAY_COUNT(symbolInfo.function));
      symbolInfo.functionOffset = (int32_t)displacement64;
    } else {
      symbolInfo.size = kMISizeInvalid;
      symbolInfo.function[0] = 0;
      symbolInfo.functionOffset = kMIFunctionOffsetInvalid;
    }

    // Get the file/line
    IMAGEHLP_LINE64 iLine64;
    DWORD displacement = 0;
    memset(&iLine64, 0, sizeof(iLine64));
    iLine64.SizeOfStruct = sizeof(iLine64);

    bResult = (pSymGetLineFromAddr64 != nullptr) &&
        (pSymGetLineFromAddr64(hProcess, address, &displacement, &iLine64) != FALSE);

    if (bResult) {
      success = true;
      OVR_strlcpy(symbolInfo.filePath, iLine64.FileName, OVR_ARRAY_COUNT(symbolInfo.filePath));
      symbolInfo.fileLineNumber = (int32_t)iLine64.LineNumber;
    } else {
      symbolInfo.filePath[0] = 0;
      symbolInfo.fileLineNumber = kMILineNumberInvalid;
    }

    // To do: get the source code when possible. We need to use the user-registered directory paths
    // and the symbolInfo.filePath
    // and find the given file in the tree(s), then open the file and find the
    // symbolInfo.fileLineNumber line (and surrounding lines).
    // symbolInfo.sourceCode[1024]
    symbolInfo.sourceCode[0] = '\0';
  }

#elif defined(OVR_OS_APPLE)
  // Apple has an internal CoreSymbolication library which could help with this.
  // Third party implementations of the CoreSymbolication header are available and could be used
  // to get file/line info better than other means. It used Objective C, so we'll need a .m or .mm
  // file.

  memset(pSymbolInfoArray, 0, arraySize * sizeof(SymbolInfo));

  for (size_t i = 0; i < arraySize; i++) {
    pSymbolInfoArray[i].address = addressArray[i];
    pSymbolInfoArray[i].pModuleInfo = GetModuleInfoForAddress(addressArray[i]);
  }

  // Problem: backtrace_symbols allocates memory from malloc. If you got into a SIGSEGV due to
  // malloc arena corruption (quite common) you will likely fault in backtrace_symbols.
  // To do: Use allowMemoryAllocation here.

#if (OVR_PTR_SIZE == 4)
  // backtrace_symbols takes a void* array, but we have a uint64_t array. So for 32 bit we
  // need to convert the 64 bit array to 32 bit temporarily for the backtrace_symbols call.
  void* ptr32Array[256]; // To do: Remove this limit.
  for (size_t i = 0, iEnd = MIN(arraySize, OVR_ARRAY_COUNT(ptr32Array)); i < iEnd; i++)
    ptr32Array[i] = reinterpret_cast<void*>(addressArray[i]);
  char** symbolArray = backtrace_symbols(reinterpret_cast<void**>(ptr32Array), (int)arraySize);
#else
  char** symbolArray = backtrace_symbols(reinterpret_cast<void**>(addressArray), (int)arraySize);
#endif

  if (symbolArray) {
    success = true;

    for (size_t i = 0; i < arraySize; i++) {
      // Generates a string like this: "0 OculusWorldDemo 0x000000010000cfd5
      // _ZN18OculusWorldDemoApp9OnStartupEiPPKc + 213"
      static_assert(
          OVR_ARRAY_COUNT(pSymbolInfoArray[i].function) == 384,
          "Need to change the string format size below");

      sscanf(
          symbolArray[i],
          "%*d %*s %*x %384s + %d",
          pSymbolInfoArray[i].function,
          &pSymbolInfoArray[i].functionOffset);

      if (AllowMemoryAllocation) {
        int status = 0;
        char* strDemangled =
            abi::__cxa_demangle(pSymbolInfoArray[i].function, nullptr, nullptr, &status);

        if (strDemangled) {
          OVR_strlcpy(
              pSymbolInfoArray[i].function,
              strDemangled,
              OVR_ARRAY_COUNT(pSymbolInfoArray[i].function));
          free(strDemangled);
        }
      }
    }

    free(symbolArray);
  }

  // To consider: use CoreSybolication to get file/line info instead. atos is a bit slow and
  // cumbersome.
  // https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man1/atos.1.html
  // atos -p <pid> <addr> <addr> ...
  // atos -o <binary image path> -l <load-address> <addr> <addr> ...
  // Generates output like this: "OVR::CreateException(OVR::CreateExceptionType) (in
  // OculusWorldDemo) (ExceptionHandler.cpp:598)"
  for (size_t i = 0; i < arraySize; i++) {
    struct stat statStruct;

    if (pSymbolInfoArray[i].pModuleInfo && pSymbolInfoArray[i].pModuleInfo->filePath[0] &&
        (stat(pSymbolInfoArray[i].pModuleInfo->filePath, &statStruct) == 0)) {
      char command[PATH_MAX * 2]; // Problem: We can't unilaterally use pSymbolInfoArray[0] for all
      // addresses. We need to match addresses to the corresponding
      // modules.
      // FIXME: atos is moving. Should start with 'xcrun atos'.
      snprintf(
          command,
          OVR_ARRAY_COUNT(command),
          "atos -o %s -l 0x%llx 0x%llx",
          pSymbolInfoArray[i].pModuleInfo->filePath,
          (int64_t)pSymbolInfoArray[i].pModuleInfo->baseAddress,
          (int64_t)pSymbolInfoArray[i].address);

      char output[512];
      if (SpawnShellCommand(command, output, OVR_ARRAY_COUNT(output)) != (size_t)-1) {
        char* pLastOpenParen = strrchr(output, '(');
        char* pColon = strrchr(output, ':');

        if (pLastOpenParen && (pColon > pLastOpenParen)) {
          *pColon = '\0';
          OVR_strlcpy(
              pSymbolInfoArray[i].filePath,
              pLastOpenParen + 1,
              OVR_ARRAY_COUNT(pSymbolInfoArray[i].filePath));
        }
      }
    }
  }

#elif defined(OVR_OS_LINUX)
  // We can use libunwind's unw_get_proc_name to try to get function name info. It can work
  // regardless of relocation.
  // Use backtrace_symbols and addr2line. Need to watch out for module load-time relocation.
  // Ned to pass the -rdynamic flag to the linker. It will cause the linker to out in the link
  // tables the name of all the none static functions in your code, not just the exported ones.
  OVR_UNUSED(addressArray);
  OVR_UNUSED(pSymbolInfoArray);
  OVR_UNUSED(arraySize);
#endif

  return success;
}

const ModuleInfo* SymbolLookup::GetModuleInfoForAddress(uint64_t address) {
  // This is a linear seach. To consider: it would be significantly faster to search by
  // address if we ordered it by base address and did a binary search.
  for (size_t i = 0; i < ModuleInfoArraySize; ++i) {
    const ModuleInfo& mi = ModuleInfoArray[i];

    if ((mi.baseAddress <= address) && (address < (mi.baseAddress + mi.size)))
      return &mi;
  }

  return nullptr;
}

ExceptionInfo::ExceptionInfo()
    : time(),
      timeVal(0),
      backtrace(),
      backtraceCount(0),
      threadHandle(OVR_THREADHANDLE_INVALID),
      threadSysId(OVR_THREADSYSID_INVALID),
      threadName(),
      pExceptionInstructionAddress(nullptr),
      pExceptionMemoryAddress(nullptr),
      cpuContext(),
      exceptionDescription(),
      symbolInfo()
#if defined(OVR_OS_MS)
      ,
      exceptionRecord()
#elif defined(OVR_OS_APPLE)
      ,
      exceptionType(0),
      cpuExceptionId(0),
      cpuExceptionIdError(0),
      machExceptionData(),
      machExceptionDataCount(0)
#endif
{
}

ExceptionHandler::ExceptionHandler()
    : enabled(false),
      reportPrivacyEnabled(true),
      exceptionResponse(kERHandle),
      exceptionListener(nullptr),
      exceptionListenerUserValue(0),
      appDescription(),
      codeBasePathArray(),
      reportFilePath(),
      minidumpInfoLevel(kMILMedium),
      miniDumpFilePath(),
      LogFile(nullptr),
      scratchBuffer(),
      exceptionOccurred(false),
      reportFilePathActual(),
      minidumpFilePathActual(),
      terminateReturnValue(0),
      exceptionInfo()
#if defined(OVR_OS_MS)
      ,
      vectoredHandle(nullptr),
      previousFilter(nullptr),
      pExceptionPointers(nullptr)
#elif defined(OVR_OS_MAC)
      ,
      machHandlerInitialized(false),
      machExceptionPort(0),
      machExceptionPortsSaved(),
      machThreadShouldContinue(false),
      machThreadExecuting(false),
      machThread((pthread_t)OVR_THREADHANDLE_INVALID)
#endif
{
  SetExceptionPaths("default", "default");
}

void ExceptionHandler::GetCrashDumpDirectoryFromNames(
    char* path,
    const char* organizationName,
    const char* ApplicationName) {
  ExceptionHandler::GetCrashDumpDirectory(path, OVR_MAX_PATH);
  OVR_strlcat(path, organizationName, OVR_MAX_PATH);

// make the organization folder if necessary
#ifdef OVR_OS_MS
  WCHAR wpath[OVR_MAX_PATH];
  auto requiredUTF8Length = OVR::UTF8Util::Strlcpy(wpath, OVR_ARRAY_COUNT(wpath), path);
  // XXX error handling/logging?
  if (requiredUTF8Length < OVR_ARRAY_COUNT(wpath))
    CreateDirectoryW(wpath, NULL);
#else
  mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

#ifdef OVR_OS_MS
  const char* separator = "\\";
#else
  const char* separator = "/";
#endif
  OVR_strlcat(path, separator, OVR_MAX_PATH);
  OVR_strlcat(path, ApplicationName, OVR_MAX_PATH);
#ifdef OVR_OS_MS
  requiredUTF8Length = OVR::UTF8Util::Strlcpy(wpath, OVR_ARRAY_COUNT(wpath), path);
  // XXX error handling/logging?
  if (requiredUTF8Length < OVR_ARRAY_COUNT(wpath))
    CreateDirectoryW(wpath, NULL);
#else
  mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
  OVR_strlcat(path, separator, OVR_MAX_PATH);
}

void ExceptionHandler::SetPathsFromNames(
    const char* organizationName,
    const char* ApplicationName,
    const char* exceptionFormat,
    const char* minidumpFormat) {
  char exceptionPath[OVR_MAX_PATH];
  char miniDumpPath[OVR_MAX_PATH];
  ExceptionHandler::GetCrashDumpDirectoryFromNames(
      exceptionPath, organizationName, ApplicationName);
  ExceptionHandler::GetCrashDumpDirectoryFromNames(miniDumpPath, organizationName, ApplicationName);

  OVR::OVR_strlcat(exceptionPath, exceptionFormat, OVR_MAX_PATH);
  OVR::OVR_strlcat(miniDumpPath, minidumpFormat, OVR_MAX_PATH);

  SetExceptionPaths(exceptionPath, miniDumpPath);
}

ExceptionHandler::~ExceptionHandler() {
  if (enabled) {
    Enable(false);
  }
}

size_t ExceptionHandler::GetCrashDumpDirectory(char* directoryPath, size_t directoryPathCapacity) {
#if defined(OVR_OS_MS)
  wchar_t pathW[OVR_MAX_PATH];
  HRESULT hr = SHGetFolderPathW(
      nullptr,
      CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
      nullptr,
      SHGFP_TYPE_CURRENT,
      pathW); // Expects pathW to be MAX_PATH. The returned path does not include a trailing
  // backslash.

  if (SUCCEEDED(hr)) {
    auto requiredUTF8Length = OVR::UTF8Util::Strlcpy(directoryPath, directoryPathCapacity, pathW);
    if (requiredUTF8Length < directoryPathCapacity) // We need space for a trailing path separator.
    {
      if ((requiredUTF8Length == 0) ||
          (directoryPath[requiredUTF8Length - 1] != '\\')) // If there is no trailing \ char...
      {
        OVR::OVR_strlcat(directoryPath, "\\", directoryPathCapacity);
        requiredUTF8Length++;
      }
    }

    return requiredUTF8Length; // Returns the required strlen.
  }

#elif defined(OVR_OS_MAC)
  // This is the same location that Apple puts its OS-generated .crash files.
  const char* home = getenv("HOME");
  size_t requiredStrlen = snprintf(
      directoryPath,
      directoryPathCapacity,
      "%s/Library/Logs/DiagnosticReports/",
      home ? home : "/Users/Shared/Logs/DiagnosticReports/");
  // To do: create the directory if it doesn't already exist.
  return requiredStrlen;

#elif defined(OVR_OS_UNIX)
  const char* home = getenv("HOME");
  size_t requiredStrlen =
      snprintf(directoryPath, directoryPathCapacity, "%s/Library/", home ? home : "/Users/Shared/");
  // To do: create the directory if it doesn't already exist.
  return requiredStrlen;
#endif

  return 0;
}

#if defined(OVR_OS_MS)

static ExceptionHandler* sExceptionHandler = nullptr;

unsigned WINAPI ExceptionHandler::ExceptionHandlerThreadExec(void* callingHandler) {
  ExceptionHandler* caller = reinterpret_cast<ExceptionHandler*>(callingHandler);
  if (caller->miniDumpFilePath[0])
    caller->WriteMiniDump();

  if (caller->reportFilePath[0])
    caller->WriteReport("Exception");

  if (caller->exceptionListener)
    caller->exceptionListener->HandleException(
        caller->exceptionListenerUserValue,
        caller,
        &caller->exceptionInfo,
        caller->reportFilePathActual);
  return 1;
}

LONG WINAPI Win32ExceptionFilter(LPEXCEPTION_POINTERS pExceptionPointersArg) {
  if (sExceptionHandler)
    return (LONG)sExceptionHandler->ExceptionFilter(pExceptionPointersArg);
  return EXCEPTION_CONTINUE_SEARCH;
}

LONG ExceptionHandler::ExceptionFilter(LPEXCEPTION_POINTERS pExceptionPointersArg) {
  if (pauseCount)
    return EXCEPTION_CONTINUE_SEARCH;

  // Exception codes < 0x80000000 are not true exceptions but rather are debugger notifications.
  // They include DBG_TERMINATE_THREAD,
  // DBG_TERMINATE_PROCESS, DBG_CONTROL_BREAK, DBG_COMMAND_EXCEPTION, DBG_CONTROL_C,
  // DBG_PRINTEXCEPTION_C, DBG_RIPEXCEPTION,
  // and 0x406d1388 (thread named, http://blogs.msdn.com/b/stevejs/archive/2005/12/19/505815.aspx).

  if (pExceptionPointersArg->ExceptionRecord->ExceptionCode < 0x80000000)
    return EXCEPTION_CONTINUE_SEARCH;

  // VC++ C++ exceptions use code 0xe06d7363 ('Emsc')
  // http://support.microsoft.com/kb/185294
  // http://blogs.msdn.com/b/oldnewthing/archive/2010/07/30/10044061.aspx
  if (pExceptionPointersArg->ExceptionRecord->ExceptionCode == 0xe06d7363)
    return EXCEPTION_CONTINUE_SEARCH;

  unsigned int tmp_zero = 0;
  // If we can successfully change it from 0 to 1.
  if (handlingBusy.compare_exchange_strong(tmp_zero, 1, std::memory_order_acquire)) {
    exceptionOccurred = true;

    SymbolLookup::Initialize();

    this->pExceptionPointers = pExceptionPointersArg;

    // Disable the handler while we do this processing.
    ULONG result = RemoveVectoredExceptionHandler(vectoredHandle);
    OVR_ASSERT_AND_UNUSED(result != 0, result);

    // Time
    exceptionInfo.timeVal = time(nullptr);
    exceptionInfo.time = *gmtime(&exceptionInfo.timeVal);

    // Thread id
    // This is the thread id of the current thread and not the exception thread.
    if (!DuplicateHandle(
            GetCurrentProcess(),
            GetCurrentThread(),
            GetCurrentProcess(),
            &exceptionInfo.threadHandle,
            0,
            true,
            DUPLICATE_SAME_ACCESS))
      exceptionInfo.threadHandle = 0;
    exceptionInfo.threadSysId = ConvertThreadHandleToThreadSysId(exceptionInfo.threadHandle);

    OVR::GetThreadName(
        exceptionInfo.threadHandle,
        exceptionInfo.threadName,
        OVR_ARRAY_COUNT(exceptionInfo.threadName));

    // Backtraces
    exceptionInfo.backtraceCount = symbolLookup.GetBacktrace(
        exceptionInfo.backtrace,
        OVR_ARRAY_COUNT(exceptionInfo.backtrace),
        0,
        nullptr,
        OVR_THREADSYSID_INVALID); // Get current thread backtrace.

    // Context
    exceptionInfo.cpuContext = *pExceptionPointersArg->ContextRecord;
    exceptionInfo.exceptionRecord = *pExceptionPointersArg->ExceptionRecord;
    exceptionInfo.pExceptionInstructionAddress = exceptionInfo.exceptionRecord.ExceptionAddress;
    if ((exceptionInfo.exceptionRecord.ExceptionCode == EXCEPTION_ACCESS_VIOLATION) ||
        (exceptionInfo.exceptionRecord.ExceptionCode == EXCEPTION_IN_PAGE_ERROR))
      exceptionInfo.pExceptionMemoryAddress =
          (void*)exceptionInfo.exceptionRecord.ExceptionInformation[1]; // ExceptionInformation[0]
    // indicates if it was a
    // read (0), write (1), or
    // data execution attempt
    // (8).
    else
      exceptionInfo.pExceptionMemoryAddress =
          pExceptionPointersArg->ExceptionRecord->ExceptionAddress;

    WriteExceptionDescription();

    if (pExceptionPointersArg->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {
      unsigned int IdValue;

      void* ThreadHandle = (HANDLE)_beginthreadex(
          0, (unsigned)128 * 1024, ExceptionHandlerThreadExec, this, 0, (unsigned*)&IdValue);
      WaitForSingleObject(ThreadHandle, INFINITE);
      CloseHandle(ThreadHandle);

    } else {
      if (reportFilePath[0])
        WriteReport("Exception");

      if (miniDumpFilePath[0])
        WriteMiniDump();

      if (exceptionListener)
        exceptionListener->HandleException(
            exceptionListenerUserValue, this, &exceptionInfo, reportFilePathActual);
    }
    if (exceptionInfo.threadHandle) {
      CloseHandle(exceptionInfo.threadHandle);
      exceptionInfo.threadHandle = 0;
    }

    SymbolLookup::Shutdown();

    // Restore the handler that we temporarily disabled above.
    vectoredHandle = AddVectoredExceptionHandler(1, Win32ExceptionFilter);

    handlingBusy.store(0, std::memory_order_release);
  }

  if (exceptionResponse == ExceptionHandler::kERContinue)
    return EXCEPTION_CONTINUE_EXECUTION;
  else if (exceptionResponse == ExceptionHandler::kERHandle)
    return EXCEPTION_EXECUTE_HANDLER;
  else if (exceptionResponse == ExceptionHandler::kERTerminate) {
    TerminateProcess(GetCurrentProcess(), (UINT)terminateReturnValue);
    return terminateReturnValue;
  } else if (exceptionResponse == ExceptionHandler::kERThrow)
    return EXCEPTION_CONTINUE_SEARCH;

  // kERDefault
  return EXCEPTION_EXECUTE_HANDLER;
}

#endif // defined(OVR_OS_MS)

#if defined(OVR_OS_APPLE)
// http://www.opensource.apple.com/source/xnu/xnu-2050.22.13/
// http://www.opensource.apple.com/source/xnu/xnu-2050.22.13/osfmk/man/
// http://www.opensource.apple.com/source/Libc/Libc-825.26/
// https://mikeash.com/pyblog/friday-qa-2013-01-11-mach-exception-handlers.html

void* ExceptionHandler::MachHandlerThreadFunction() {
  __Request__mach_exception_raise_state_identity_t msg;
  __Reply__mach_exception_raise_state_identity_t reply;
  mach_msg_return_t result;

  machThreadExecuting = true;
  pthread_setname_np("ExceptionHandler");

  while (machThreadShouldContinue) {
    mach_msg_option_t options = MACH_RCV_MSG | MACH_RCV_LARGE;
    natural_t timeout = 0; // Would be better to support a non-zero time.

    if (timeout)
      options |= MACH_RCV_TIMEOUT;

    result =
        mach_msg(&msg.Head, options, 0, sizeof(msg), machExceptionPort, timeout, MACH_PORT_NULL);

    if (msg.Head.msgh_id != sMachCancelMessageType) {
      if (result == MACH_MSG_SUCCESS) {
        if (mach_exc_server_OVR(&msg.Head, &reply.Head) ==
            0) // This will call our HandleMachException function.
          result = ~MACH_MSG_SUCCESS;
      }

      // Send the reply
      if (result == MACH_MSG_SUCCESS) {
        result = mach_msg(
            &reply.Head,
            MACH_SEND_MSG,
            reply.Head.msgh_size,
            0,
            MACH_PORT_NULL,
            MACH_MSG_TIMEOUT_NONE,
            MACH_PORT_NULL);

        if (result != MACH_MSG_SUCCESS) {
          // Failure.
        }
      }
    }
  }

  machThreadExecuting = false;

  return nullptr;
}

kern_return_t ExceptionHandler::HandleMachException(
    mach_port_t /*machPort*/,
    mach_port_t threadSysId,
    mach_port_t machTask,
    exception_type_t machExceptionType,
    mach_exception_data_type_t* pMachExceptionData,
    mach_msg_type_number_t exceptionDataCount,
    int* /*pMachExceptionFlavor*/,
    thread_state_t threadStatePrev,
    mach_msg_type_number_t /*threadStatePrevCount*/,
    thread_state_t /*threadStateNew*/,
    mach_msg_type_number_t* /*pThreadStateNewCount*/) {
  // We don't want to handle exceptions for other processes.
  if (machTask != mach_task_self())
    return ForwardMachException(
        threadSysId, machTask, machExceptionType, pMachExceptionData, exceptionDataCount);

  unsigned int tmp_zero = 0;

  if (handlingBusy.compare_exchange_strong(
          tmp_zero, 1, std::memory_order_acquire)) // If we can successfully change it from 0 to 1.
  {
    exceptionOccurred = true;

    // Disable the handler while we do this processing.
    // To do.

    // Time
    exceptionInfo.timeVal = time(nullptr);
    exceptionInfo.time = *gmtime(&exceptionInfo.timeVal);

    // Thread id
    exceptionInfo.threadHandle = pthread_from_mach_thread_np(threadSysId);
    exceptionInfo.threadSysId = threadSysId;
    pthread_getname_np(
        (pthread_t)exceptionInfo.threadHandle,
        exceptionInfo.threadName,
        sizeof(exceptionInfo.threadName));

    // Backtraces
    exceptionInfo.backtraceCount = symbolLookup.GetBacktraceFromThreadSysId(
        exceptionInfo.backtrace, OVR_ARRAY_COUNT(exceptionInfo.backtrace), 0, threadSysId);

// Context
#if defined(OVR_CPU_X86) || defined(OVR_CPU_X86_64)
    // We can read x86_THREAD_STATE directly fromk threadStatePrev.
    exceptionInfo.cpuContext.threadState = *reinterpret_cast<x86_thread_state_t*>(threadStatePrev);

    mach_msg_type_number_t stateCount = x86_FLOAT_STATE_COUNT;
    thread_get_state(
        threadSysId,
        x86_FLOAT_STATE,
        (natural_t*)&exceptionInfo.cpuContext.floatState,
        &stateCount);

    stateCount = x86_DEBUG_STATE_COUNT;
    thread_get_state(
        threadSysId,
        x86_DEBUG_STATE,
        (natural_t*)&exceptionInfo.cpuContext.debugState,
        &stateCount);

    stateCount = x86_AVX_STATE_COUNT;
    thread_get_state(
        threadSysId, x86_AVX_STATE, (natural_t*)&exceptionInfo.cpuContext.avxState, &stateCount);

    stateCount = x86_EXCEPTION_STATE_COUNT;
    thread_get_state(
        threadSysId,
        x86_EXCEPTION_STATE,
        (natural_t*)&exceptionInfo.cpuContext.exceptionState,
        &stateCount);

#if defined(OVR_CPU_X86)
    exceptionInfo.pExceptionInstructionAddress =
        (void*)exceptionInfo.cpuContext.threadState.uts.ts32.__eip;
    exceptionInfo.pExceptionMemoryAddress =
        (void*)exceptionInfo.cpuContext.exceptionState.ues.es32.__faultvaddr;
    exceptionInfo.cpuExceptionId = exceptionInfo.cpuContext.exceptionState.ues.es32.__trapno;
    exceptionInfo.cpuExceptionIdError = exceptionInfo.cpuContext.exceptionState.ues.es32.__err;
#else
    exceptionInfo.pExceptionInstructionAddress =
        (void*)exceptionInfo.cpuContext.threadState.uts.ts64.__rip;
    exceptionInfo.pExceptionMemoryAddress =
        (void*)exceptionInfo.cpuContext.exceptionState.ues.es64.__faultvaddr;
    exceptionInfo.cpuExceptionId = exceptionInfo.cpuContext.exceptionState.ues.es64.__trapno;
    exceptionInfo.cpuExceptionIdError = exceptionInfo.cpuContext.exceptionState.ues.es64.__err;
#endif
#endif

    exceptionInfo.exceptionType = machExceptionType;

    exceptionInfo.machExceptionDataCount =
        MIN(exceptionDataCount, OVR_ARRAY_COUNT(exceptionInfo.machExceptionData));
    for (int i = 0; i < exceptionInfo.machExceptionDataCount; i++)
      exceptionInfo.machExceptionData[i] = pMachExceptionData[i];

    WriteExceptionDescription();

    if (reportFilePath[0])
      WriteReport("Exception");

    if (miniDumpFilePath[0])
      WriteMiniDump();

    if (exceptionListener)
      exceptionListener->HandleException(
          exceptionListenerUserValue, this, &exceptionInfo, reportFilePathActual);

    // Re-restore the handler.
    // To do.

    handlingBusy.store(0, std::memory_order_release);
  }

  kern_return_t result =
      KERN_FAILURE; // By default pass on the exception to another handler after we are done here.

  if (exceptionResponse == ExceptionHandler::kERContinue)
    result = KERN_SUCCESS; // This will trigger a re-execution of the function.
  else if (exceptionResponse == ExceptionHandler::kERTerminate)
    ::exit(terminateReturnValue);
  else if (exceptionResponse == ExceptionHandler::kERThrow)
    ForwardMachException(
        threadSysId, machTask, machExceptionType, pMachExceptionData, exceptionDataCount);
  else if (exceptionResponse == ExceptionHandler::kERDefault)
    ::exit(terminateReturnValue);

  // kERHandle
  return result;
}

bool ExceptionHandler::InitMachExceptionHandler() {
  if (!machHandlerInitialized) {
    mach_port_t machTaskSelf = mach_task_self();
    kern_return_t result = MACH_MSG_SUCCESS;
    exception_mask_t mask =
        EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC | EXC_MASK_CRASH;

    if (machExceptionPort == MACH_PORT_NULL) {
      result = mach_port_allocate(machTaskSelf, MACH_PORT_RIGHT_RECEIVE, &machExceptionPort);

      if (result == MACH_MSG_SUCCESS) {
        result = mach_port_insert_right(
            machTaskSelf, machExceptionPort, machExceptionPort, MACH_MSG_TYPE_MAKE_SEND);

        if (result == MACH_MSG_SUCCESS)
          result = task_get_exception_ports(
              machTaskSelf,
              mask,
              machExceptionPortsSaved.masks,
              &machExceptionPortsSaved.count,
              machExceptionPortsSaved.ports,
              machExceptionPortsSaved.behaviors,
              machExceptionPortsSaved.flavors);
      }
    }

    if (result == MACH_MSG_SUCCESS) {
      result = task_set_exception_ports(
          machTaskSelf,
          mask,
          machExceptionPort,
          EXCEPTION_STATE_IDENTITY | MACH_EXCEPTION_CODES,
          MACHINE_THREAD_STATE);

      if (result == MACH_MSG_SUCCESS) {
        machThreadShouldContinue = true;

        pthread_attr_t attr;
        pthread_attr_init(&attr);

        result = pthread_create(&machThread, &attr, MachHandlerThreadFunctionStatic, (void*)this);
        pthread_attr_destroy(&attr);

        machHandlerInitialized = (result == 0);
      }
    }

    if (!machHandlerInitialized)
      ShutdownMachExceptionHandler();
  }

  return machHandlerInitialized;
}

void ExceptionHandler::ShutdownMachExceptionHandler() {
  if (machThreadExecuting) {
    machThreadShouldContinue = false; // Tell it to stop.

    // Cancel the current exception handler thread (which is probably blocking in a call to
    // mach_msg) by sending it a cencel message.
    struct CancelMessage {
      mach_msg_header_t msgHeader;
    };

    CancelMessage msg;
    memset(&msg.msgHeader, 0, sizeof(CancelMessage));
    msg.msgHeader.msgh_id = sMachCancelMessageType;
    msg.msgHeader.msgh_size = sizeof(CancelMessage);
    msg.msgHeader.msgh_bits = MACH_MSGH_BITS_REMOTE(MACH_MSG_TYPE_MAKE_SEND);
    msg.msgHeader.msgh_remote_port = machExceptionPort;
    msg.msgHeader.msgh_local_port = MACH_PORT_NULL;

    mach_msg_return_t result = mach_msg(
        &msg.msgHeader,
        MACH_SEND_MSG,
        msg.msgHeader.msgh_size,
        0,
        MACH_PORT_NULL,
        MACH_MSG_TIMEOUT_NONE,
        MACH_PORT_NULL);

    if (result == MACH_MSG_SUCCESS) {
      const time_t secondsLater = time(NULL) + 4;

      while (machThreadExecuting && (time(NULL) < secondsLater)) {
        timespec ts = {0, 1000000000};
        nanosleep(&ts, nullptr);
      }
    }

    void* joinResult = nullptr;
    pthread_join(machThread, &joinResult);
    machThread = 0;
  }

  if (machExceptionPort != MACH_PORT_NULL) {
    // Restore the previous ports
    kern_return_t result = KERN_SUCCESS;
    mach_port_t machTaskSelf = mach_task_self();

    for (unsigned i = 0; (i < machExceptionPortsSaved.count) && (result == KERN_SUCCESS); i++) {
      result = task_set_exception_ports(
          machTaskSelf,
          machExceptionPortsSaved.masks[i],
          machExceptionPortsSaved.ports[i],
          machExceptionPortsSaved.behaviors[i],
          machExceptionPortsSaved.flavors[i]);
    }

    mach_port_deallocate(machTaskSelf, machExceptionPort);
    machExceptionPort = MACH_PORT_NULL;
  }

  machHandlerInitialized = false;
}

kern_return_t ExceptionHandler::ForwardMachException(
    mach_port_t thread,
    mach_port_t task,
    exception_type_t exceptionType,
    mach_exception_data_t pMachExceptionData,
    mach_msg_type_number_t exceptionDataCount) {
  kern_return_t result = KERN_FAILURE;
  mach_msg_type_number_t i;

  for (i = 0; i < machExceptionPortsSaved.count; i++) {
    if (machExceptionPortsSaved.masks[i] & (1 << exceptionType))
      break;
  }

  if (i < machExceptionPortsSaved.count) {
    mach_port_t port = machExceptionPortsSaved.ports[i];
    exception_behavior_t behavior = machExceptionPortsSaved.behaviors[i];
    thread_state_flavor_t flavor = machExceptionPortsSaved.flavors[i];
    mach_msg_type_number_t threadStateCount = THREAD_STATE_MAX;
    thread_state_data_t threadState;

    if (behavior != EXCEPTION_DEFAULT)
      thread_get_state(thread, flavor, threadState, &threadStateCount);

    switch (behavior) {
      case EXCEPTION_DEFAULT:
        result = mach_exception_raise_OVR(
            port, thread, task, exceptionType, pMachExceptionData, exceptionDataCount);
        break;

      case EXCEPTION_STATE:
        result = mach_exception_raise_state_OVR(
            port,
            exceptionType,
            pMachExceptionData,
            exceptionDataCount,
            &flavor,
            threadState,
            threadStateCount,
            threadState,
            &threadStateCount);
        break;

      case EXCEPTION_STATE_IDENTITY:
        result = mach_exception_raise_state_identity_OVR(
            port,
            thread,
            task,
            exceptionType,
            pMachExceptionData,
            exceptionDataCount,
            &flavor,
            threadState,
            threadStateCount,
            threadState,
            &threadStateCount);
        break;

      default:
        result = KERN_FAILURE;
        break;
    }

    if (behavior != EXCEPTION_DEFAULT)
      result = thread_set_state(thread, flavor, threadState, threadStateCount);
  }

  return result;
}

#endif // OVR_OS_APPLE

bool ExceptionHandler::Enable(bool enable) {
#if defined(OVR_OS_MS)
  if (enable && !enabled) {
    OVR_ASSERT(vectoredHandle == nullptr);
    vectoredHandle = AddVectoredExceptionHandler(1, Win32ExceptionFilter); // Windows call.
    enabled = (vectoredHandle != nullptr);
    OVR_ASSERT(enabled);
    sExceptionHandler = this;
    return enabled;
  } else if (!enable && enabled) {
    if (sExceptionHandler == this)
      sExceptionHandler = nullptr;
    OVR_ASSERT(vectoredHandle != nullptr);
    ULONG result = RemoveVectoredExceptionHandler(vectoredHandle); // Windows call.
    OVR_ASSERT_AND_UNUSED(result != 0, result);
    vectoredHandle = nullptr;
    enabled = false;
    return true;
  }

#elif defined(OVR_OS_APPLE)

  if (enable && !enabled) {
    enabled = InitMachExceptionHandler();
    OVR_ASSERT(enabled);
    sExceptionHandler = this;
    return enabled;
  } else if (!enable && enabled) {
    if (sExceptionHandler == this)
      sExceptionHandler = nullptr;
    ShutdownMachExceptionHandler();
    enabled = false;
    return true;
  }
#else
  OVR_UNUSED(enable);
#endif

  return true;
}

int ExceptionHandler::PauseHandling(bool pause) {
  if (pause)
    return ++pauseCount;

  OVR_ASSERT(pauseCount > 0);
  return --pauseCount;
}

void ExceptionHandler::EnableReportPrivacy(bool enable) {
  reportPrivacyEnabled = enable;
}

void ExceptionHandler::WriteExceptionDescription() {
#if defined(OVR_OS_MS)
  // There is some extra information available for AV exception.
  if (exceptionInfo.exceptionRecord.ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
    const char* error = (exceptionInfo.exceptionRecord.ExceptionInformation[0] == 0)
        ? "reading"
        : ((exceptionInfo.exceptionRecord.ExceptionInformation[0] == 1) ? "writing" : "executing");

    char addressStr[24];
    SprintfAddress(addressStr, OVR_ARRAY_COUNT(addressStr), exceptionInfo.pExceptionMemoryAddress);
    snprintf(
        exceptionInfo.exceptionDescription,
        OVR_ARRAY_COUNT(exceptionInfo.exceptionDescription),
        "ACCESS_VIOLATION %s address %s",
        error,
        addressStr);
  } else {
    exceptionInfo.exceptionDescription[0] = 0;

// Process "standard" exceptions, other than 'access violation'
#define FORMAT_EXCEPTION(x)                                   \
  case EXCEPTION_##x:                                         \
    OVR::OVR_strlcpy(                                         \
        exceptionInfo.exceptionDescription,                   \
        #x,                                                   \
        OVR_ARRAY_COUNT(exceptionInfo.exceptionDescription)); \
    break;

    switch (exceptionInfo.exceptionRecord.ExceptionCode) {
      // FORMAT_EXCEPTION(ACCESS_VIOLATION) Already handled above.
      FORMAT_EXCEPTION(DATATYPE_MISALIGNMENT)
      FORMAT_EXCEPTION(BREAKPOINT)
      FORMAT_EXCEPTION(SINGLE_STEP)
      FORMAT_EXCEPTION(ARRAY_BOUNDS_EXCEEDED)
      FORMAT_EXCEPTION(FLT_DENORMAL_OPERAND)
      FORMAT_EXCEPTION(FLT_DIVIDE_BY_ZERO)
      FORMAT_EXCEPTION(FLT_INEXACT_RESULT)
      FORMAT_EXCEPTION(FLT_INVALID_OPERATION)
      FORMAT_EXCEPTION(FLT_OVERFLOW)
      FORMAT_EXCEPTION(FLT_STACK_CHECK)
      FORMAT_EXCEPTION(FLT_UNDERFLOW)
      FORMAT_EXCEPTION(INT_DIVIDE_BY_ZERO)
      FORMAT_EXCEPTION(INT_OVERFLOW)
      FORMAT_EXCEPTION(PRIV_INSTRUCTION)
      FORMAT_EXCEPTION(IN_PAGE_ERROR)
      FORMAT_EXCEPTION(ILLEGAL_INSTRUCTION)
      FORMAT_EXCEPTION(NONCONTINUABLE_EXCEPTION)
      FORMAT_EXCEPTION(STACK_OVERFLOW)
      FORMAT_EXCEPTION(INVALID_DISPOSITION)
      FORMAT_EXCEPTION(GUARD_PAGE)
      FORMAT_EXCEPTION(INVALID_HANDLE)
#if defined(EXCEPTION_POSSIBLE_DEADLOCK) && \
    defined(STATUS_POSSIBLE_DEADLOCK) // This type seems to be non-existant in practice.
      FORMAT_EXCEPTION(POSSIBLE_DEADLOCK)
#endif
    }

    // If not one of the "known" exceptions, try to get the string from NTDLL.DLL's message table.
    if (exceptionInfo.exceptionDescription[0] == 0) {
      char addressStr[24];
      SprintfAddress(
          addressStr, OVR_ARRAY_COUNT(addressStr), exceptionInfo.pExceptionMemoryAddress);

      char buffer[384];
      DWORD capacity = OVR_ARRAY_COUNT(buffer);

      const size_t length = (size_t)FormatMessageA(
          FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
          GetModuleHandleW(L"NTDLL.DLL"),
          exceptionInfo.exceptionRecord.ExceptionCode,
          0,
          buffer,
          capacity,
          nullptr);
      if (length)
        snprintf(
            exceptionInfo.exceptionDescription,
            OVR_ARRAY_COUNT(exceptionInfo.exceptionDescription),
            "%s at instruction %s",
            buffer,
            addressStr);

      // If everything else failed just show the hex code.
      if (exceptionInfo.exceptionDescription[0] == 0)
        snprintf(
            exceptionInfo.exceptionDescription,
            OVR_ARRAY_COUNT(exceptionInfo.exceptionDescription),
            "Unknown exception 0x%08x at instruction %s",
            exceptionInfo.exceptionRecord.ExceptionCode,
            addressStr);
    }
  }

#elif defined(OVR_OS_APPLE)
  struct MachExceptionInfo {
    static const char* GetCPUExceptionIdString(uint32_t cpuExceptionId) {
      const char* id;

#if defined(OVR_CPU_X86) || defined(OVR_CPU_X86_64)
      switch (cpuExceptionId) {
        case 0:
          id = "integer div/0";
          break;
        case 1:
          id = "breakpoint fault";
          break;
        case 2:
          id = "non-maskable interrupt";
          break;
        case 3:
          id = "int 3";
          break;
        case 4:
          id = "overflow";
          break;
        case 5:
          id = "bounds check failure";
          break;
        case 6:
          id = "invalid instruction";
          break;
        case 7:
          id = "coprocessor unavailable";
          break;
        case 8:
          id = "exception within exception";
          break;
        case 9:
          id = "coprocessor segment overrun";
          break;
        case 10:
          id = "invalid task switch";
          break;
        case 11:
          id = "segment not present";
          break;
        case 12:
          id = "stack exception";
          break;
        case 13:
          id = "general protection fault";
          break;
        case 14:
          id = "page fault";
          break;
        case 16:
          id = "coprocessor error";
          break;
        default:
          id = "<unknown>";
          break;
      }
#else
// To do: Support ARM or others.
#endif

      return id;
    }

    static const char* GetMachExceptionTypeString(uint64_t exceptionCause) {
      switch (exceptionCause) {
        case EXC_ARITHMETIC:
          return "EXC_ARITHMETIC";
        case EXC_BAD_ACCESS:
          return "EXC_BAD_ACCESS";
        case EXC_BAD_INSTRUCTION:
          return "EXC_BAD_INSTRUCTION";
        case EXC_BREAKPOINT:
          return "EXC_BREAKPOINT";
        case EXC_CRASH:
          return "EXC_CRASH";
        case EXC_EMULATION:
          return "EXC_EMULATION";
        case EXC_MACH_SYSCALL:
          return "EXC_MACH_SYSCALL";
        case EXC_RPC_ALERT:
          return "EXC_RPC_ALERT";
        case EXC_SOFTWARE:
          return "EXC_SOFTWARE";
        case EXC_SYSCALL:
          return "EXC_SYSCALL";
      };

      return "EXC_<unknown>";
    }

    static const char* GetMachExceptionIdString(uint64_t machExceptionId, uint64_t code0) {
      const char* id = "<unknown>";

#if defined(OVR_CPU_X86) || defined(OVR_CPU_X86_64)
      switch (machExceptionId) {
        case EXC_ARITHMETIC:
          switch (code0) {
            case EXC_I386_BOUND:
              id = "EXC_I386_BOUND";
              break;
            case EXC_I386_DIV:
              id = "EXC_I386_DIV";
              break;
            case EXC_I386_EMERR:
              id = "EXC_I386_EMERR";
              break;
            case EXC_I386_EXTERR:
              id = "EXC_I386_EXTERR";
              break;
            case EXC_I386_EXTOVR:
              id = "EXC_I386_EXTOVR";
              break;
            case EXC_I386_INTO:
              id = "EXC_I386_INTO";
              break;
            case EXC_I386_NOEXT:
              id = "EXC_I386_NOEXT";
              break;
            case EXC_I386_SSEEXTERR:
              id = "EXC_I386_SSEEXTERR";
              break;
          }
          break;

        case EXC_BAD_INSTRUCTION:
          if (code0 == EXC_I386_INVOP)
            id = "EXC_I386_INVOP";
          break;

        case EXC_BREAKPOINT:
          if (code0 == EXC_I386_BPT)
            id = "EXC_I386_BPT";
          else if (code0 == EXC_I386_SGL)
            id = "EXC_I386_SGL";
          break;
      };
#else
// To do.
#endif

      return id;
    }
  };

  snprintf(
      exceptionInfo.exceptionDescription,
      OVR_ARRAY_COUNT(exceptionInfo.exceptionDescription),
      "Mach exception type: %llu (%s)\n",
      exceptionInfo.exceptionType,
      MachExceptionInfo::GetMachExceptionTypeString(exceptionInfo.exceptionType));

  snprintf(
      scratchBuffer,
      OVR_ARRAY_COUNT(scratchBuffer),
      "CPU exception info: exception id: %u (%s), exception id error: %u, fault memory address: %p\n",
      exceptionInfo.cpuExceptionId,
      MachExceptionInfo::GetCPUExceptionIdString(exceptionInfo.cpuExceptionId),
      exceptionInfo.cpuExceptionIdError,
      exceptionInfo.pExceptionMemoryAddress);
  OVR::OVR_strlcat(
      exceptionInfo.exceptionDescription,
      scratchBuffer,
      OVR_ARRAY_COUNT(exceptionInfo.exceptionDescription));

  snprintf(
      scratchBuffer,
      OVR_ARRAY_COUNT(scratchBuffer),
      "Mach exception info: exception id: %llu (%s), 0x%llx (%llu)\n",
      (uint64_t)exceptionInfo.machExceptionData[0],
      MachExceptionInfo::GetMachExceptionIdString(
          exceptionInfo.exceptionType, exceptionInfo.machExceptionData[0]),
      (uint64_t)exceptionInfo.machExceptionData[1],
      (uint64_t)exceptionInfo.machExceptionData[1]);
  OVR::OVR_strlcat(
      exceptionInfo.exceptionDescription,
      scratchBuffer,
      OVR_ARRAY_COUNT(exceptionInfo.exceptionDescription));
#else
  // To do.
  exceptionInfo.exceptionDescription[0] = 0;
#endif
}

void ExceptionHandler::writeLogLine(const char* buffer, int length) {
  OVR_ASSERT((int)strlen(buffer) == length); // Input must be null-terminated.

  if (LogFile != nullptr)
    fwrite(buffer, length, 1, LogFile);

  fwrite(buffer, length, 1, stdout);

#if defined(OVR_OS_WIN32)
  ::OutputDebugStringA(buffer);
#endif
}

void ExceptionHandler::WriteReportLine(const char* pLine) {
  writeLogLine(pLine, (int)strlen(pLine));
}

void ExceptionHandler::WriteReportLineF(const char* format, ...) {
  va_list args;
  va_start(args, format);
  int length = vsnprintf(scratchBuffer, OVR_ARRAY_COUNT(scratchBuffer), format, args);
  if (length >= (int)OVR_ARRAY_COUNT(scratchBuffer)) // If we didn't have enough space...
    length = (OVR_ARRAY_COUNT(scratchBuffer) - 1); // ... use what we have.
  va_end(args);

  writeLogLine(scratchBuffer, length);
}

// Thread <name> <handle> <id>
// 0   <module> <address> <function> <file>:<line>
// 1   <module> <address> <function> <file>:<line>
// . . .
//
void ExceptionHandler::WriteThreadCallstack(
    ThreadHandle threadHandle,
    ThreadSysId threadSysId,
    const char* additionalInfo) {
  // We intentionally do not directly use the SymbolInfo::ReportThreadCallstack function because
  // that function allocates memory,
  // which we cannot do due to possibly being within an exception handler.

  // Print the header
  char threadSysIdStr[32];
  char stackBaseStr[24];
  char stackLimitStr[24];
  char stackCurrentStr[24];
  void* pStackBase;
  void* pStackLimit;
  bool isExceptionThread = (threadSysId == exceptionInfo.threadSysId);

#if defined(OVR_OS_MS) && (OVR_PTR_SIZE == 8)
  void* pStackCurrent = (threadSysId == exceptionInfo.threadSysId)
      ? (void*)exceptionInfo.cpuContext.Rsp
      : nullptr; // We would need to suspend the thread, get its context, resume it, then read the
// rsp register. It turns out we are already doing that suspend/resume below in the
// backtrace call.
#elif defined(OVR_OS_MS)
  void* pStackCurrent =
      (threadSysId == exceptionInfo.threadSysId) ? (void*)exceptionInfo.cpuContext.Esp : nullptr;
#elif defined(OVR_OS_MAC) && (OVR_PTR_SIZE == 8)
  void* pStackCurrent = (threadSysId == exceptionInfo.threadSysId)
      ? (void*)exceptionInfo.cpuContext.threadState.uts.ts64.__rsp
      : nullptr;
#elif defined(OVR_OS_MAC)
  void* pStackCurrent = (threadSysId == exceptionInfo.threadSysId)
      ? (void*)exceptionInfo.cpuContext.threadState.uts.ts32.__esp
      : nullptr;
#elif defined(OVR_OS_LINUX)
  void* pStackCurrent = nullptr; // To do.
#endif

  OVR::GetThreadStackBounds(pStackBase, pStackLimit, threadHandle);

  SprintfThreadSysId(threadSysIdStr, OVR_ARRAY_COUNT(threadSysIdStr), threadSysId);
  SprintfAddress(stackBaseStr, OVR_ARRAY_COUNT(stackBaseStr), pStackBase);
  SprintfAddress(stackLimitStr, OVR_ARRAY_COUNT(stackLimitStr), pStackLimit);
  SprintfAddress(stackCurrentStr, OVR_ARRAY_COUNT(stackCurrentStr), pStackCurrent);

  WriteReportLineF(
      "Thread id: %s, stack base: %s, stack limit: %s, stack current: %s, %s\n",
      threadSysIdStr,
      stackBaseStr,
      stackLimitStr,
      stackCurrentStr,
      additionalInfo ? additionalInfo : "");

  // Print the backtrace info
  void* addressArray[64];
  size_t addressCount = symbolLookup.GetBacktraceFromThreadSysId(
      addressArray, OVR_ARRAY_COUNT(addressArray), 0, threadSysId);
  SymbolInfo symbolInfo;
  const char* pModuleName;
  size_t backtraceSkipCount = 0;

  if (isExceptionThread) {
// If this thread is the exception thread, skip some frames.
#if defined(OVR_OS_MS)
    size_t i, iEnd = MIN(16, addressCount);

    for (i = 0; i < iEnd; i++) {
      symbolLookup.LookupSymbol((uint64_t)addressArray[i], symbolInfo);
      if (strstr(symbolInfo.function, "UserExceptionDispatcher") != nullptr)
        break;
    }

    if (i < iEnd) // If found...
      backtraceSkipCount = i;
    else if (addressCount >= 9) // Else default to 9, which is coincidentally what works.
      backtraceSkipCount = 9;
    else
      backtraceSkipCount = 0;

    addressArray[backtraceSkipCount] = exceptionInfo.pExceptionInstructionAddress;
#endif
  }

  if (addressCount == 0) {
    WriteReportLine("<Unable to read backtrace>\n\n");
  } else {
    for (size_t i = backtraceSkipCount; i < addressCount; ++i) {
      symbolLookup.LookupSymbol((uint64_t)addressArray[i], symbolInfo);

      if (symbolInfo.pModuleInfo && symbolInfo.pModuleInfo->name[0])
        pModuleName = symbolInfo.pModuleInfo->name;
      else
        pModuleName = "(unknown module)";

      char addressStr[24];
      SprintfAddress(addressStr, OVR_ARRAY_COUNT(addressStr), addressArray[i]);

      if (symbolInfo.filePath[0])
        WriteReportLineF(
            "%-2u %-24s %s %s+%d %s:%d\n%s",
            (unsigned)i,
            pModuleName,
            addressStr,
            symbolInfo.function,
            symbolInfo.functionOffset,
            symbolInfo.filePath,
            symbolInfo.fileLineNumber,
            (i + 1) == addressCount ? "\n" : "");
      else
        WriteReportLineF(
            "%-2u %-24s %s %s+%d\n%s",
            (unsigned)i,
            pModuleName,
            addressStr,
            symbolInfo.function,
            symbolInfo.functionOffset,
            (i + 1) == addressCount ? "\n" : ""); // If this is the last line, append another \n.
    }
  }
}

void ExceptionHandler::WriteReport(const char* reportType) {
  // It's important that we don't allocate any memory here if we can help it.
  using namespace OVR;

  if (!reportType)
    reportType = "Exception";

  if (strstr(
          reportFilePath,
          "%s")) // If the user-specified file path includes a date/time component...
  {
    char dateTimeBuffer[64];
    FormatDateTime(
        dateTimeBuffer,
        OVR_ARRAY_COUNT(dateTimeBuffer),
        exceptionInfo.timeVal,
        true,
        true,
        false,
        true);
    snprintf(
        reportFilePathActual,
        OVR_ARRAY_COUNT(reportFilePathActual),
        reportFilePath,
        dateTimeBuffer);
  } else {
    OVR_strlcpy(reportFilePathActual, reportFilePath, OVR_ARRAY_COUNT(reportFilePathActual));
  }

  // Since LogFile is still null at this point, ...
  OVR_ASSERT(LogFile == nullptr);

  // ...we are writing this to the console/syslog but not the log file.
  if (strcmp(reportType, "Exception") == 0)
    WriteReportLine("[ExceptionHandler] Exception caught! ");

  WriteReportLineF("Writing report to file: %s\n", reportFilePathActual);

#if defined(OVR_OS_WIN32)
  HANDLE hEventSource = ::RegisterEventSourceW(nullptr, OVR_SYSLOG_NAME);

  if (hEventSource) {
    // This depends on the scratch buffer containing the file location.
    const char* lines = scratchBuffer;
    static_assert(
        sizeof(scratchBuffer) < 31839, "RegisterEventSource has a size limit of 31839 per string");
    ::ReportEventA(hEventSource, EVENTLOG_ERROR_TYPE, 0, 0, nullptr, 1, 0, &lines, nullptr);
    ::DeregisterEventSource(hEventSource);
  }
#endif

  LogFile = fopen(reportFilePathActual, "w");
  OVR_ASSERT(LogFile != nullptr);
  if (!LogFile)
    return;

  SymbolLookup::Initialize();

  {
    // Exception information
    WriteReportLineF("%s Info\n", reportType);

    WriteReportLineF("%s report file: %s\n", reportType, reportFilePathActual);

#if defined(OVR_OS_MS)
    if (miniDumpFilePath[0])
      WriteReportLineF("%s minidump file: %s\n", reportType, minidumpFilePathActual);
#endif

    char dateTimeBuffer[64];
    FormatDateTime(
        dateTimeBuffer,
        OVR_ARRAY_COUNT(dateTimeBuffer),
        exceptionInfo.timeVal,
        true,
        true,
        false,
        false);
    WriteReportLineF("Time (GMT): %s\n", dateTimeBuffer);

    FormatDateTime(
        dateTimeBuffer,
        OVR_ARRAY_COUNT(scratchBuffer),
        exceptionInfo.timeVal,
        true,
        true,
        true,
        false);
    WriteReportLineF("Time (local): %s\n", dateTimeBuffer);
    WriteReportLineF(
        "Thread name: %s\n",
        exceptionInfo.threadName[0] ? exceptionInfo.threadName
                                    : "(not available)"); // It's never possible on Windows to get
    // thread names, as they are stored in the
    // debugger at runtime.

    SprintfThreadHandle(scratchBuffer, OVR_ARRAY_COUNT(scratchBuffer), exceptionInfo.threadHandle);
    OVR_strlcat(scratchBuffer, "\n", OVR_ARRAY_COUNT(scratchBuffer));
    WriteReportLine("Thread handle: ");
    WriteReportLine(scratchBuffer);

    SprintfThreadSysId(scratchBuffer, OVR_ARRAY_COUNT(scratchBuffer), exceptionInfo.threadSysId);
    OVR_strlcat(scratchBuffer, "\n", OVR_ARRAY_COUNT(scratchBuffer));
    WriteReportLine("Thread sys id: ");
    WriteReportLine(scratchBuffer);

    char addressStr[24];
    SprintfAddress(
        addressStr, OVR_ARRAY_COUNT(addressStr), exceptionInfo.pExceptionInstructionAddress);
    WriteReportLineF("Exception instruction address: %s (see callstack below)\n", addressStr);
    WriteReportLineF("Exception description: %s\n", exceptionInfo.exceptionDescription);

    if (symbolLookup.LookupSymbol(
            (uint64_t)exceptionInfo.pExceptionInstructionAddress, exceptionInfo.symbolInfo)) {
      if (exceptionInfo.symbolInfo.filePath[0])
        WriteReportLineF(
            "Exception location: %s (%d)\n",
            exceptionInfo.symbolInfo.filePath,
            exceptionInfo.symbolInfo.fileLineNumber);
      else
        WriteReportLineF(
            "Exception location: %s (%d)\n",
            exceptionInfo.symbolInfo.function,
            exceptionInfo.symbolInfo.functionOffset);
    }

    // To consider: print exceptionInfo.cpuContext registers
  }

#if defined(OVR_OS_WIN32)
  {
    WriteReportLine("\nApp Info\n");

    // Print the app path.
    char appPath[OVR_MAX_PATH];
    GetCurrentProcessFilePath(appPath, OVR_ARRAY_COUNT(appPath));
    WriteReportLineF("Process path: %s\n", appPath);

#if (OVR_PTR_SIZE == 4)
    WriteReportLine("App format: 32 bit\n");
#else
    WriteReportLine("App format: 64 bit\n");
#endif

    // Print the app version
    wchar_t pathW[OVR_MAX_PATH] = {};
    GetModuleFileNameW(0, pathW, (DWORD)OVR_ARRAY_COUNT(pathW));
    DWORD dwUnused;
    DWORD dwSize = GetFileVersionInfoSizeW(pathW, &dwUnused);
    scratchBuffer[0] = 0;

    if (dwSize > 0) {
      void* const pVersionData = SafeMMapAlloc(dwSize);

      if (pVersionData) {
        if (GetFileVersionInfoW(pathW, 0, dwSize, pVersionData)) {
          VS_FIXEDFILEINFO* pFFI;
          UINT size;

          if (VerQueryValueW(pVersionData, L"\\", (void**)&pFFI, &size)) {
            // This is the convention used by RelEng to encode the CL # for releases.
            // This greatly simplifies figuring out which version of the Oculus Runtime
            // this file came from, though it may not apply to some users of DebugHelp.
            // Feel free to ignore it if you're not looking at the Oculus Runtime.
            const unsigned CLNum = LOWORD(pFFI->dwFileVersionLS) + 65536;

            WriteReportLineF(
                "App version: %u.%u.%u.%u (Runtime Installer CL# %u)\n",
                HIWORD(pFFI->dwFileVersionMS),
                LOWORD(pFFI->dwFileVersionMS),
                HIWORD(pFFI->dwFileVersionLS),
                LOWORD(pFFI->dwFileVersionLS),
                CLNum);
          }
        }

        SafeMMapFree(pVersionData, dwSize);
      }
    }

    if (!scratchBuffer[0]) // If version info couldn't be found or read...
      WriteReportLine("App version info not present\n");
  }

  {
    WriteReportLine("\nSystem Info\n");

    OSVERSIONINFOEXW vi;
    memset(&vi, 0, sizeof(vi));
    vi.dwOSVersionInfoSize = sizeof(vi);
    GetVersionExW((LPOSVERSIONINFOW)&vi); // Cast to the older type.

    char osVersionName[256];
    GetOSVersionName(osVersionName, OVR_ARRAY_COUNT(osVersionName));
    WriteReportLineF(
        "OS name: %s, version: %u.%u build %u, %s, platform id: %u, service pack: %ls\n",
        osVersionName,
        vi.dwMajorVersion,
        vi.dwMinorVersion,
        vi.dwBuildNumber,
        Is64BitOS() ? "64 bit" : "32 bit",
        vi.dwPlatformId,
        vi.szCSDVersion[0] ? vi.szCSDVersion : L"<none>");

    WriteReportLineF("Debugger present: %s\n", OVRIsDebuggerPresent() ? "yes" : "no");

    // System info
    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo(&systemInfo);

    WriteReportLineF("Processor count: %u\n", systemInfo.dwNumberOfProcessors);

    // Windows Vista and later:
    // BOOL WINAPI GetLogicalProcessorInformation(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer,
    // PDWORD ReturnLength);

    if (systemInfo.wProcessorArchitecture == 0)
      WriteReportLineF("Processor type: x86\n");
    else if (systemInfo.wProcessorArchitecture == 9)
      WriteReportLineF("Processor type: x86-64\n");
    else if (systemInfo.wProcessorArchitecture == 10)
      WriteReportLineF("Processor type: x86 on x86-64\n");

    WriteReportLineF("Processor level: %u\n", systemInfo.wProcessorLevel);
    WriteReportLineF("Processor revision: %u\n", systemInfo.wProcessorRevision);

    // Memory information
    MEMORYSTATUSEX memoryStatusEx;
    memset(&memoryStatusEx, 0, sizeof(memoryStatusEx));
    memoryStatusEx.dwLength = sizeof(memoryStatusEx);
    GlobalMemoryStatusEx(&memoryStatusEx);

    WriteReportLineF("Memory load: %d%%\n", memoryStatusEx.dwMemoryLoad);
    WriteReportLineF(
        "Total physical memory: %I64d MiB\n",
        memoryStatusEx.ullTotalPhys / (1024 * 1024)); // Or are Mebibytes equal to (1024 * 1000)
    WriteReportLineF(
        "Available physical memory: %I64d MiB\n", memoryStatusEx.ullAvailPhys / (1024 * 1024));
    WriteReportLineF(
        "Total page file memory: %I64d MiB\n", memoryStatusEx.ullTotalPageFile / (1024 * 1024));
    WriteReportLineF(
        "Available page file memory: %I64d MiB\n", memoryStatusEx.ullAvailPageFile / (1024 * 1024));
    WriteReportLineF(
        "Total virtual memory: %I64d MiB\n", memoryStatusEx.ullTotalVirtual / (1024 * 1024));
    WriteReportLineF(
        "Free virtual memory: %I64d MiB\n", memoryStatusEx.ullAvailVirtual / (1024 * 1024));

    DISPLAY_DEVICEW dd;
    memset(&dd, 0, sizeof(DISPLAY_DEVICE));
    dd.cb = sizeof(DISPLAY_DEVICE);

    for (int i = 0; EnumDisplayDevicesW(nullptr, (DWORD)i, &dd, EDD_GET_DEVICE_INTERFACE_NAME);
         ++i) {
      WriteReportLineF(
          "Display Device %d name: %ls, context: %ls, primary: %s, mirroring: %s\n",
          i,
          dd.DeviceName,
          dd.DeviceString,
          (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) ? "yes" : "no",
          (dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) ? "yes" : "no");
    }
  }

  // Print video card information
  // http://msdn.microsoft.com/en-us/library/aa394512%28v=vs.85%29.aspx
  {
    IWbemLocator* pIWbemLocator = nullptr;
    BSTR bstrServer = nullptr;
    IWbemServices* pIWbemServices = nullptr;
    BSTR bstrWQL = nullptr;
    BSTR bstrPath = nullptr;
    IEnumWbemClassObject* pEnum = nullptr;
    IWbemClassObject* pObj = nullptr;

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    HRESULT hr = CoCreateInstance(
        __uuidof(WbemLocator),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IWbemLocator),
        (LPVOID*)&pIWbemLocator);
    if (FAILED(hr))
      goto End;

    bstrServer = SysAllocString(L"\\\\.\\root\\cimv2");
    hr = pIWbemLocator->ConnectServer(
        bstrServer, nullptr, nullptr, 0L, 0L, nullptr, nullptr, &pIWbemServices);
    if (FAILED(hr))
      goto End;

    hr = CoSetProxyBlanket(
        pIWbemServices,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        nullptr,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_DEFAULT);
    if (FAILED(hr))
      goto End;

    bstrWQL = SysAllocString(L"WQL");
    bstrPath = SysAllocString(L"select * from Win32_VideoController");
    hr = pIWbemServices->ExecQuery(bstrWQL, bstrPath, WBEM_FLAG_FORWARD_ONLY, nullptr, &pEnum);
    if (FAILED(hr))
      goto End;

    ULONG uReturned;
    hr = pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturned);
    if (FAILED(hr))
      goto End;

    WriteReportLine("\nDisplay adapter list\n");

    // helper function to properly init & clear variants
    struct VariantHelper {
      VariantHelper() {
        ::VariantInit(&var); // must init before use
      }
      ~VariantHelper() {
        ::VariantClear(&var); // must clear after use to avoid leaks
      }

      BSTR getBSTR() const // return a value or default empty string
      {
        if ((var.vt == VT_BSTR) && var.bstrVal)
          return var.bstrVal;
        return L"";
      }

      uint32_t getLVal() const // return a value or default 0
      {
        if (var.vt == VT_I4)
          return var.lVal;
        return 0;
      }

      VARIANT* getVARIANT() // convenience function for passing to Get()
      {
        return &var;
      }

     private:
      VARIANT var;
    };

    for (unsigned i = 0; SUCCEEDED(hr) && uReturned; i++) {
      char sString[256];

      if (i > 0)
        WriteReportLine("\n");

      WriteReportLineF("Info for display adapter %u\n", i);

      {
        VariantHelper v;
        hr = pObj->Get(L"Name", 0, v.getVARIANT(), nullptr, nullptr);
        if (SUCCEEDED(hr)) {
          WideCharToMultiByte(
              CP_ACP, 0, v.getBSTR(), -1, sString, sizeof(sString), nullptr, nullptr);
          WriteReportLineF("Display Adapter Name: %s\n", sString);
        }
      }
      {
        VariantHelper v;
        hr = pObj->Get(L"AdapterRAM", 0, v.getVARIANT(), nullptr, nullptr);
        if (SUCCEEDED(hr)) {
          uint32_t value(v.getLVal());

          WriteReportLineF(
              "Display Adapter RAM: %u %s\n",
              (value > (1024 * 1024 * 1024) ? value / (1024 * 1024 * 1024) : value / (1024 * 1024)),
              (value > (1024 * 1024 * 1024) ? "GiB" : "MiB"));
        }
      }
      {
        VariantHelper v;
        hr = pObj->Get(L"DeviceID", 0, v.getVARIANT(), nullptr, nullptr);
        if (SUCCEEDED(hr)) {
          WideCharToMultiByte(
              CP_ACP, 0, v.getBSTR(), -1, sString, sizeof(sString), nullptr, nullptr);
          WriteReportLineF("Display Adapter DeviceID: %s\n", sString);
        }
      }
      {
        VariantHelper v;
        hr = pObj->Get(L"DriverVersion", 0, v.getVARIANT(), nullptr, nullptr);
        if (SUCCEEDED(hr)) {
          WideCharToMultiByte(
              CP_ACP, 0, v.getBSTR(), -1, sString, sizeof(sString), nullptr, nullptr);
          WriteReportLineF("Display Adapter DriverVersion: %s\n", sString);
        }
      }

      {
        VariantHelper v;
        hr = pObj->Get(L"DriverDate", 0, v.getVARIANT(), nullptr, nullptr);
        if (SUCCEEDED(hr)) {
          std::wstring val(v.getBSTR()); // may return empty string

          while (val.size() < 8) // make at least 8 chars long to simplify below code
          {
            val += L" ";
          }

          // http://technet.microsoft.com/en-us/library/ee156576.aspx
          wchar_t year[5] = {val[0], val[1], val[2], val[3], 0};
          wchar_t month[3] = {val[4], val[5], 0};
          wchar_t monthDay[3] = {val[6], val[7], 0};

          WriteReportLineF(
              "Display Adapter DriverDate (US format): %ls/%ls/%ls\n", month, monthDay, year);
        }
      }
      {
        VariantHelper v;
        // VideoProcessor
        hr = pObj->Get(L"VideoProcessor", 0, v.getVARIANT(), nullptr, nullptr);
        if (SUCCEEDED(hr)) {
          WideCharToMultiByte(
              CP_ACP, 0, v.getBSTR(), -1, sString, sizeof(sString), nullptr, nullptr);
          WriteReportLineF("Display Adapter VideoProcessor %s\n", sString);
        }
      }

      {
        VariantHelper v;
        hr = pObj->Get(L"VideoModeDescription", 0, v.getVARIANT(), nullptr, nullptr);
        if (SUCCEEDED(hr)) {
          WideCharToMultiByte(
              CP_ACP, 0, v.getBSTR(), -1, sString, sizeof(sString), nullptr, nullptr);
          WriteReportLineF("Display Adapter VideoModeDescription: %s\n", sString);
        }
      }

      pObj->Release();

      hr = pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturned);
    }

  End:
    if (pEnum)
      pEnum->Release();
    if (bstrPath)
      SysFreeString(bstrPath);
    if (bstrWQL)
      SysFreeString(bstrWQL);
    if (pIWbemServices)
      pIWbemServices->Release();
    if (bstrServer)
      SysFreeString(bstrServer);
    if (pIWbemLocator)
      pIWbemLocator->Release();

    CoUninitialize();
  }

  {
    // Print a list of threads.
    DWORD currentProcessId = GetCurrentProcessId();
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(
        TH32CS_SNAPTHREAD,
        currentProcessId); // ICreateToolhelp32Snapshot actually ignores currentProcessId.

    if (hThreadSnap != INVALID_HANDLE_VALUE) {
      THREADENTRY32 te32;
      te32.dwSize = sizeof(THREADENTRY32);

      if (Thread32First(hThreadSnap, &te32)) {
        WriteReportLine("\nThread list\n");

        do {
          if (te32.th32OwnerProcessID == currentProcessId) {
            OVR::ThreadSysId threadSysId = te32.th32ThreadID;
            OVR::ThreadSysId threadSysIdCurrent = ::GetCurrentThreadId();

            HANDLE hThread = ConvertThreadSysIdToThreadHandle(threadSysId);

            if (hThread) {
              if (threadSysId !=
                  threadSysIdCurrent) // If threadSysId refers to a different thread...
              {
                // We are working with another thread. We need to suspend it and get its CONTEXT.
                DWORD suspendResult = ::SuspendThread(
                    hThread); // Requires that the handle have THREAD_SUSPEND_RESUME rights.

                if (suspendResult == (DWORD)-1) // If failed...
                {
                  WriteReportLineF("Skipping thread id: %u\n", (unsigned)threadSysId);
                  continue;
                }
              }

              char
                  buffer[96]; // Can't use scratchBuffer, because it's used by WriteThreadCallstack.
              snprintf(
                  buffer,
                  OVR_ARRAY_COUNT(buffer),
                  "base priority: %ld, delta priority: %ld",
                  te32.tpBasePri,
                  te32.tpDeltaPri);

              bool threadIsExceptionThread =
                  (te32.th32ThreadID == (DWORD)exceptionInfo.threadSysId);
              if (threadIsExceptionThread)
                OVR_strlcat(buffer, ", exception thread", OVR_ARRAY_COUNT(buffer));

              WriteThreadCallstack(hThread, (OVR::ThreadSysId)te32.th32ThreadID, buffer);

              if (threadSysId != threadSysIdCurrent) // If we called SuspendThread above...
                ::ResumeThread(hThread);

              FreeThreadHandle(hThread);
            }
          }
        } while (Thread32Next(hThreadSnap, &te32));
      }

      CloseHandle(hThreadSnap);
    }
  }

  {
    // Print a list of the current modules within this process.
    // DbgHelp.dll also provides a EnumerateLoadedModules64 function.
    // To do: Convert the code below to use the GetModuleInfoArray function which we now have.
    HMODULE hModule = LoadLibraryW(L"psapi.dll");

    if (hModule) {
      typedef BOOL(WINAPI * ENUMPROCESSMODULES)(
          HANDLE hProcess, HMODULE * phModule, DWORD cb, LPDWORD lpcbNeeded);
      typedef DWORD(WINAPI * GETMODULEBASENAME)(
          HANDLE hProcess, HMODULE hModule, LPWSTR lpFilename, DWORD nSize);
      typedef DWORD(WINAPI * GETMODULEFILENAMEEX)(
          HANDLE hProcess, HMODULE hModule, LPWSTR lpFilename, DWORD nSize);
      typedef BOOL(WINAPI * GETMODULEINFORMATION)(
          HANDLE hProcess, HMODULE hModule, MODULEINFO * pmi, DWORD nSize);

      ENUMPROCESSMODULES pEnumProcessModules =
          (ENUMPROCESSMODULES)(uintptr_t)GetProcAddress(hModule, "EnumProcessModules");
      GETMODULEBASENAME pGetModuleBaseName =
          (GETMODULEBASENAME)(uintptr_t)GetProcAddress(hModule, "GetModuleBaseNameW");
      GETMODULEFILENAMEEX pGetModuleFileNameEx =
          (GETMODULEFILENAMEEX)(uintptr_t)GetProcAddress(hModule, "GetModuleFileNameExW");
      GETMODULEINFORMATION pGetModuleInformation =
          (GETMODULEINFORMATION)(uintptr_t)GetProcAddress(hModule, "GetModuleInformation");

      HANDLE hProcess = GetCurrentProcess();
      HMODULE hModuleArray[200];
      DWORD cbNeeded;

      if (pEnumProcessModules(hProcess, hModuleArray, sizeof(hModuleArray), &cbNeeded)) {
        size_t actualModuleCount = (cbNeeded / sizeof(HMODULE));

        if (actualModuleCount >
            OVR_ARRAY_COUNT(hModuleArray)) // If hModuleArray's capacity was not enough...
          actualModuleCount = OVR_ARRAY_COUNT(hModuleArray);

        // Print a header
        WriteReportLine("\nModule list\n");

#if (OVR_PTR_SIZE == 4)
        WriteReportLine("Base        Size       Entrypoint Name                     Path\n");
#else
        WriteReportLine(
            "Base                Size               Entrypoint         Name                     Path\n");
#endif

        // And go through the list one by one
        for (size_t i = 0; i < actualModuleCount; i++) {
          MODULEINFO mi;
          size_t length;

          if (!pGetModuleInformation(hProcess, hModuleArray[i], &mi, sizeof(mi))) {
            mi.EntryPoint = nullptr;
            mi.lpBaseOfDll = nullptr;
            mi.SizeOfImage = 0;
          }

          // Write the base name.
          wchar_t name[OVR_MAX_PATH + 3];
          name[0] = '"';
          if (pGetModuleBaseName(hProcess, hModuleArray[i], name + 1, OVR_MAX_PATH))
            length = wcslen(name);
          else {
            wcscpy(name + 1, L"(unknown)");
            length = 10;
          }

          name[length] = '"';
          name[length + 1] = '\0';

          // Write the path
          wchar_t path[OVR_MAX_PATH + 3];
          path[0] = '"';
          if (pGetModuleFileNameEx(hProcess, hModuleArray[i], path + 1, OVR_MAX_PATH))
            length = wcslen(path);
          else {
            wcscpy(path + 1, L"(unknown)");
            length = 10;
          }
          path[length] = '"';
          path[length + 1] = '\0';

#if (OVR_PTR_SIZE == 4)
          WriteReportLineF(
              "0x%08x, 0x%08x 0x%08x %-24ls %ls\n",
              (uint32_t)mi.lpBaseOfDll,
              (uint32_t)mi.SizeOfImage,
              (uint32_t)mi.EntryPoint,
              name,
              path);
#else
          WriteReportLineF(
              "0x%016I64x 0x%016I64x 0x%016I64x %-24ls %ls\n",
              (uint64_t)mi.lpBaseOfDll,
              (uint64_t)mi.SizeOfImage,
              (uint64_t)mi.EntryPoint,
              name,
              path);
#endif
        }
      }
    }
  }

  {
    // Print a list of processes.
    // DbgHelp.dll provides a SymEnumProcesses function, but it's available with DbgHelp.dll v6.2
    // which doesn't ship with Windows until Windows 8.
    WriteReportLine("\nProcess list\n");

    if (reportPrivacyEnabled)
      WriteReportLine("Disabled by report privacy settings\n");
    else {
      HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

      if (hProcessSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        memset(&pe32, 0, sizeof(pe32));
        pe32.dwSize = sizeof(pe32);

        if (Process32FirstW(hProcessSnapshot, &pe32)) {
          WriteReportLine("Process Id File\n");

          do {
            // Try to get the full path to the process, as pe32.szExeFile holds only the process
            // file name.
            // This will typically fail with a privilege error unless this process has debug
            // privileges: http://support.microsoft.com/kb/131065/en-us
            wchar_t filePathW[OVR_MAX_PATH];
            const wchar_t* pFilePathW = pe32.szExeFile;
            HANDLE hProcess = ::OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                FALSE,
                pe32.th32ProcessID); // With Windows Vista+ we can use
            // PROCESS_QUERY_LIMITED_INFORMATION.
            if (hProcess) {
              if (GetProcessImageFileNameW(hProcess, filePathW, (DWORD)OVR_ARRAY_COUNT(filePathW)))
                pFilePathW = filePathW;
            }

            WriteReportLineF("0x%08x %ls\n", pe32.th32ProcessID, pFilePathW);
          } while (Process32NextW(hProcessSnapshot, &pe32));
        }

        CloseHandle(hProcessSnapshot);
      } else {
        WriteReportLine("Unable to read process list\n");
      }
    }
  }

#elif defined(OVR_OS_APPLE)

  WriteReportLine("\nApp Info\n");

  // App path
  const pid_t processId = getpid();
  WriteReportLineF("Process id: ", "%lld (0x%llx)\n", (int64_t)processId, (int64_t)processId);

  char appPath[PATH_MAX];
  GetCurrentProcessFilePath(appPath, OVR_ARRAY_COUNT(appPath));
  WriteReportLineF("Process path: %s\n", appPath);

#if (OVR_PTR_SIZE == 4)
  WriteReportLine("App format: 32 bit\n");
#else
  WriteReportLine("App format: 64 bit\n");
#endif

  // App version
  // To do.

  // System Info
  WriteReportLine("\nSystem Info\n");

  char osVersionName[256];
  GetOSVersionName(osVersionName, OVR_ARRAY_COUNT(osVersionName));
  WriteReportLineF("OS name: %s, %s\n", osVersionName, Is64BitOS() ? "64 bit" : "32 bit");

  int name[2];
  int intValue;
  size_t length;
  char tempBuffer[256];

  name[0] = CTL_KERN;
  name[1] = KERN_OSTYPE;
  length = sizeof(tempBuffer);
  tempBuffer[0] = 0;
  if (sysctl(name, 2, tempBuffer, &length, nullptr, 0) == 0) {
    WriteReportLineF("KERN_OSTYPE: %s\n", tempBuffer);
  }

  name[0] = CTL_KERN;
  name[1] = KERN_OSREV;
  length = sizeof(intValue);
  intValue = 0;
  if (sysctl(name, 2, &intValue, &length, nullptr, 0) == 0) {
    WriteReportLineF("KERN_OSREV: %d\n", intValue);
  }

  name[0] = CTL_KERN;
  name[1] = KERN_OSRELEASE;
  length = sizeof(tempBuffer);
  tempBuffer[0] = 0;
  if (sysctl(name, 2, tempBuffer, &length, nullptr, 0) == 0)
    WriteReportLineF("KERN_OSRELEASE: %s\n", tempBuffer);

  name[0] = CTL_HW;
  name[1] = HW_MACHINE;
  length = sizeof(tempBuffer);
  tempBuffer[0] = 0;
  if (sysctl(name, 2, tempBuffer, &length, nullptr, 0) == 0)
    WriteReportLineF("HW_MACHINE: %s\n", tempBuffer);

  name[0] = CTL_HW;
  name[1] = HW_MODEL;
  length = sizeof(tempBuffer);
  tempBuffer[0] = 0;
  if (sysctl(name, 2, tempBuffer, &length, nullptr, 0) == 0)
    WriteReportLineF("sHW_MODEL: %s\n", tempBuffer);

  name[0] = CTL_HW;
  name[1] = HW_NCPU;
  length = sizeof(intValue);
  intValue = 0;
  if (sysctl(name, 2, &intValue, &length, nullptr, 0) == 0)
    WriteReportLineF("HW_NCPU: %d\n", intValue);

  length = sizeof(tempBuffer);
  tempBuffer[0] = 0;
  if (sysctlbyname("machdep.cpu.brand_string", &tempBuffer, &length, nullptr, 0) == 0)
    WriteReportLineF("machdep.cpu.brand_string: %s\n", tempBuffer);

  length = sizeof(tempBuffer);
  tempBuffer[0] = 0;
  if (sysctlbyname("hw.acpi.thermal.tz0.temperature", &tempBuffer, &length, nullptr, 0) == 0)
    WriteReportLineF("hw.acpi.thermal.tz0.temperature: %s\n", tempBuffer);

  host_basic_info_data_t hostinfo;
  mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
  kern_return_t kr = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostinfo, &count);

  if (kr == KERN_SUCCESS) {
    const uint64_t memoryMib = (uint64_t)hostinfo.max_mem / (1024 * 1024);
    WriteReportLineF("System memory: %lld Mib (%.1f Gib)\n", memoryMib, (double)memoryMib / 1024);
  }

  // Video card info
  // To do.

  // Thread list
  mach_port_t taskSelf = mach_task_self();
  thread_act_port_array_t threadArray;
  mach_msg_type_number_t threadCount;

  kern_return_t result = task_threads(taskSelf, &threadArray, &threadCount);

  if (result == KERN_SUCCESS) {
    WriteReportLine("\nThread list\n");

    for (mach_msg_type_number_t i = 0; i < threadCount; i++) {
      union TBIUnion {
        natural_t words[THREAD_INFO_MAX];
        thread_basic_info tbi;
      };

      TBIUnion tbiUnion;
      mach_port_t thread = threadArray[i];
      pthread_t pthread =
          pthread_from_mach_thread_np(thread); // We assume the thread was created through pthreads.

      char threadState[32] = "unknown";
      mach_msg_type_number_t threadInfoCount = THREAD_INFO_MAX;
      result = thread_info(thread, THREAD_BASIC_INFO, (thread_info_t)&tbiUnion, &threadInfoCount);

      if (result == KERN_SUCCESS) {
        const char* state;

        switch (tbiUnion.tbi.run_state) {
          case TH_STATE_HALTED:
            state = "halted";
            break;
          case TH_STATE_RUNNING:
            state = "running";
            break;
          case TH_STATE_STOPPED:
            state = "stopped";
            break;
          case TH_STATE_UNINTERRUPTIBLE:
            state = "uninterruptible";
            break;
          case TH_STATE_WAITING:
            state = "waiting";
            break;
          default:
            state = "<unknown>";
            break;
        }

        snprintf(threadState, OVR_ARRAY_COUNT(threadState), "%s", state);
        if (tbiUnion.tbi.flags & TH_FLAGS_IDLE)
          OVR_strlcat(threadState, ", idle", sizeof(threadState));
        if (tbiUnion.tbi.flags & TH_FLAGS_SWAPPED)
          OVR_strlcat(threadState, ", swapped", sizeof(threadState));
      }

      thread_identifier_info threadIdentifierInfo;
      memset(&threadIdentifierInfo, 0, sizeof(threadIdentifierInfo));

      mach_msg_type_number_t threadIdentifierInfoCount = THREAD_IDENTIFIER_INFO_COUNT;
      thread_info(
          thread,
          THREAD_IDENTIFIER_INFO,
          (thread_info_t)&threadIdentifierInfo,
          &threadIdentifierInfoCount);

      proc_threadinfo procThreadInfo;
      memset(&procThreadInfo, 0, sizeof(procThreadInfo));
      result = proc_pidinfo(
          processId,
          PROC_PIDTHREADINFO,
          threadIdentifierInfo.thread_handle,
          &procThreadInfo,
          sizeof(procThreadInfo));
      OVR_UNUSED(result);

      char buffer[256]; // Can't use scratchBuffer, because it's used by WriteThreadCallstack.
      snprintf(
          buffer,
          OVR_ARRAY_COUNT(buffer),
          "state: %s, suspend count: %d, kernel priority: %d",
          threadState,
          (int)tbiUnion.tbi.suspend_count,
          (int)procThreadInfo.pth_curpri);

      bool threadIsExceptionThread = (thread == exceptionInfo.threadSysId);
      if (threadIsExceptionThread)
        OVR_strlcat(buffer, ", exception thread", OVR_ARRAY_COUNT(buffer));

      WriteThreadCallstack(pthread, thread, buffer);
    }

    vm_deallocate(taskSelf, (vm_address_t)threadArray, threadCount * sizeof(thread_act_t));
  }

  WriteReportLine("\nModule list\n");

  const size_t mifCapacity = 256;
  const size_t mifAllocSize = mifCapacity * sizeof(ModuleInfo);
  ModuleInfo* moduleInfoArray = (ModuleInfo*)SafeMMapAlloc(mifAllocSize);

  if (moduleInfoArray) {
#if (OVR_PTR_SIZE == 4)
    WriteReportLine("Base        Size       Name                     Path\n");
#else
    WriteReportLine("Base                Size               Name                     Path\n");
#endif

    size_t moduleCount = symbolLookup.GetModuleInfoArray(moduleInfoArray, mifCapacity);
    if (moduleCount > mifCapacity)
      moduleCount = mifCapacity;

    for (size_t i = 0; i < moduleCount; i++) {
      const ModuleInfo& mi = moduleInfoArray[i];

#if (OVR_PTR_SIZE == 4)
      WriteReportLineF(
          "0x%08x, 0x%08x %-24s %s\n",
          (uint32_t)mi.baseAddress,
          (uint32_t)mi.size,
          mi.name,
          mi.filePath);
#else
      WriteReportLineF(
          "0x%016llx 0x%016llx %-24s %s\n",
          (uint64_t)mi.baseAddress,
          (uint64_t)mi.size,
          mi.name,
          mi.filePath);
#endif
    }

    SafeMMapFree(moduleInfoArray, mifAllocSize);
  }

  WriteReportLine("\nProcess list\n");

  if (reportPrivacyEnabled)
    WriteReportLine("Disabled by report privacy settings\n");
  else {
    WriteReportLine("Process Id File\n");

    pid_t pidArray[1024];
    int processCount = proc_listpids(
        PROC_ALL_PIDS,
        0,
        pidArray,
        sizeof(pidArray)); // Important that we use sizeof not OVR_ARRAY_COUNT.
    char processFilePath[PATH_MAX];

    for (int i = 0; i < processCount; i++) {
      if (proc_pidpath(pidArray[i], processFilePath, sizeof(processFilePath)) > 0)
        WriteReportLineF("%-10d %s\n", pidArray[i], processFilePath);
    }

    if (!processCount)
      WriteReportLine("Unable to read process list\n");
  }

#elif defined(OVR_OS_UNIX)
  Is64BitOS();
  GetCurrentProcessFilePath(nullptr, 0);
  GetFileNameFromPath(nullptr);
  GetOSVersionName(nullptr, 0);

#endif // OVR_OS_MS

  SymbolLookup::Shutdown();

  fclose(LogFile);
  LogFile = nullptr;
}

#if defined(OVR_OS_WIN32) || defined(OVR_OS_WIN64)

static bool IsModuleDataSegmentNeeded(const wchar_t* modulePath) {
  if (modulePath) {
    const wchar_t* recognizedModuleNames[] = {
        // To consider: make this dynamically specifiable by the application.
        L"OVRServer_x64",
        L"OculusAppFramework",
        L"hid",
        L"kernel32",
        L"KernelBase",
        L"msvcrt",
        L"ws2_32",
        L"mswsock",
        L"ntdll",
        L"user32",
        L"wshtcpip"};

    const wchar_t* recognizedModulePrefixes[] = {L"d3d", L"dxgi", L"nv", L"ati", L"amd"};

    wchar_t fileName[MAX_PATH] = {};
    ::_wsplitpath_s(modulePath, NULL, 0, NULL, 0, fileName, MAX_PATH, NULL, 0);
    std::transform(fileName, fileName + wcslen(fileName), fileName, ::towlower);

    for (size_t i = 0; i < OVR_ARRAY_COUNT(recognizedModuleNames); ++i) {
      if (wcscmp(fileName, recognizedModuleNames[i]) ==
          0) // If fileName equals recognizedModuleNames[i]...
        return true;
    }

    for (size_t i = 0; i < OVR_ARRAY_COUNT(recognizedModulePrefixes); ++i) {
      if (wcsstr(fileName, recognizedModulePrefixes[i]) ==
          fileName) // If fileName begins with recognizedModulePrefixes[i]...
        return true;
    }
  }

  return false;
}

struct MinidumpCallbackContext {
  ExceptionHandler::MinidumpInfoLevel infoLevel;
  DWORD minidumpThreadId;
};

static BOOL CALLBACK MinidumpFilter(
    PVOID pContext,
    const PMINIDUMP_CALLBACK_INPUT callbackInput,
    PMINIDUMP_CALLBACK_OUTPUT callbackOutput) {
  if (!callbackInput || !callbackOutput || !pContext) {
    // This should not normally occur.
    return FALSE;
  }

  // Most of the time, we're going to let the minidump writer do whatever
  // it needs to do.  However, when trying to limit information for things
  // like smaller dumps, we want to filter out various things.  So this callback
  // is used as a way to filter out information that the user has said they
  // don't want.
  //
  // If we were so inclined, we could use a delegate class to allow the user
  // to customize the dump files even further.  But for right now, this is
  // close enough.
  const MinidumpCallbackContext* pFilter =
      reinterpret_cast<const MinidumpCallbackContext*>(pContext);

  switch (callbackInput->CallbackType) {
    case IncludeModuleCallback:
    case ThreadCallback:
    case ThreadExCallback:
      return TRUE;

    case CancelCallback:
      return FALSE;

    case IncludeThreadCallback:
      // Return FALSE if the ThreadId is our minidump-writing thread, so it gets skipped.
      return (callbackInput->IncludeThread.ThreadId != pFilter->minidumpThreadId);

    case MemoryCallback:
      // Include memory only for large dumps.
      return (pFilter->infoLevel == ExceptionHandler::kMILLarge);

    case ModuleCallback: {
      if (pFilter->infoLevel == ExceptionHandler::kMILSmall) {
        // Filter out modules that aren't being referenced by memory.
        if (!(callbackOutput->ModuleWriteFlags & ModuleReferencedByMemory))
          callbackOutput->ModuleWriteFlags &= ~ModuleWriteModule;
      } else if (pFilter->infoLevel == ExceptionHandler::kMILMedium) {
        // Filter out modules that aren't of primary interest to us.
        if (callbackOutput->ModuleWriteFlags & ModuleWriteDataSeg) {
          if (!IsModuleDataSegmentNeeded(callbackInput->Module.FullPath))
            callbackOutput->ModuleWriteFlags &= ~ModuleWriteDataSeg;
        }
      } else if (pFilter->infoLevel == ExceptionHandler::kMILLarge) {
        // We currently write all modules.
      }

      return TRUE;
    }
  }

  return FALSE;
}

#endif // defined(OVR_OS_WIN32) || defined(OVR_OS_WIN64)

void ExceptionHandler::WriteMiniDump() {
  if (minidumpInfoLevel == kMILNone)
    return;

  if (strstr(
          miniDumpFilePath,
          "%s")) // If the user-specified file path includes a date/time component...
  {
    char dateTimeBuffer[64];
    FormatDateTime(
        dateTimeBuffer,
        OVR_ARRAY_COUNT(dateTimeBuffer),
        exceptionInfo.timeVal,
        true,
        true,
        false,
        true);
    snprintf(
        minidumpFilePathActual,
        OVR_ARRAY_COUNT(minidumpFilePathActual),
        miniDumpFilePath,
        dateTimeBuffer);
  } else {
    OVR_strlcpy(minidumpFilePathActual, miniDumpFilePath, OVR_ARRAY_COUNT(minidumpFilePathActual));
  }

#if defined(OVR_OS_WIN32) || defined(OVR_OS_WIN64)
  typedef BOOL(WINAPI * MINIDUMPWRITEDUMP)(
      HANDLE hProcess,
      DWORD ProcessId,
      HANDLE hFile,
      MINIDUMP_TYPE dumpType,
      CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
      CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
      CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
  HMODULE hModuleDbgHelp = LoadLibraryW(L"DbgHelp.dll");

  MINIDUMPWRITEDUMP pMiniDumpWriteDump = hModuleDbgHelp
      ? (MINIDUMPWRITEDUMP)(void*)GetProcAddress(hModuleDbgHelp, "MiniDumpWriteDump")
      : nullptr;

  if (pMiniDumpWriteDump) {
    wchar_t miniDumpFilePathW[OVR_MAX_PATH];
    auto requiredUTF8Length = OVR::UTF8Util::Strlcpy(
        miniDumpFilePathW, OVR_ARRAY_COUNT(miniDumpFilePathW), minidumpFilePathActual);
    if (requiredUTF8Length < OVR_ARRAY_COUNT(miniDumpFilePathW)) {
      HANDLE hFile = CreateFileW(
          miniDumpFilePathW,
          GENERIC_READ | GENERIC_WRITE,
          0,
          0,
          CREATE_ALWAYS,
          FILE_FLAG_WRITE_THROUGH,
          0);

      if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION minidumpExceptionInfo = {
            ::GetCurrentThreadId(), pExceptionPointers, TRUE};

        int miniDumpFlags = MiniDumpNormal;

        switch ((int)minidumpInfoLevel) {
          case kMILSmall: {
            miniDumpFlags |= MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory;
            break;
          }

          case kMILMedium: {
            miniDumpFlags |= MiniDumpWithDataSegs | MiniDumpWithPrivateReadWriteMemory |
                MiniDumpWithHandleData | MiniDumpWithFullMemoryInfo | MiniDumpWithThreadInfo |
                MiniDumpWithUnloadedModules;
            break;
          }

          case kMILLarge: {
            miniDumpFlags |= MiniDumpWithDataSegs | MiniDumpWithPrivateReadWriteMemory |
                MiniDumpWithHandleData | MiniDumpWithFullMemory | MiniDumpWithFullMemoryInfo |
                MiniDumpWithThreadInfo | MiniDumpWithUnloadedModules |
                MiniDumpWithProcessThreadData | MiniDumpIgnoreInaccessibleMemory |
                MiniDumpWithTokenInformation;
            break;
          }
        }

        MinidumpCallbackContext minidumpCallbackContext = {minidumpInfoLevel,
                                                           ::GetCurrentThreadId()};
        MINIDUMP_CALLBACK_INFORMATION minidumpCallbackInfo = {MinidumpFilter,
                                                              &minidumpCallbackContext};

        // We are about to write out a minidump due to a crash.  This minidump write can take a very
        // long time.
        // Look up the stack to see what went wrong.  This is a bug in the code somewhere.
        if (OVRIsDebuggerPresent()) {
          OVR_ASSERT(false);
        }

        BOOL result = pMiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            (MINIDUMP_TYPE)miniDumpFlags,
            pExceptionPointers ? &minidumpExceptionInfo : nullptr,
            (CONST PMINIDUMP_USER_STREAM_INFORMATION) nullptr,
            &minidumpCallbackInfo);
        CloseHandle(hFile);
        hFile = 0;

        OVR_ASSERT(result);
        if (!result) {
// We print out some error information in debug builds. We can't write to
// a log because we are likely executing this within an exception.
#if defined(OVR_BUILD_DEBUG)
          DWORD dwError = GetLastError();
          char errorBufferA[1024];
          DWORD errorBufferACapacity = OVR_ARRAY_COUNT(errorBufferA);
          DWORD length = FormatMessageA(
              FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
              nullptr,
              dwError,
              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
              errorBufferA,
              errorBufferACapacity,
              nullptr);

          if (length) {
            OutputDebugStringA("MiniDumpWriteDump failure: ");
            OutputDebugStringA(errorBufferA);
          }
#endif

          DeleteFileW(miniDumpFilePathW);
        }
      } else {
        // XXX this will never assert
        OVR_ASSERT(pMiniDumpWriteDump); // OVR_FAIL_F(("ExceptionHandler::WriteMiniDump: Failed to
        // create minidump file at %s", minidumpFilePathActual));
      }
    } else {
      // failed to convert miniDumpFilePathW
      OVR_ASSERT(false);
    }
  }

  FreeLibrary(hModuleDbgHelp);
#else
// Some platforms support various forms or exception reports and core dumps which are automatically
// generated upon us
// returning from our own exception handling. We might want to put something here if we are using a
// custom version of
// this, such as Google Breakpad.
#endif
}

void ExceptionHandler::SetExceptionListener(
    ExceptionListener* pExceptionListener,
    uintptr_t userValue) {
  exceptionListener = pExceptionListener;
  exceptionListenerUserValue = userValue;
}

void ExceptionHandler::SetAppDescription(const char* pAppDescription) {
  appDescription = pAppDescription;
}

void ExceptionHandler::SetExceptionPaths(
    const char* exceptionReportPath,
    const char* exceptionMiniDumpFilePath) {
  char tempPath[OVR_MAX_PATH];

  if (exceptionReportPath) {
    if (OVR_stricmp(exceptionReportPath, "default") == 0) {
      GetCrashDumpDirectory(tempPath, OVR_ARRAY_COUNT(tempPath));
      OVR::OVR_strlcat(tempPath, "Exception Report (%s).txt", OVR_ARRAY_COUNT(tempPath));
      exceptionReportPath = tempPath;
    }

    OVR_strlcpy(reportFilePath, exceptionReportPath, OVR_ARRAY_COUNT(reportFilePath));
  } else {
    reportFilePath[0] = '\0';
  }

  if (exceptionMiniDumpFilePath) {
    if (OVR_stricmp(exceptionMiniDumpFilePath, "default") == 0) {
      GetCrashDumpDirectory(tempPath, OVR_ARRAY_COUNT(tempPath));
      OVR::OVR_strlcat(tempPath, "Exception Minidump (%s).mdmp", OVR_ARRAY_COUNT(tempPath));
      exceptionMiniDumpFilePath = tempPath;
    }

    OVR_strlcpy(miniDumpFilePath, exceptionMiniDumpFilePath, OVR_ARRAY_COUNT(miniDumpFilePath));
  } else {
    miniDumpFilePath[0] = '\0';
  }
}

void ExceptionHandler::SetCodeBaseDirectoryPaths(
    const char* codeBaseDirectoryPathArray[],
    size_t codeBaseDirectoryPathCount) {
  for (size_t i = 0,
              iCount = OVR::Alg::Min<size_t>(
                  codeBaseDirectoryPathCount, OVR_ARRAY_COUNT(codeBasePathArray));
       i != iCount;
       ++i) {
    codeBasePathArray[i] = codeBaseDirectoryPathArray[i];
  }
}

const char* ExceptionHandler::GetExceptionUIText(const char* exceptionReportPath) {
  char* uiText = nullptr;
  OVR::SysFile file(exceptionReportPath, SysFile::Open_Read, SysFile::Mode_ReadWrite);

  if (file.IsValid()) {
    size_t length = (size_t)file.GetLength();
    uiText = (char*)OVR::SafeMMapAlloc(length + 1);

    if (uiText) {
      file.Read((uint8_t*)uiText, (int)length);
      uiText[length] = '\0';
      file.Close();

// Currently on Mac our message box implementation is unable to display arbitrarily large amounts of
// text.
// So we reduce its size to a more summary version before presenting.
#if defined(OVR_OS_MAC)
      struct Find {
        static char* PreviousChar(char* p, char c) {
          while (*p != c)
            p--;
          return p;
        }
      }; // Assumes the given c is present prior to p.

      // Print that the full report is at <file path>
      // Exception Info section
      // Exception thread callstack.
      char empty[] = "";
      char* pExceptionInfoBegin =
          strstr(uiText, "Exception Info") ? strstr(uiText, "Exception Info") : empty;
      char* pExceptionInfoEnd =
          (pExceptionInfoBegin == empty) ? (empty + 1) : strstr(uiText, "\n\n");
      char* pExceptionThreadArea = strstr(uiText, ", exception thread");
      char* pExceptionThreadBegin =
          pExceptionThreadArea ? Find::PreviousChar(pExceptionThreadArea, '\n') + 1 : empty;
      char* pExceptionThreadEnd =
          (pExceptionThreadBegin == empty) ? (empty + 1) : strstr(pExceptionThreadArea, "\n\n");

      if (!pExceptionInfoEnd)
        pExceptionInfoEnd = pExceptionInfoBegin;
      *pExceptionInfoEnd = '\0';

      if (!pExceptionThreadEnd)
        pExceptionThreadEnd = pExceptionThreadBegin;
      *pExceptionThreadEnd = '\0';

      size_t uiTextBriefLength = snprintf(
          nullptr,
          0,
          "Full report:%s\n\nSummary report:\n%s\n\n%s",
          exceptionReportPath,
          pExceptionInfoBegin,
          pExceptionThreadBegin);
      char* uiTextBrief = (char*)OVR::SafeMMapAlloc(uiTextBriefLength + 1);

      if (uiTextBrief) {
        snprintf(
            uiTextBrief,
            uiTextBriefLength + 1,
            "Full report:%s\n\nSummary report:\n%s\n\n%s",
            exceptionReportPath,
            pExceptionInfoBegin,
            pExceptionThreadBegin);
        OVR::SafeMMapFree(uiText, length);
        uiText = uiTextBrief;
      }
#endif
    }
  }

  return uiText;
}

void ExceptionHandler::FreeExceptionUIText(const char* messageBoxText) {
  OVR::SafeMMapFree(messageBoxText, OVR_strlen(messageBoxText));
}

void ExceptionHandler::ReportDeadlock(
    const char* threadName,
    const char* organizationName,
    const char* applicationName) {
  if (!organizationName || !organizationName[0])
    organizationName = "Oculus";
  if (!applicationName || !applicationName[0])
    applicationName = "DefaultApp";

  ExceptionHandler handler;

  handler.SetPathsFromNames(
      organizationName, applicationName, "Deadlock Report (%s).txt", "Deadlock Minidump (%s).mdmp");

  snprintf(
      handler.exceptionInfo.exceptionDescription,
      sizeof(handler.exceptionInfo.exceptionDescription),
      "Deadlock in thread '%s'",
      threadName ? threadName : "(null)");

  handler.exceptionInfo.timeVal = time(nullptr);
  handler.exceptionInfo.time = *gmtime(&handler.exceptionInfo.timeVal);
  handler.WriteReport("Deadlock");
  handler.WriteMiniDump();
}

//-----------------------------------------------------------------------------
// GUI Exception Listener

GUIExceptionListener::GUIExceptionListener() : Handler() {}

int GUIExceptionListener::HandleException(
    uintptr_t userValue,
    ExceptionHandler* pExceptionHandler,
    ExceptionInfo* pExceptionInfo,
    const char* reportFilePath) {
  OVR_UNUSED3(userValue, pExceptionHandler, pExceptionInfo);

  // If debugger is not present,
  if (!OVRIsDebuggerPresent()) {
    const char* uiText = ExceptionHandler::GetExceptionUIText(reportFilePath);
    if (uiText) {
      OVR::Util::DisplayMessageBox("Exception occurred", uiText);
      ExceptionHandler::FreeExceptionUIText(uiText);
    }
  }
  return 0;
}

} // namespace OVR

OVR_RESTORE_MSVC_WARNING()
