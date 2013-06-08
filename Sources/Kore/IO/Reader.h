#pragma once

namespace Kore {
	class Reader {
	public:
		virtual ~Reader() { }
		virtual int read(void* data, int size) = 0;
		virtual void* readAll() = 0;
		virtual int size() const = 0;
		virtual int pos() const = 0;
		virtual void seek(int pos) = 0;

		float readF32LE();
		float readF32BE();
		u32 readU32LE();
		u32 readU32BE();
		s32 readS32LE();
		s32 readS32BE();
		u16 readU16LE();
		u16 readU16BE();
		s16 readS16LE();
		s16 readS16BE();
		u8 readU8();
		s8 readS8();

		static float readF32LE(u8* data);
		static float readF32BE(u8* data);
		static u32 readU32LE(u8* data);
		static u32 readU32BE(u8* data);
		static s32 readS32LE(u8* data);
		static s32 readS32BE(u8* data);
		static u16 readU16LE(u8* data);
		static u16 readU16BE(u8* data);
		static s16 readS16LE(u8* data);
		static s16 readS16BE(u8* data);
	};
}
