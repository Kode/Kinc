#pragma once

#ifdef KORE_PS4
#include <Kore/FileReaderImpl.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KORE_DEBUGDIR
#define KORE_DEBUGDIR "Deployment"
#endif

#ifdef KORE_ANDROID
struct AAsset;
struct __sFILE;
typedef struct __sFILE FILE;
#endif

#define KINC_FILE_TYPE_ASSET 0
#define KINC_FILE_TYPE_SAVE 1

#ifdef KORE_ANDROID
typedef struct {
	int pos;
	int size;
	FILE *file;
	struct AAsset *asset;
	int type;
} kinc_file_reader_t;
#else
typedef struct kinc_file_reader {
	void *file;
	int size;
	int type;
	int mode;
	bool mounted;
#ifdef KORE_PS4
	kinc_file_reader_impl_t impl;
#endif
} kinc_file_reader_t;
#endif

bool kinc_file_reader_open(kinc_file_reader_t *reader, const char *filename, int type);
void kinc_file_reader_close(kinc_file_reader_t *reader);
int kinc_file_reader_read(kinc_file_reader_t *reader, void *data, size_t size);
size_t kinc_file_reader_size(kinc_file_reader_t *reader);
int kinc_file_reader_pos(kinc_file_reader_t *reader);
void kinc_file_reader_seek(kinc_file_reader_t *reader, int pos);

float    kinc_read_f32le(uint8_t *data);
float    kinc_read_f32be(uint8_t *data);
uint64_t kinc_read_u64le(uint8_t *data);
uint64_t kinc_read_u64be(uint8_t *data);
int64_t  kinc_read_s64le(uint8_t *data);
int64_t  kinc_read_s64be(uint8_t *data);
uint32_t kinc_read_u32le(uint8_t *data);
uint32_t kinc_read_u32be(uint8_t *data);
int32_t  kinc_read_s32le(uint8_t *data);
int32_t  kinc_read_s32be(uint8_t *data);
uint16_t kinc_read_u16le(uint8_t *data);
uint16_t kinc_read_u16be(uint8_t *data);
int16_t  kinc_read_s16le(uint8_t *data);
int16_t  kinc_read_s16be(uint8_t *data);
uint8_t  kinc_read_u8(uint8_t *data);
int8_t   kinc_read_s8(uint8_t *data);

void kinc_internal_set_files_location(char *dir);
char *kinc_internal_get_files_location();

#ifdef __cplusplus
}
#endif
