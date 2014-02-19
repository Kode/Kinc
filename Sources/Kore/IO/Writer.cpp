#include "pch.h"
#include "Writer.h"

using namespace Kore;

void Writer::writeLE(float value) {
	u8 data[4];
	writeLE(value, &data[0]);
	write(data, 4);
}

void Writer::writeBE(float value) {
	u8 data[4];
	writeBE(value, &data[0]);
	write(data, 4);
}

void Writer::writeU32LE(u32 value) {
	u8 data[4];
	writeLE(value, &data[0]);
	write(data, 4);
}

void Writer::writeU32BE(u32 value) {
	u8 data[4];
	writeBE(value, &data[0]);
	write(data, 4);
}

void Writer::writeS32LE(s32 value) {
	u8 data[4];
	writeLE(value, &data[0]);
	write(data, 4);
}

void Writer::writeS32BE(s32 value) {
	u8 data[4];
	writeBE(value, &data[0]);
	write(data, 4);
}

void Writer::writeU16LE(u16 value) {
	u8 data[2];
	writeLE(value, &data[0]);
	write(data, 2);
}

void Writer::writeU16BE(u16 value) {
	u8 data[2];
	writeBE(value, &data[0]);
	write(data, 2);
}

void Writer::writeS16LE(s16 value) {
	u8 data[2];
	writeLE(value, &data[0]);
	write(data, 2);
}

void Writer::writeS16BE(s16 value) {
	u8 data[2];
	writeBE(value, &data[0]);
	write(data, 2);
}

void Writer::writeU8(u8 value) {
	write(&value, 1);
}

void Writer::writeS8(s8 value) {
	write(&value, 1);
}

#ifdef SYS_LITTLE_ENDIAN
#define TO_LE(size) u8* values = (u8*)&value; for (int i = 0; i < size; ++i) data[i] = values[i];
#define TO_BE(size) u8* values = (u8*)&value; for (int i = 0; i < size; ++i) data[i] = values[size - 1 - i];
#else
#define TO_LE(size) u8* values = (u8*)&value; for (int i = 0; i < size; ++i) data[i] = values[size - 1 - i];
#define TO_BE(size) u8* values = (u8*)&value; for (int i = 0; i < size; ++i) data[i] = values[i];
#endif

void Writer::writeLE(float value, u8* data) {
	TO_LE(4)
}

void Writer::writeBE(float value, u8* data) {
	TO_BE(4)
}

void Writer::writeLE(u32 value, u8* data) {
	TO_LE(4)
}

void Writer::writeBE(u32 value, u8* data) {
	TO_BE(4)
}

void Writer::writeLE(s32 value, u8* data) {
	TO_LE(4)
}

void Writer::writeBE(s32 value, u8* data) {
	TO_BE(4)
}

void Writer::writeLE(u16 value, u8* data) {
	TO_LE(2)
}

void Writer::writeBE(u16 value, u8* data) {
	TO_BE(2)
}

void Writer::writeLE(s16 value, u8* data) {
	TO_LE(2)
}

void Writer::writeBE(s16 value, u8* data) {
	TO_BE(2)
}
