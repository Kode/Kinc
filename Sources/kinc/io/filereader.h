#pragma once

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
typedef __sFILE FILE;
#endif

#define KINC_FILE_TYPE_ASSET 0
#define KINC_FILE_TYPE_SAVE 1

#ifdef KORE_ANDROID
typedef struct {
	int pos;
	int size;
	FILE *file;
	AAsset *asset;
	int type;
} Kinc_FileReader;
#else
typedef struct kinc_file_reader {
	void *file;
	int size;
	int type;
} kinc_file_reader_t;
#endif

bool kinc_file_reader_open(kinc_file_reader_t *reader, const char *filename, int type);
void kinc_file_reader_close(kinc_file_reader_t *reader);
int kinc_file_reader_read(kinc_file_reader_t *reader, void *data, size_t size);
size_t kinc_file_reader_size(kinc_file_reader_t *reader);
int kinc_file_reader_pos(kinc_file_reader_t *reader);
void kinc_file_reader_seek(kinc_file_reader_t *reader, int pos);

float    Kinc_ReadF32LE(uint8_t *data);
float    Kinc_ReadF32BE(uint8_t *data);
uint64_t Kinc_ReadU64LE(uint8_t *data);
uint64_t Kinc_ReadU64BE(uint8_t *data);
int64_t  Kinc_ReadS64LE(uint8_t *data);
int64_t  Kinc_ReadS64BE(uint8_t *data);
uint32_t Kinc_ReadU32LE(uint8_t *data);
uint32_t Kinc_ReadU32BE(uint8_t *data);
int32_t  Kinc_ReadS32LE(uint8_t *data);
int32_t  Kinc_ReadS32BE(uint8_t *data);
uint16_t Kinc_ReadU16LE(uint8_t *data);
uint16_t Kinc_ReadU16BE(uint8_t *data);
int16_t  Kinc_ReadS16LE(uint8_t *data);
int16_t  Kinc_ReadS16BE(uint8_t *data);
uint8_t  Kinc_ReadU8(uint8_t *data);
int8_t   Kinc_ReadS8(uint8_t *data);

void Kinc_Internal_SetFilesLocation(char *dir);
char *Kinc_Internal_GetFilesLocation();

#ifdef __cplusplus
}
#endif
