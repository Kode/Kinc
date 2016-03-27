#pragma once

namespace Kore {
	typedef unsigned char      u8;  // 1 Byte
	typedef unsigned short     u16; // 2 Byte
	typedef unsigned int       u32; // 4 Byte

#if defined(__LP64__) || defined(_LP64)
#define SYS_64
#endif

#ifdef SYS_WINDOWS
	typedef unsigned __int64   u64; // 8 Byte
#else
	typedef unsigned long long u64;
#endif
	typedef char               s8;  // 1 Byte
	typedef short              s16; // 2 Byte
	typedef int                s32; // 4 Byte
#ifdef SYS_WINDOWS
	typedef __int64            s64; // 8 Byte
#else
	typedef long long          s64;
#endif

typedef u32                uint; // 4 Byte
typedef s32                sint; // 4 Byte

#ifdef SYS_64
	typedef s64 spint;
	typedef u64 upint;
#else
	typedef s32 spint;
	typedef u32 upint;
#endif
}

#if defined(SYS_PPC)
#define SYS_BIG_ENDIAN
#else
#define SYS_LITTLE_ENDIAN
#endif

//pseudo C++11
#if !defined(SYS_WINDOWS) && !defined(SYS_WINDOWSAPP) && __cplusplus <= 199711L
#define nullptr 0
#define override
#endif

#define Noexcept throw()
