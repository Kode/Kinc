#include "pch.h"
#include "Reader.h"
#include <Kore/Math/Core.h>
#include <cstdlib>
#include <cstring>
#include <stdio.h>

using namespace Kore;

float Reader::readF32LE(u8* data) {
#ifdef SYS_LITTLE_ENDIAN //speed optimization
	return *(float*)data;
#else //works on all architectures
	int i = (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	return *(float*)&i;
#endif
}

float Reader::readF32BE(u8* data) {
#ifdef SYS_BIG_ENDIAN //speed optimization
	return *(float*)data;
#else //works on all architectures
	int i = (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
	return *(float*)&i;
#endif
}

u64 Reader::readU64LE(u8* data) {
#ifdef SYS_LITTLE_ENDIAN
	return *(u64*)data;
#else
	return ((u64)data[0] << 0) | ((u64)data[1] << 8) | ((u64)data[2] << 16) | ((u64)data[3] << 24) | ((u64)data[4] << 32) | ((u64)data[5] << 40) | ((u64)data[6] << 48) | ((u64)data[7] << 56);
#endif
}

u64 Reader::readU64BE(u8* data) {
#ifdef SYS_BIG_ENDIAN
	return *(u64*)data;
#else
	return ((u64)data[7] << 0) | ((u64)data[6] << 8) | ((u64)data[5] << 16) | ((u64)data[4] << 24) | ((u64)data[3] << 32) | ((u64)data[2] << 40) | ((u64)data[1] << 48) | ((u64)data[0] << 56);
#endif
}

s64 Reader::readS64LE(u8* data) {
#ifdef SYS_LITTLE_ENDIAN
	return *(s64*)data;
#else
	return ((s64)data[0] << 0) | ((s64)data[1] << 8) | ((s64)data[2] << 16) | ((s64)data[3] << 24) | ((s64)data[4] << 32) | ((s64)data[5] << 40) | ((s64)data[6] << 48) | ((s64)data[7] << 56);
#endif
}

s64 Reader::readS64BE(u8* data) {
#ifdef SYS_BIG_ENDIAN
	return *(s64*)data;
#else
	return ((s64)data[7] << 0) | ((s64)data[6] << 8) | ((s64)data[5] << 16) | ((s64)data[4] << 24) | ((s64)data[3] << 32) | ((s64)data[2] << 40) | ((s64)data[1] << 48) | ((s64)data[0] << 56);
#endif
}

u32 Reader::readU32LE(u8* data) {
#ifdef SYS_LITTLE_ENDIAN
	return *(u32*)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

u32 Reader::readU32BE(u8* data) {
#ifdef SYS_BIG_ENDIAN
	return *(u32*)data;
#else
	return (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
#endif
}

s32 Reader::readS32LE(u8* data) {
#ifdef SYS_LITTLE_ENDIAN
	return *(s32*)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

s32 Reader::readS32BE(u8* data) {
#ifdef SYS_BIG_ENDIAN
	return *(s32*)data;
#else
	return (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
#endif
}

u16 Reader::readU16LE(u8* data) {
#ifdef SYS_LITTLE_ENDIAN
	return *(u16*)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

u16 Reader::readU16BE(u8* data) {
#ifdef SYS_BIG_ENDIAN
	return *(u16*)data;
#else
	return (data[1] << 0) | (data[0] << 8);
#endif
}

s16 Reader::readS16LE(u8* data) {
#ifdef SYS_LITTLE_ENDIAN
	return *(s16*)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

s16 Reader::readS16BE(u8* data) {
#ifdef SYS_BIG_ENDIAN
	return *(s16*)data;
#else
	return (data[1] << 0) | (data[0] << 8);
#endif
}

u8 Reader::readU8(u8* data) {
	return *data;
}

s8 Reader::readS8(u8* data) {
	return *(s8*)data;
}

float Reader::readF32LE() {
	u8 data[4];
	read(data, 4);
	return readF32LE(&data[0]);
}

float Reader::readF32BE() {
	u8 data[4];
	read(data, 4);
	return readF32BE(&data[0]);
}

u64 Reader::readU64LE() {
	u8 data[8];
	read(data, 8);
	return readU64LE(&data[0]);
}

u64 Reader::readU64BE() {
	u8 data[8];
	read(data, 8);
	return readU64BE(&data[0]);
}

s64 Reader::readS64LE() {
	u8 data[8];
	read(data, 8);
	return readS64LE(&data[0]);
}

s64 Reader::readS64BE() {
	u8 data[8];
	read(data, 8);
	return readS64BE(&data[0]);
}

u32 Reader::readU32LE() {
	u8 data[4];
	read(data, 4);
	return readU32LE(&data[0]);
}

u32 Reader::readU32BE() {
	u8 data[4];
	read(data, 4);
	return readU32BE(&data[0]);
}

s32 Reader::readS32LE() {
	u8 data[4];
	read(data, 4);
	return readS32LE(&data[0]);
}

s32 Reader::readS32BE() {
	u8 data[4];
	read(data, 4);
	return readS32BE(&data[0]);
}

u16 Reader::readU16LE() {
	u8 data[2];
	read(data, 2);
	return readU16LE(&data[0]);
}

u16 Reader::readU16BE() {
	u8 data[2];
	read(data, 2);
	return readU16BE(&data[0]);
}

s16 Reader::readS16LE() {
	u8 data[2];
	read(data, 2);
	return readS16LE(&data[0]);
}

s16 Reader::readS16BE() {
	u8 data[2];
	read(data, 2);
	return readS16BE(&data[0]);
}

u8 Reader::readU8() {
	u8 data;
	read(&data, 1);
	return data;
}

s8 Reader::readS8() {
	s8 data;
	read(&data, 1);
	return data;
}
