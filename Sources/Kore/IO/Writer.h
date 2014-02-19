#pragma once

namespace Kore {
	class Writer {
	public:
		virtual ~Writer() { }
		virtual void write(void* data, int size) = 0;

		void writeLE(float value);
		void writeBE(float value);
		void writeU32LE(u32 value);
		void writeU32BE(u32 value);
		void writeS32LE(s32 value);
		void writeS32BE(s32 value);
		void writeU16LE(u16 value);
		void writeU16BE(u16 value);
		void writeS16LE(s16 value);
		void writeS16BE(s16 value);
		void writeU8(u8 value);
		void writeS8(s8 value);

		static void writeLE(float value, u8* data);
		static void writeBE(float value, u8* data);
		static void writeLE(u32 value, u8* data);
		static void writeBE(u32 value, u8* data);
		static void writeLE(s32 value, u8* data);
		static void writeBE(s32 value, u8* data);
		static void writeLE(u16 value, u8* data);
		static void writeBE(u16 value, u8* data);
		static void writeLE(s16 value, u8* data);
		static void writeBE(s16 value, u8* data);
	};
}
