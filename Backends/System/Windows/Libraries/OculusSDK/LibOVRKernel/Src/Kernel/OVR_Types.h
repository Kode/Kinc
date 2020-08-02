/************************************************************************************

Filename    :   OVR_Types.h
Content     :   Standard library defines and simple types
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

#ifndef OVR_Types_h
#define OVR_Types_h

#include <cstddef>
#include "OVR_Compiler.h"

#if defined(_WIN32) && defined(OVR_KILL_WINDOWS_A_FUNCTIONS)
#include "OVR_Win32_IncludeWindows.h"
#include <commctrl.h>
#pragma warning(push)
#pragma warning(disable : 4091)
#include <DbgHelp.h>
#pragma warning(pop)
#include <ShlObj.h>
#include <Shlwapi.h>
#include <Sddl.h>
#include <WtsApi32.h>
#include <SetupAPI.h>
#include <Cfgmgr32.h>
#include <Psapi.h>
#include "WindowsAFunctions.h"
#endif

// Unsupported compiler configurations
#if _MSC_VER == 0x1600
#if _MSC_FULL_VER < 160040219
#error "Oculus does not support VS2010 without SP1 installed: It will crash in Release mode"
#endif
#endif

//-----------------------------------------------------------------------------------
// ****** Operating system identification
//
// Try to use the most generic version of these defines as possible in order to achieve
// the simplest portable code. For example, instead of using #if (defined(OVR_OS_IPHONE) ||
// defined(OVR_OS_MAC)),
// consider using #if defined(OVR_OS_APPLE).
//
// Type definitions exist for the following operating systems: (OVR_OS_x)
//
//    WIN32      - Win32 and Win64 (Windows XP and later) Does not include Microsoft phone and
//    console platforms, despite that Microsoft's _WIN32 may be defined by the compiler for them.
//    WIN64      - Win64 (Windows XP and later)
//    MAC        - Mac OS X (may be defined in addition to BSD)
//    LINUX      - Linux
//    BSD        - BSD Unix
//    ANDROID    - Android (may be defined in addition to LINUX)
//    IPHONE     - iPhone
//    MS_MOBILE  - Microsoft mobile OS.
//
//  Meta platforms
//    MS        - Any OS by Microsoft (e.g. Win32, Win64, phone, console)
//    APPLE     - Any OS by Apple (e.g. iOS, OS X)
//    UNIX      - Linux, BSD, Mac OS X.
//    MOBILE    - iOS, Android, Microsoft phone
//    CONSOLE   - Console platforms.
//

#if (defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))) || \
    defined(__MACOS__)
#if (                                                          \
    defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) || \
    defined(__IPHONE_OS_VERSION_MIN_REQUIRED))
#if !defined(OVR_OS_IPHONE)
#define OVR_OS_IPHONE
#endif
#else
#if !defined(OVR_OS_MAC)
#define OVR_OS_MAC
#endif
#if !defined(OVR_OS_DARWIN)
#define OVR_OS_DARWIN
#endif
#if !defined(OVR_OS_BSD)
#define OVR_OS_BSD
#endif
#endif
#elif (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#if !defined(OVR_OS_WIN64)
#define OVR_OS_WIN64
#endif
#if !defined(OVR_OS_WIN32)
#define OVR_OS_WIN32 // Can be a 32 bit Windows build or a WOW64 support for Win32.  In this case
// WOW64 support for Win32.
#endif
#elif (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#if !defined(OVR_OS_WIN32)
#define OVR_OS_WIN32 // Can be a 32 bit Windows build or a WOW64 support for Win32.  In this case
// WOW64 support for Win32.
#endif
#elif defined(ANDROID) || defined(__ANDROID__)
#if !defined(OVR_OS_ANDROID)
#define OVR_OS_ANDROID
#endif
#if !defined(OVR_OS_LINUX)
#define OVR_OS_LINUX
#endif
#elif defined(__linux__) || defined(__linux)
#if !defined(OVR_OS_LINUX)
#define OVR_OS_LINUX
#endif
#elif defined(_BSD_) || defined(__FreeBSD__)
#if !defined(OVR_OS_BSD)
#define OVR_OS_BSD
#endif
#else
#if !defined(OVR_OS_OTHER)
#define OVR_OS_OTHER
#endif
#endif

#if !defined(OVR_OS_MS_MOBILE)
#if (defined(_M_ARM) || defined(_M_IX86) || defined(_M_AMD64)) && !defined(OVR_OS_WIN32) && \
    !defined(OVR_OS_CONSOLE)
#define OVR_OS_MS_MOBILE
#endif
#endif

#if !defined(OVR_OS_MS)
#if defined(OVR_OS_WIN32) || defined(OVR_OS_WIN64) || defined(OVR_OS_MS_MOBILE)
#define OVR_OS_MS
#endif
#endif

#if !defined(OVR_OS_APPLE)
#if defined(OVR_OS_MAC) || defined(OVR_OS_IPHONE)
#define OVR_OS_APPLE
#endif
#endif

#if !defined(OVR_OS_UNIX)
#if defined(OVR_OS_ANDROID) || defined(OVR_OS_BSD) || defined(OVR_OS_LINUX) || defined(OVR_OS_MAC)
#define OVR_OS_UNIX
#endif
#endif

#if !defined(OVR_OS_MOBILE)
#if defined(OVR_OS_ANDROID) || defined(OVR_OS_IPHONE) || defined(OVR_OS_MS_MOBILE)
#define OVR_OS_MOBILE
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** CPU Architecture
//
// The following CPUs are defined: (OVR_CPU_x)
//
//    X86        - x86 (IA-32)
//    X86_64     - x86_64 (amd64)
//    PPC        - PowerPC
//    PPC64      - PowerPC64
//    MIPS       - MIPS
//    OTHER      - CPU for which no special support is present or needed

#if defined(__x86_64__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || \
    defined(_M_AMD64)
#define OVR_CPU_X86_64
#define OVR_64BIT_POINTERS
#elif defined(__i386__) || defined(OVR_OS_WIN32)
#define OVR_CPU_X86
#elif defined(__powerpc64__)
#define OVR_CPU_PPC64
#elif defined(__ppc__)
#define OVR_CPU_PPC
#elif defined(__mips__) || defined(__MIPSEL__)
#define OVR_CPU_MIPS
#elif defined(__arm__)
#define OVR_CPU_ARM
#else
#define OVR_CPU_OTHER
#endif

//-----------------------------------------------------------------------------------
// ***** Co-Processor Architecture
//
// The following co-processors are defined: (OVR_CPU_x)
//
//    SSE        - Available on all modern x86 processors.
//    Altivec    - Available on all modern ppc processors.
//    Neon       - Available on some armv7+ processors.

#if defined(__SSE__) || defined(_M_IX86) || \
    defined(_M_AMD64) // _M_IX86 and _M_AMD64 are Microsoft identifiers for Intel-based platforms.
#define OVR_CPU_SSE
#endif // __SSE__

#if defined(__ALTIVEC__)
#define OVR_CPU_ALTIVEC
#endif // __ALTIVEC__

#if defined(__ARM_NEON__)
#define OVR_CPU_ARM_NEON
#endif // __ARM_NEON__

//-----------------------------------------------------------------------------------
// ***** Compiler Warnings

// Disable MSVC warnings
#if defined(OVR_CC_MSVC)
#pragma warning(disable : 4127) // Conditional expression is constant
#pragma warning(disable : 4530) // Exception handling
#if (OVR_CC_MSVC < 1300)
#pragma warning(disable : 4514) // Unreferenced inline function has been removed
#pragma warning(disable : 4710) // Function not inlined
#pragma warning(disable : 4714) // _force_inline not inlined
#pragma warning(disable : 4786) // Debug variable name longer than 255 chars
#endif // (OVR_CC_MSVC<1300)
#if (OVR_CC_MSVC > 1800)
#pragma warning(disable : 4265) // Class has virtual functions, but destructor is not virtual
#pragma warning(disable : 4312) // Conversion from 'type1' to 'type2' of greater size
#pragma warning(disable : 4311) // Pointer truncation from 'type' to 'type'
#pragma warning(disable : 4302) // Truncation from 'type 1' to 'type 2'
#endif // OVR_CC_MSVC>1800
#endif // (OVR_CC_MSVC)

// *** Linux Unicode - must come before Standard Includes

#ifdef OVR_OS_LINUX
// Use glibc unicode functions on linux.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** Standard Includes
//
#include <stddef.h>
#include <limits.h>
#include <float.h>

// MSVC Based Memory Leak checking - for now
#if defined(OVR_CC_MSVC) && defined(OVR_BUILD_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

//-----------------------------------------------------------------------------------
// ***** int8_t, int16_t, etc.

#if defined(OVR_CC_MSVC) && (OVR_CC_VERSION <= 1500) // VS2008 and earlier
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

//-----------------------------------------------------------------------------------
// ***** Type definitions for Common Systems

namespace OVR {

typedef char Char;

// Pointer-sized integer
typedef size_t UPInt;
typedef std::ptrdiff_t SPInt;

#if defined(OVR_OS_MS)

typedef char SByte; // 8 bit Integer (Byte)
typedef unsigned char UByte;
typedef short SInt16; // 16 bit Integer (Word)
typedef unsigned short UInt16;
typedef long SInt32; // 32 bit Integer
typedef unsigned long UInt32;
typedef __int64 SInt64; // 64 bit Integer (QWord)
typedef unsigned __int64 UInt64;

#elif defined(OVR_OS_MAC) || defined(OVR_OS_IPHONE) || defined(OVR_CC_GNU)

typedef int SByte __attribute__((__mode__(__QI__)));
typedef unsigned int UByte __attribute__((__mode__(__QI__)));
typedef int SInt16 __attribute__((__mode__(__HI__)));
typedef unsigned int UInt16 __attribute__((__mode__(__HI__)));
typedef int SInt32 __attribute__((__mode__(__SI__)));
typedef unsigned int UInt32 __attribute__((__mode__(__SI__)));
typedef int SInt64 __attribute__((__mode__(__DI__)));
typedef unsigned int UInt64 __attribute__((__mode__(__DI__)));

#else

#include <sys/types.h>
typedef int8_t SByte;
typedef uint8_t UByte;
typedef int16_t SInt16;
typedef uint16_t UInt16;
typedef int32_t SInt32;
typedef uint32_t UInt32;
typedef int64_t SInt64;
typedef uint64_t UInt64;

#endif

// osx PID is a signed int32 (already defined to pid_t in OSX framework)
// linux PID is a signed int32 (already defined)
// win32 PID is an unsigned int32
#ifdef OVR_OS_WIN32
// process ID representation
typedef unsigned long pid_t;
#endif

// OVR_INVALID_PID defines an invalid process id in a portable way.
#if !defined(OVR_INVALID_PID)
#define OVR_INVALID_PID 0
#endif

struct OVR_GUID {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
};

} // namespace OVR

//-----------------------------------------------------------------------------------
// ****** Standard C/C++ Library
//
// Identifies which standard library is currently being used.
//
//    LIBSTDCPP   - GNU libstdc++, used by GCC.
//    LIBCPP      - LLVM libc++, typically used by clang and GCC.
//    DINKUMWARE  - Used by Microsoft and various non-Microsoft compilers (e.g. Sony clang).

#if !defined(OVR_STDLIB_LIBSTDCPP)
#if defined(__GLIBCXX__)
#define OVR_STDLIB_LIBSTDCPP 1
#endif
#endif

#if !defined(OVR_STDLIB_LIBCPP)
#if defined(__clang__)
#if defined(__cplusplus) && __has_include(<__config>)
#define OVR_STDLIB_LIBCPP 1
#endif
#endif
#endif

#if !defined(OVR_STDLIB_DINKUMWARE)
#if defined(_YVALS) // Dinkumware globally #defines _YVALS from the #includes above.
#define OVR_STDLIB_DINKUMWARE 1
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** Macro Definitions
//
// We define the following:
//
//  OVR_BYTE_ORDER      - Defined to either OVR_LITTLE_ENDIAN or OVR_BIG_ENDIAN
//  OVR_FORCE_INLINE    - Forces inline expansion of function
//  OVR_ASM             - Assembly language prefix
//  OVR_STR             - Prefixes string with L"" if building unicode
//
//  OVR_STDCALL         - Use stdcall calling convention (Pascal arg order)
//  OVR_CDECL           - Use cdecl calling convention (C argument order)
//  OVR_FASTCALL        - Use fastcall calling convention (registers)
//

// Byte order constants, OVR_BYTE_ORDER is defined to be one of these.
#define OVR_LITTLE_ENDIAN 1
#define OVR_BIG_ENDIAN 2

#if defined(OVR_OS_MS)

// ***** Windows and non-desktop platforms

// Byte order
#define OVR_BYTE_ORDER OVR_LITTLE_ENDIAN

// Calling convention - goes after function return type but before function name
#ifdef __cplusplus_cli
#define OVR_FASTCALL __stdcall
#else
#define OVR_FASTCALL __fastcall
#endif

#define OVR_STDCALL __stdcall
#define OVR_CDECL __cdecl

// Assembly macros
#if defined(OVR_CC_MSVC)
#define OVR_ASM _asm
#else
#define OVR_ASM asm
#endif // (OVR_CC_MSVC)

#ifdef UNICODE
#define OVR_STR(str) L##str
#else
#define OVR_STR(str) str
#endif // UNICODE

#else

// **** Standard systems

#if (defined(BYTE_ORDER) && (BYTE_ORDER == BIG_ENDIAN)) || \
    (defined(_BYTE_ORDER) && (_BYTE_ORDER == _BIG_ENDIAN))
#define OVR_BYTE_ORDER OVR_BIG_ENDIAN
#elif (defined(__ARMEB__) || defined(OVR_CPU_PPC) || defined(OVR_CPU_PPC64))
#define OVR_BYTE_ORDER OVR_BIG_ENDIAN
#else
#define OVR_BYTE_ORDER OVR_LITTLE_ENDIAN
#endif

// Assembly macros
#define OVR_ASM __asm__
#define OVR_ASM_PROC(procname) OVR_ASM
#define OVR_ASM_END OVR_ASM

// Calling convention - goes after function return type but before function name
#define OVR_FASTCALL
#define OVR_STDCALL
#define OVR_CDECL

#endif // defined(OVR_OS_WIN32)

//-----------------------------------------------------------------------------------
// ***** OVR_PTR_SIZE
//
// Specifies the byte size of pointers (same as sizeof void*).

#if !defined(OVR_PTR_SIZE)
#if defined(__WORDSIZE)
#define OVR_PTR_SIZE ((__WORDSIZE) / 8)
#elif defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(_M_IA64) || \
    defined(__ia64__) || defined(__arch64__) || defined(__64BIT__) || defined(__Ptr_Is_64)
#define OVR_PTR_SIZE 8
#elif defined(__CC_ARM) && (__sizeof_ptr == 8)
#define OVR_PTR_SIZE 8
#else
#define OVR_PTR_SIZE 4
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_WORD_SIZE
//
// Specifies the byte size of a machine word/register. Not necessarily the same as
// the size of pointers, but usually >= the size of pointers.

#if !defined(OVR_WORD_SIZE)
#define OVR_WORD_SIZE OVR_PTR_SIZE // For our currently supported platforms these are equal.
#endif

// ------------------------------------------------------------------------
// ***** OVR_CONST_FLOAT / OVR_CONST_DOUBLE
//
// Used for defining a const float, typically in a header file. The resulting
// variable has external linkage and thus any other declarations of variables
// with the same name must be of an identical value.
//
// Example usage:
//     OVR_CONST_FLOAT  kPi  = 3.14159265f;
//     OVR_CONST_DOUBLE kOne = 1.0;

#if defined(_MSC_VER)
#define OVR_CONST_FLOAT extern const __declspec(selectany) float
#define OVR_CONST_DOUBLE extern const __declspec(selectany) double
#else
#define OVR_CONST_FLOAT __attribute__((weak)) extern const float
#define OVR_CONST_DOUBLE __attribute__((weak)) extern const double
#endif

// ------------------------------------------------------------------------
// ***** OVR_FORCE_INLINE
//
// Force inline substitute - goes before function declaration
// Example usage:
//     OVR_FORCE_INLINE void Test();

#if !defined(OVR_FORCE_INLINE)
#if defined(OVR_CC_MSVC)
#define OVR_FORCE_INLINE __forceinline
#elif defined(OVR_CC_GNU)
#define OVR_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define OVR_FORCE_INLINE inline
#endif // OVR_CC_MSVC
#endif

// ------------------------------------------------------------------------
// ***** OVR_NO_INLINE
//
// Cannot be used with inline or OVR_FORCE_INLINE.
// Example usage:
//     OVR_NO_INLINE void Test();

#if !defined(OVR_NO_INLINE)
#if defined(OVR_CC_MSVC) && (_MSC_VER >= 1500) // VS2008+
#define OVR_NO_INLINE __declspec(noinline)
#elif !defined(OVR_CC_MSVC)
#define OVR_NO_INLINE __attribute__((noinline))
#endif
#endif

// -----------------------------------------------------------------------------------
// ***** OVR_STRINGIZE
//
// Converts a preprocessor symbol to a string.
//
// Example usage:
//     printf("Line: %s", OVR_STRINGIZE(__LINE__));
//
#if !defined(OVR_STRINGIZE)
#define OVR_STRINGIZEIMPL(x) #x
#define OVR_STRINGIZE(x) OVR_STRINGIZEIMPL(x)
#endif

// -----------------------------------------------------------------------------------
// ***** OVR_JOIN
//
// Joins two preprocessing symbols together. Supports the case when either or the
// the symbols are macros themselves.
//
// Example usage:
//    char OVR_JOIN(unique_, __LINE__);  // Results in (e.g.) char unique_123;
//
#if !defined(OVR_JOIN)
#define OVR_JOIN(a, b) OVR_JOIN1(a, b)
#define OVR_JOIN1(a, b) OVR_JOIN2(a, b)
#define OVR_JOIN2(a, b) a##b
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_OFFSETOF
//
// Portable implementation of offsetof for structs and classes. offsetof and GCC's
// __builtin_offsetof work only with POD types (standard-layout types under C++11),
// despite that it can safely work with a number of types that aren't POD. This
// version works with more types without generating compiler warnings or errors.
// Returns the offset as a size_t, as per offsetof.
//
// Example usage:
//     struct Test{ int i; float f; };
//     size_t fPos = OVR_OFFSETOF(Test, f);

#if defined(OVR_CC_GNU)
#define OVR_OFFSETOF(class_, member_)                                                       \
  ((size_t)(                                                                                \
      ((uintptr_t) & reinterpret_cast<const volatile char&>((((class_*)65536)->member_))) - \
      65536))
#else
#define OVR_OFFSETOF(class_, member_) offsetof(class_, member_)
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_SIZEOF_MEMBER
//
// Implements a portable way to determine the size of struct or class data member.
// C++11 allows this directly via sizeof (see OVR_CPP_NO_EXTENDED_SIZEOF), and this
// macro exists to handle pre-C++11 compilers.
// Returns the offset as a size_t, as per sizeof.
//
// Example usage:
//     struct Test{ int i; float f; };
//     size_t fSize = OVR_SIZEOF_MEMBER(Test, f);
//
#if defined(OVR_CPP_NO_EXTENDED_SIZEOF)
#define OVR_SIZEOF_MEMBER(class_, member_) (sizeof(((class_*)0)->member_))
#else
#define OVR_SIZEOF_MEMBER(class_, member_) (sizeof(class_::member_))
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_DEBUG_BREAK,
//       OVR_ASSERT, OVR_ASSERT_M, OVR_ASSERT_AND_UNUSED
//
// Macros have effect only in debug builds.
//
// Example OVR_DEBUG_BREAK usage (note the lack of parentheses):
//     #define MY_ASSERT(expression) do { if (!(expression)) { OVR_DEBUG_BREAK; } } while(0)
//
//
// Example OVR_ASSERT usage:
//     OVR_ASSERT(count < 100);
//     OVR_ASSERT_M(count < 100, "count is too high");
//
#if defined(OVR_BUILD_DEBUG)
// Causes a debugger breakpoint in debug builds. Has no effect in release builds.
// Microsoft Win32 specific debugging support
#if defined(OVR_CC_MSVC)
// The __debugbreak() intrinsic works for the MSVC debugger, but when the debugger
// is not attached we want our VectoredExceptionHandler to catch assertions, and
// VEH does not trap "int 3" breakpoints.
#define OVR_DEBUG_BREAK __debugbreak()
#elif defined(OVR_CC_GNU) || defined(OVR_CC_CLANG)
#if defined(OVR_CPU_X86) || defined(OVR_CPU_X86_64)
#define OVR_DEBUG_BREAK    \
  do {                     \
    OVR_ASM("int $3\n\t"); \
  } while (0)
#else
#define OVR_DEBUG_BREAK __builtin_trap()
#endif
#else
#define OVR_DEBUG_BREAK \
  do {                  \
    *((int*)0) = 1;     \
  } while (0)
#endif

// In debug builds this tests the given expression; if false then executes OVR_DEBUG_BREAK,
// if true then no action. Has no effect in release builds.
#if defined(__clang_analyzer__) // During static analysis, make it so the analyzer thinks that
// failed asserts result in program exit. Reduced false positives.
#include <stdlib.h>
#ifndef OVR_FAIL_M
#define OVR_FAIL_M(message) \
  do {                      \
    OVR_DEBUG_BREAK;        \
    exit(0);                \
  } while (0)
#endif
#ifndef OVR_FAIL
#define OVR_FAIL()   \
  do {               \
    OVR_DEBUG_BREAK; \
    exit(0);         \
  } while (0)
#endif
#ifndef OVR_ASSERT_M
#define OVR_ASSERT_M(p, message) \
  do {                           \
    if (!(p)) {                  \
      OVR_DEBUG_BREAK;           \
      exit(0);                   \
    }                            \
  } while (0)
#endif
#ifndef OVR_ASSERT
#define OVR_ASSERT(p)  \
  do {                 \
    if (!(p)) {        \
      OVR_DEBUG_BREAK; \
      exit(0);         \
    }                  \
  } while (0)
#endif
#else
#include <stdlib.h>

// OVR::IsAutomationRunning() is a flag that indicates the current process is running automated
// tests.
// Tests are usually running using GoogleTest or some other test harness
//
// Using OVR_DEBUG_BREAK will cause the test to bail out and kill the current process.
// abort() will fail the test and let subsequent tests run.
#ifndef OVR_FAIL_M
#define OVR_FAIL_M_ENABLED // Indicates that OVR_FAIL_M is enabled.
#define OVR_FAIL_M(message)                                                                        \
  {                                                                                                \
    intptr_t ovrAssertUserParam;                                                                   \
    OVR::OVRAssertionHandler ovrAssertUserHandler = OVR::GetAssertionHandler(&ovrAssertUserParam); \
                                                                                                   \
    if (OVR::IsAutomationRunning() && !OVR::OVRIsDebuggerPresent()) {                              \
      /* Do nothing to allow error code examination */                                             \
    } else if (ovrAssertUserHandler && !OVR::OVRIsDebuggerPresent()) {                             \
      ovrAssertUserHandler(ovrAssertUserParam, "Assertion failure", message);                      \
    } else {                                                                                       \
      OVR_DEBUG_BREAK;                                                                             \
    }                                                                                              \
  }
#endif

#ifndef OVR_FAIL
#define OVR_FAIL_ENABLED // Indicates that OVR_FAIL is enabled.
#define OVR_FAIL() OVR_FAIL_M("Assertion failure")
#endif

// void OVR_ASSERT_M(bool expression, const char* message);
// Note: The expression below is expanded into all usage of this assertion macro.
// We should try to minimize the size of the expanded code to the extent possible.
#ifndef OVR_ASSERT_M
#define OVR_ASSERT_M_ENABLED // Indicates that OVR_ASSERT_M is enabled.
#define OVR_ASSERT_M(p, message) \
  do {                           \
    if (!(p))                    \
      OVR_FAIL_M(message)        \
  } while (0)
#endif

// void OVR_ASSERT(bool expression);
#ifndef OVR_ASSERT
#define OVR_ASSERT_ENABLED // Indicates that OVR_ASSERT is enabled.
#define OVR_ASSERT(p) OVR_ASSERT_M((p), (#p))
#endif
#endif

// Acts the same as OVR_ASSERT in debug builds. Acts the same as OVR_UNUSED in release builds.
// Example usage: OVR_ASSERT_AND_UNUSED(x < 30, x);
#ifndef OVR_ASSERT_AND_UNUSED
#define OVR_ASSERT_AND_UNUSED(expression, value) \
  OVR_ASSERT(expression);                        \
  OVR_UNUSED(value)
#endif

#else // non-debug builds

// Causes a debugger breakpoint in debug builds. Has no effect in release builds.
#ifndef OVR_DEBUG_BREAK
#define OVR_DEBUG_BREAK ((void)0)
#endif

// In debug builds this tests the given expression; if false then executes OVR_DEBUG_BREAK,
// if true then no action. Has no effect in release builds.
#ifndef OVR_FAIL_M
#define OVR_FAIL_M(message) ((void)0)
#endif
#ifndef OVR_FAIL
#define OVR_FAIL() ((void)0)
#endif
#ifndef OVR_ASSERT_M
#define OVR_ASSERT_M(p, m) ((void)0)
#endif
#ifndef OVR_ASSERT
#define OVR_ASSERT(p) ((void)0)
#endif

// Acts the same as OVR_ASSERT in debug builds. Acts the same as OVR_UNUSED in release builds.
// Example usage: OVR_ASSERT_AND_UNUSED(x < 30, x);
#ifndef OVR_ASSERT_AND_UNUSED
#define OVR_ASSERT_AND_UNUSED(expression, value) OVR_UNUSED(value)
#endif
#endif // OVR_BUILD_DEBUG

// Assert handler
// The user of this library can override the default assertion handler and provide their own.
namespace OVR {
// The return value meaning is reserved for future definition and currently has no effect.
typedef intptr_t (*OVRAssertionHandler)(intptr_t userParam, const char* title, const char* msg);

// Returns the current assertion handler.
OVRAssertionHandler GetAssertionHandler(intptr_t* userParameter = NULL);

// Sets the current assertion handler.
// The default assertion handler if none is set simply issues a debug break.
// Example usage:
//     intptr_t CustomAssertionHandler(intptr_t /*userParameter*/, const char* title, const char*
//     message)) {
//         MessageBox(title, message);
//         OVR_DEBUG_BREAK;
//         return 0;
//     }
void SetAssertionHandler(OVRAssertionHandler assertionHandler, intptr_t userParameter = 0);

// Implements the default assertion handler.
intptr_t DefaultAssertionHandler(intptr_t userParameter, const char* title, const char* message);

// Checks if the current application is an automated test
bool IsAutomationRunning();

// Currently defined in OVR_DebugHelp.cpp
bool OVRIsDebuggerPresent();
} // namespace OVR

// ------------------------------------------------------------------------
// ***** OVR_COMPILER_ASSERT
//
// Compile-time assert; produces compiler error if condition is false.
// The expression must be a compile-time constant expression.
// This macro is deprecated in favor of static_assert, which provides better
// compiler output and works in a broader range of contexts.
//
// Example usage:
//     OVR_COMPILER_ASSERT(sizeof(int32_t == 4));

#if !defined(OVR_COMPILER_ASSERT)
#define OVR_COMPILER_ASSERT(expression) static_assert(expression, #expression)
#define OVR_COMPILER_ASSERT_M(expression, msg) static_assert(expression, msg)
#endif

// ***** OVR_PROCESSOR_PAUSE
//
// Yields the processor for other hyperthreads, usually for the purpose of implementing spins and
// spin locks.
//
// Example usage:
//     while(!finished())
//         OVR_PROCESSOR_PAUSE();

#if !defined(OVR_PROCESSOR_PAUSE)
#if defined(OVR_CPU_X86) || defined(OVR_CPU_X86_64)
#if defined(OVR_CC_GNU) || defined(OVR_CC_CLANG)
#define OVR_PROCESSOR_PAUSE() \
  asm volatile("pause" ::: "memory") // Consumes 38-40 clocks on current Intel x86 and x64 hardware.
#elif defined(OVR_CC_MSVC)
#include <emmintrin.h>
#pragma intrinsic(_mm_pause) // Maps to asm pause.
#define OVR_PROCESSOR_PAUSE _mm_pause
#else
#define OVR_PROCESSOR_PAUSE()
#endif
#else
#define OVR_PROCESSOR_PAUSE()
#endif
#endif

// ------------------------------------------------------------------------
// ***** OVR_union_cast
//
// Implements a reinterpret cast which is strict-aliasing safe. Recall that
// it's not sufficient to do a C++ reinterpret_cast or C-style cast in order
// to avoid strict-aliasing violation. The downside to this utility is that
// it works by copying the variable through a union, and this can be have
// a performance hit if the type is not small.
//
// Requires both types to be POD (plain old data), such as built-in types
// or C style structs.
//
// Example usage:
//    double  d = 1.0;
//    int64_t i = union_cast<int64_t>(d);
//
// Note that you cannot safetly use union_cast to alias the contents of two
// unrelated pointers. It can be used to alias values, not pointers to values.

namespace OVR {
template <class DestType, class SourceType>
DestType union_cast(SourceType sourceValue) {
  static_assert(sizeof(DestType) == sizeof(SourceType), "union_cast size mismatch");
  static_assert(OVR_ALIGNOF(DestType) == OVR_ALIGNOF(SourceType), "union_cast alignment mismatch");

  union SourceDest {
    SourceType sourceValue;
    DestType destValue;
  };

  SourceDest sd = {sourceValue};
  return sd.destValue;
}
} // namespace OVR

// ------------------------------------------------------------------------
// ***** OVR_VA_COPY
//
// Implements a version of C++11's va_copy that works with pre-C++11 compilers.
//
// Example usage:
//     void Printf(char* pFormat, ...)
//     {
//         va_list argList;
//         va_list argListCopy;
//
//         va_start(argList, pFormat);
//         OVR_VA_COPY(argListCopy, argList);
//         <use argList and argListCopy>
//
//         va_end(argList);
//         va_end(argListCopy);
//     }
//
#if !defined(OVR_VA_COPY)
#if defined(__GNUC__) || defined(__clang__) // GCC / clang
#define OVR_VA_COPY(dest, src) va_copy((dest), (src))
#elif defined(_MSC_VER) && (_MSC_VER >= 1800) // VS2013+
#define OVR_VA_COPY(dest, src) va_copy((dest), (src))
#else
// This may not work for some platforms, depending on their ABI.
// It works for many Microsoft platforms.
#define OVR_VA_COPY(dest, src) memcpy(&(dest), &(src), sizeof(va_list))
#endif
#endif

// ------------------------------------------------------------------------
// ***** OVR_ARRAY_COUNT
//
// Returns the element count of a C array.
//
// Example usage:
//     float itemArray[16];
//     for(size_t i = 0; i < OVR_ARRAY_COUNT(itemArray); i++) { ... }

#if defined(OVR_CPP_NO_CONSTEXPR)
#ifndef OVR_ARRAY_COUNT
#define OVR_ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))
#endif
#else
// Smarter C++11 version which knows the difference between arrays and pointers. ;
template <typename R>
struct OVRArrayCountOf_helper;

template <typename T, std::size_t N>
struct OVRArrayCountOf_helper<T[N]> {
  enum : std::size_t { size = N };
};

template <typename T, std::size_t N>
struct OVRArrayCountOf_helper<T (&)[N]> {
  enum : std::size_t { size = N };
};

template <typename T>
constexpr std::size_t OVRArrayCountOf() {
  return OVRArrayCountOf_helper<T>::size;
}

#define OVR_ARRAY_COUNT(x) OVRArrayCountOf<decltype(x)>()
#endif

// ------------------------------------------------------------------------
// ***** OVR_CURRENT_FUNCTION
//
// Portable wrapper for __PRETTY_FUNCTION__, C99 __func__, __FUNCTION__.
// This represents the most expressive version available.
// Acts as if the following were declared:
//     static const char OVR_CURRENT_FUNCTION[] = "function-name";
//
// Example usage:
//     void Test() { printf("%s", OVR_CURRENT_FUNCTION); }

#if defined(OVR_CC_GNU) || defined(OVR_CC_CLANG) || \
    (defined(__ICC) && (__ICC >= 600)) // GCC, clang, Intel
#define OVR_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__) // VC++
#define OVR_CURRENT_FUNCTION __FUNCSIG__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901) // C99 compilers
#define OVR_CURRENT_FUNCTION __func__
#else
#define OVR_CURRENT_FUNCTION __FUNCTION__
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_DEPRECATED / OVR_DEPRECATED_MSG
//
// Portably annotates a function or struct as deprecated.
// Note that clang supports __deprecated_enum_msg, which may be useful to support.
//
// Example usage:
//    OVR_DEPRECATED void Test();       // Use on the function declaration, as opposed to
//    definition.
//
//    struct OVR_DEPRECATED Test{ ... };
//
//    OVR_DEPRECATED_MSG("Test is deprecated")
//    void Test();

#if !defined(OVR_DEPRECATED)
#if defined(OVR_CC_MSVC) && (OVR_CC_VERSION > 1400) // VS2005+
#define OVR_DEPRECATED __declspec(deprecated)
#define OVR_DEPRECATED_MSG(msg) __declspec(deprecated(msg))
#elif defined(OVR_CC_CLANG) && OVR_CC_HAS_FEATURE(attribute_deprecated_with_message)
#define OVR_DEPRECATED __declspec(deprecated)
#define OVR_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#elif defined(OVR_CC_GNU) && (OVR_CC_VERSION >= 405)
#define OVR_DEPRECATED __declspec(deprecated)
#define OVR_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#elif !defined(OVR_CC_MSVC)
#define OVR_DEPRECATED __attribute__((deprecated))
#define OVR_DEPRECATED_MSG(msg) __attribute__((deprecated))
#else
#define OVR_DEPRECATED
#define OVR_DEPRECATED_MSG(msg)
#endif
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_SELECTANY
//
// Implements a wrapper for Microsoft's __declspec(selectany)
//
// Example usage:
//    SomeFile.h:
//        extern const OVR_SELECTANY float OVR_Pi = 3.14f;   // Allows you to declare floating point
//        constants in header files.
//
// Example usage:
//    SomeStruct.h:
//        struct Some {
//            static const x = 37; // See below for definition, which is required by C++11: 9.4.2p3.
//        };
//
//    SomeStruct.cpp:
//        OVR_SELECTANY const int Some::x;  // You need to use selectany here for static const
//        member definitions because the VC++ linker requires it. Other linkers do not.
//

#if !defined(OVR_SELECTANY)
#if defined(_MSC_VER)
#define OVR_SELECTANY __declspec(selectany)
#else
#define OVR_SELECTANY // selectany is similar to weak linking but not enough to attempt to use
// __attribute__((weak)) here.
#endif
#endif

// OVR_RUN_ONCE
//
// Executes a block of code only the first time it is encountered.
//
// This macro is not thread-safe. If two threads execute this simultaneously then it's
// possible that execution will occur twice.
//
// This macro will generate some code (to read a static bool) even if the block is empty,
// such as if the block goes away in a release build.
//
// C++11 has the std::call_once facility, which is similar to this but is restricted to a
// calling an individual function. It is thread-safe, however.
//
// Example usage:
//    OVR_RUN_ONCE {
//        printf("Hello world");
//    }
//
#ifndef OVR_RUN_ONCE
#define OVR_RUN_ONCE for (static bool executed = false; !executed; executed = true)
#endif

// OVR_RUN_ONCE_CONDITION
//
// This is the same as OVR_RUN_ONCE except that it evaluates a condition before considering
// whether to execute the code block. The block is executed at most once, the first time the
// condition evaluates to true.
//
// Example usage:
//    OVR_RUN_ONCE_CONDITION(time > 1000) {
//        printf("Hello world");
//    }
//
#ifndef OVR_RUN_ONCE_CONDITION
#define OVR_RUN_ONCE_CONDITION(condition) \
  for (static bool executed = false; !executed && (condition); executed = true)
#endif

//-----------------------------------------------------------------------------------
// ***** OVR_UNUSED - Unused Argument handling
// Macro to quiet compiler warnings about unused parameters/variables.
//
// Example usage:
//     void Test() {
//         int x = SomeFunction();
//         OVR_UNUSED(x);
//     }
//

#ifndef OVR_UNUSED
#if defined(OVR_CC_GNU)
#define OVR_UNUSED(a)                                  \
  do {                                                 \
    __typeof__(&a) __attribute__((unused)) __tmp = &a; \
  } while (0)
#else
#define OVR_UNUSED(a) ((void)a)
#endif

#define OVR_UNUSED1(a1) OVR_UNUSED(a1)
#define OVR_UNUSED2(a1, a2) \
  OVR_UNUSED(a1);           \
  OVR_UNUSED(a2)
#define OVR_UNUSED3(a1, a2, a3) \
  OVR_UNUSED2(a1, a2);          \
  OVR_UNUSED(a3)
#define OVR_UNUSED4(a1, a2, a3, a4) \
  OVR_UNUSED3(a1, a2, a3);          \
  OVR_UNUSED(a4)
#define OVR_UNUSED5(a1, a2, a3, a4, a5) \
  OVR_UNUSED4(a1, a2, a3, a4);          \
  OVR_UNUSED(a5)
#define OVR_UNUSED6(a1, a2, a3, a4, a5, a6) \
  OVR_UNUSED4(a1, a2, a3, a4);              \
  OVR_UNUSED2(a5, a6)
#define OVR_UNUSED7(a1, a2, a3, a4, a5, a6, a7) \
  OVR_UNUSED4(a1, a2, a3, a4);                  \
  OVR_UNUSED3(a5, a6, a7)
#define OVR_UNUSED8(a1, a2, a3, a4, a5, a6, a7, a8) \
  OVR_UNUSED4(a1, a2, a3, a4);                      \
  OVR_UNUSED4(a5, a6, a7, a8)
#define OVR_UNUSED9(a1, a2, a3, a4, a5, a6, a7, a8, a9) \
  OVR_UNUSED4(a1, a2, a3, a4);                          \
  OVR_UNUSED5(a5, a6, a7, a8, a9)
#endif

//-----------------------------------------------------------------------------------
// ***** Configuration Macros
//
// Expands to the current build type as a const char string literal.
// Acts as the following declaration: const char OVR_BUILD_STRING[];

#ifdef OVR_BUILD_DEBUG
#define OVR_BUILD_STRING "Debug"
#else
#define OVR_BUILD_STRING "Release"
#endif

//// Enables SF Debugging information
//# define OVR_BUILD_DEBUG

// OVR_DEBUG_STATEMENT injects a statement only in debug builds.
// OVR_DEBUG_SELECT injects first argument in debug builds, second argument otherwise.
#ifdef OVR_BUILD_DEBUG
#define OVR_DEBUG_STATEMENT(s) s
#define OVR_DEBUG_SELECT(d, nd) d
#else
#define OVR_DEBUG_STATEMENT(s)
#define OVR_DEBUG_SELECT(d, nd) nd
#endif

#define OVR_ENABLE_THREADS
//
// Prevents OVR from defining new within
// type macros, so developers can override
// new using the #define new new(...) trick
// - used with OVR_DEFINE_NEW macro
//# define OVR_BUILD_DEFINE_NEW
//

//-----------------------------------------------------------------------------------
// ***** Find normal allocations
//
// Our allocations are all supposed to go through the OVR System Allocator, so that
// they can be run through a game's own preferred allocator.  Occasionally we will
// accidentally introduce new code that doesn't adhere to this contract.  And it
// then becomes difficult to track down these normal allocations.  This piece of
// code makes it easy to check for normal allocations by asserting whenever they
// happen in our code.

//#define OVR_FIND_NORMAL_ALLOCATIONS
#ifdef OVR_FIND_NORMAL_ALLOCATIONS

inline void* operator new(size_t size, const char* filename, int line) {
  void* ptr = new char[size];
  OVR_ASSERT(false);
  return ptr;
}

#define new new (__FILE__, __LINE__)

#endif // OVR_FIND_NORMAL_ALLOCATIONS

#include "OVR_Nullptr.h"

#endif // OVR_Types_h
