#pragma once

#include <stdbool.h>
#include <stdint.h>

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
typedef struct {
	void *file;
	int size;
	int type;
} Kinc_FileReader;
#endif

bool Kinc_FileReader_Open(Kinc_FileReader *reader, const char *filename, int type);
void Kinc_FileReader_Close(Kinc_FileReader *reader);
int Kinc_FileReader_Read(Kinc_FileReader *reader, void *data, size_t size);
size_t Kinc_FileReader_Size(Kinc_FileReader *reader);
int Kinc_FileReader_Pos(Kinc_FileReader *reader);
void Kinc_FileReader_Seek(Kinc_FileReader *reader, int pos);

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
