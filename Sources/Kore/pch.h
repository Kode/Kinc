#pragma once

namespace Kore {
	typedef unsigned char      u8;  // 1 Byte
	typedef unsigned short     u16; // 2 Byte
	typedef unsigned int       u32; // 4 Byte
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

#ifdef SYS_OSX
	typedef s64 spint;
	typedef u64 upint;
#else
	#ifdef SYS_64
		typedef s64 spint;
		typedef u64 upint;
	#else
		typedef s32 spint;
		typedef u32 upint;
	#endif
#endif
}

#if defined SYS_XBOX360 || defined SYS_PS3
#define BIG_ENDIAN
#else
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN
#endif
#endif

//pseudo C++11
#ifndef SYS_WINDOWS
#define nullptr 0
#define override
#endif

#define Noexcept throw()
