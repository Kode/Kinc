#pragma once

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
int Kinc_FileReader_Read(Kinc_FileReader *reader, void *data, int size);
int Kinc_FileReader_Size(Kinc_FileReader *reader);
int Kinc_FileReader_Pos(Kinc_FileReader *reader);
void Kinc_FileReader_Seek(Kinc_FileReader *reader, int pos);

void Kinc_Internal_SetFilesLocation(char *dir);
char *Kinc_Internal_GetFilesLocation();

#ifdef __cplusplus
}
#endif
