#pragma once

#include <kinc/global.h>

#if defined(KORE_SONY) || defined(KORE_SWITCH)
#include <kinc/backend/FileReaderImpl.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*! \file filereader.h
    \brief Provides an API very similar to fread and friends but handles the intricacies of where files are actually hidden on each supported system.
*/

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
typedef struct kinc_file_reader {
	size_t pos;
	size_t size;
	FILE *file;
	struct AAsset *asset;
	int type;
} kinc_file_reader_t;
#else
typedef struct kinc_file_reader {
	void *file;
	size_t size;
	int type;
	int mode;
	bool mounted;
#if defined(KORE_SONY) || defined(KORE_SWITCH)
	kinc_file_reader_impl_t impl;
#endif
} kinc_file_reader_t;
#endif

/// <summary>
/// Opens a file for reading.
/// </summary>
/// <param name="reader">The reader to initialize for reading</param>
/// <param name="filepath">A filepath to identify a file</param>
/// <param name="type">Looks for a regular file (KINC_FILE_TYPE_ASSET) or a save-file (KINC_FILE_TYPE_SAVE)</param>
/// <returns>Whether the file could be opened</returns>
KINC_FUNC bool kinc_file_reader_open(kinc_file_reader_t *reader, const char *filepath, int type);

/// <summary>
/// Closes a file.
/// </summary>
/// <param name="reader">The file to close</param>
KINC_FUNC void kinc_file_reader_close(kinc_file_reader_t *reader);

/// <summary>
/// Reads data from a file starting from the current reading-position and increases the reading-position accordingly.
/// </summary>
/// <param name="reader">The reader to read from</param>
/// <param name="data">A pointer to write the data to</param>
/// <param name="size">The amount of data to read in bytes</param>
/// <returns>The number of bytes that were read - can be less than size if there is not enough data in the file</returns>
KINC_FUNC size_t kinc_file_reader_read(kinc_file_reader_t *reader, void *data, size_t size);

/// <summary>
/// Figures out the size of a file.
/// </summary>
/// <param name="reader">The reader which's file-size to figure out</param>
/// <returns>The size in bytes</returns>
KINC_FUNC size_t kinc_file_reader_size(kinc_file_reader_t *reader);

/// <summary>
/// Figures out the current reading-position in the file.
/// </summary>
/// <param name="reader">The reader which's reading-position to figure out</param>
/// <returns>The current reading-position</returns>
KINC_FUNC size_t kinc_file_reader_pos(kinc_file_reader_t *reader);

/// <summary>
/// Sets the reading-position manually.
/// </summary>
/// <param name="reader">The reader which's reading-position to set</param>
/// <param name="pos">The reading-position to set</param>
KINC_FUNC void kinc_file_reader_seek(kinc_file_reader_t *reader, size_t pos);

/// <summary>
/// Interprets four bytes starting at the provided pointer as a little-endian float.
/// </summary>
KINC_FUNC float kinc_read_f32le(uint8_t *data);

/// <summary>
/// Interprets four bytes starting at the provided pointer as a big-endian float.
/// </summary>
KINC_FUNC float kinc_read_f32be(uint8_t *data);

/// <summary>
/// Interprets eight bytes starting at the provided pointer as a little-endian uint64.
/// </summary>
KINC_FUNC uint64_t kinc_read_u64le(uint8_t *data);

/// <summary>
/// Interprets eight bytes starting at the provided pointer as a big-endian uint64.
/// </summary>
KINC_FUNC uint64_t kinc_read_u64be(uint8_t *data);

/// <summary>
/// Interprets eight bytes starting at the provided pointer as a little-endian int64.
/// </summary>
KINC_FUNC int64_t kinc_read_s64le(uint8_t *data);

/// <summary>
/// Interprets eight bytes starting at the provided pointer as a big-endian int64.
/// </summary>
KINC_FUNC int64_t kinc_read_s64be(uint8_t *data);

/// <summary>
/// Interprets four bytes starting at the provided pointer as a little-endian uint32.
/// </summary>
KINC_FUNC uint32_t kinc_read_u32le(uint8_t *data);

/// <summary>
/// Interprets four bytes starting at the provided pointer as a big-endian uint32.
/// </summary>
KINC_FUNC uint32_t kinc_read_u32be(uint8_t *data);

/// <summary>
/// Interprets four bytes starting at the provided pointer as a little-endian int32.
/// </summary>
KINC_FUNC int32_t kinc_read_s32le(uint8_t *data);

/// <summary>
/// Interprets four bytes starting at the provided pointer as a big-endian int32.
/// </summary>
KINC_FUNC int32_t kinc_read_s32be(uint8_t *data);

/// <summary>
/// Interprets two bytes starting at the provided pointer as a little-endian uint16.
/// </summary>
KINC_FUNC uint16_t kinc_read_u16le(uint8_t *data);

/// <summary>
/// Interprets two bytes starting at the provided pointer as a big-endian uint16.
/// </summary>
KINC_FUNC uint16_t kinc_read_u16be(uint8_t *data);

/// <summary>
/// Interprets two bytes starting at the provided pointer as a little-endian int16.
/// </summary>
KINC_FUNC int16_t kinc_read_s16le(uint8_t *data);

/// <summary>
/// Interprets two bytes starting at the provided pointer as a big-endian int16.
/// </summary>
KINC_FUNC int16_t kinc_read_s16be(uint8_t *data);

/// <summary>
/// Interprets one byte starting at the provided pointer as a uint8.
/// </summary>
KINC_FUNC uint8_t kinc_read_u8(uint8_t *data);

/// <summary>
/// Interprets one byte starting at the provided pointer as an int8.
/// </summary>
KINC_FUNC int8_t kinc_read_s8(uint8_t *data);

void kinc_internal_set_files_location(char *dir);
char *kinc_internal_get_files_location(void);

#ifdef KINC_IMPLEMENTATION_IO
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include "filereader.h"

#undef KINC_IMPLEMENTATION
#include <kinc/system.h>
#define KINC_IMPLEMENTATION

#ifdef KORE_ANDROID
#include <kinc/backend/Android.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef KORE_WINDOWS
#include <malloc.h>
#include <memory.h>
#endif

#ifndef KORE_CONSOLE

#ifdef KORE_IOS
const char *iphonegetresourcepath(void);
#endif

#ifdef KORE_MACOS
const char *macgetresourcepath(void);
#endif

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
#include <kinc/backend/MiniWindows.h>
#endif

#ifdef KORE_TIZEN
#include <FApp.h>
#endif

#ifdef KORE_PI
#define KORE_LINUX
#endif

static char *fileslocation = NULL;
#ifdef KORE_WINDOWS
static wchar_t wfilepath[1001];
#endif

void kinc_internal_set_files_location(char *dir) {
	fileslocation = dir;
}

char *kinc_internal_get_files_location(void) {
	return fileslocation;
}

#ifdef KORE_WINDOWSAPP
void kinc_internal_uwp_installed_location_path(char *path);
#endif

#ifndef KORE_ANDROID
bool kinc_file_reader_open(kinc_file_reader_t *reader, const char *filename, int type) {
	memset(reader, 0, sizeof(kinc_file_reader_t));
	char filepath[1001];
#ifdef KORE_IOS
	strcpy(filepath, type == KINC_FILE_TYPE_SAVE ? kinc_internal_save_path() : iphonegetresourcepath());
	if (type != KINC_FILE_TYPE_SAVE) {
		strcat(filepath, "/");
		strcat(filepath, KORE_DEBUGDIR);
		strcat(filepath, "/");
	}

	strcat(filepath, filename);
#endif
#ifdef KORE_MACOS
	strcpy(filepath, type == KINC_FILE_TYPE_SAVE ? kinc_internal_save_path() : macgetresourcepath());
	if (type != KINC_FILE_TYPE_SAVE) {
		strcat(filepath, "/");
		strcat(filepath, KORE_DEBUGDIR);
		strcat(filepath, "/");
	}
	strcat(filepath, filename);
#endif
#ifdef KORE_WINDOWS
	if (type == KINC_FILE_TYPE_SAVE) {
		strcpy(filepath, kinc_internal_save_path());
		strcat(filepath, filename);
	}
	else {
		strcpy(filepath, filename);
	}
	size_t filepathlength = strlen(filepath);
	for (size_t i = 0; i < filepathlength; ++i)
		if (filepath[i] == '/')
			filepath[i] = '\\';
#endif
#ifdef KORE_WINDOWSAPP
	kinc_internal_uwp_installed_location_path(filepath);
	strcat(filepath, "\\");
	strcat(filepath, filename);
#endif
#ifdef KORE_LINUX
	if (type == KINC_FILE_TYPE_SAVE) {
		strcpy(filepath, kinc_internal_save_path());
		strcat(filepath, filename);
	}
	else {
		strcpy(filepath, filename);
	}
#endif
#ifdef KORE_WASM
	strcpy(filepath, filename);
#endif
#ifdef KORE_EMSCRIPTEN
	strcpy(filepath, KORE_DEBUGDIR);
	strcat(filepath, "/");
	strcat(filepath, filename);
#endif
#ifdef KORE_TIZEN
	for (int i = 0; i < Tizen::App::App::GetInstance()->GetAppDataPath().GetLength(); ++i) {
		wchar_t c;
		Tizen::App::App::GetInstance()->GetAppDataPath().GetCharAt(i, c);
		filepath[i] = (char)c;
	}
	filepath[Tizen::App::App::GetInstance()->GetAppDataPath().GetLength()] = 0;
	strcat(filepath, "/");
	strcat(filepath, filename);
#endif

#ifdef KORE_WINDOWS
	// Drive letter or network
	bool isAbsolute = (filename[1] == ':' && filename[2] == '\\') || (filename[0] == '\\' && filename[1] == '\\');
#else
	bool isAbsolute = filename[0] == '/';
#endif

	if (isAbsolute) {
		strcpy(filepath, filename);
	}
	else if (fileslocation != NULL && type != KINC_FILE_TYPE_SAVE) {
		strcpy(filepath, fileslocation);
		strcat(filepath, "/");
		strcat(filepath, filename);
	}

#ifdef KORE_WINDOWS
	MultiByteToWideChar(CP_UTF8, 0, filepath, -1, wfilepath, 1000);
	reader->file = CreateFileW(wfilepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (reader->file == INVALID_HANDLE_VALUE) {
		return false;
	}
#else
	reader->file = fopen(filepath, "rb");
	if (reader->file == NULL) {
		return false;
	}
#endif

#ifdef KORE_WINDOWS
	// TODO: make this 64-bit compliant
	reader->size = (size_t)GetFileSize(reader->file, NULL);
#else
	fseek((FILE *)reader->file, 0, SEEK_END);
	reader->size = ftell((FILE *)reader->file);
	fseek((FILE *)reader->file, 0, SEEK_SET);
#endif
	return true;
}
#endif

size_t kinc_file_reader_read(kinc_file_reader_t *reader, void *data, size_t size) {
#ifdef KORE_ANDROID
	if (reader->file != NULL) {
		return fread(data, 1, size, reader->file);
	}
	else {
		size_t read = AAsset_read(reader->asset, data, size);
		reader->pos += read;
		return read;
	}
#elif defined(KORE_WINDOWS)
	DWORD readBytes = 0;
	if (ReadFile(reader->file, data, (DWORD)size, &readBytes, NULL)) {
		return (size_t)readBytes;
	}
	else {
		return 0;
	}
#else
	return fread(data, 1, size, (FILE *)reader->file);
#endif
}

void kinc_file_reader_seek(kinc_file_reader_t *reader, size_t pos) {
#ifdef KORE_ANDROID
	if (reader->file != NULL) {
		fseek(reader->file, pos, SEEK_SET);
	}
	else {
		AAsset_seek(reader->asset, pos, SEEK_SET);
		reader->pos = pos;
	}
#elif defined(KORE_WINDOWS)
	// TODO: make this 64-bit compliant
	SetFilePointer(reader->file, (LONG)pos, NULL, FILE_BEGIN);
#else
	fseek((FILE *)reader->file, pos, SEEK_SET);
#endif
}

void kinc_file_reader_close(kinc_file_reader_t *reader) {
#ifdef KORE_ANDROID
	if (reader->file != NULL) {
		fclose(reader->file);
		reader->file = NULL;
	}
	if (reader->asset != NULL) {
		AAsset_close(reader->asset);
		reader->asset = NULL;
	}
#elif defined(KORE_WINDOWS)
	CloseHandle(reader->file);
#else
	if (reader->file == NULL) {
		return;
	}
	fclose((FILE *)reader->file);
	reader->file = NULL;
#endif
}

size_t kinc_file_reader_pos(kinc_file_reader_t *reader) {
#ifdef KORE_ANDROID
	if (reader->file != NULL)
		return ftell(reader->file);
	else
		return reader->pos;
#elif defined(KORE_WINDOWS)
	// TODO: make this 64-bit compliant
	return (size_t)SetFilePointer(reader->file, 0, NULL, FILE_CURRENT);
#else
	return ftell((FILE *)reader->file);
#endif
}

size_t kinc_file_reader_size(kinc_file_reader_t *reader) {
	return reader->size;
}

#endif

float kinc_read_f32le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN // speed optimization
	return *(float *)data;
#else // works on all architectures
	int i = (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	return *(float *)&i;
#endif
}

float kinc_read_f32be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN // speed optimization
	return *(float *)data;
#else // works on all architectures
	int i = (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
	return *(float *)&i;
#endif
}

uint64_t kinc_read_u64le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(uint64_t *)data;
#else
	return ((uint64_t)data[0] << 0) | ((uint64_t)data[1] << 8) | ((uint64_t)data[2] << 16) | ((uint64_t)data[3] << 24) | ((uint64_t)data[4] << 32) |
	       ((uint64_t)data[5] << 40) | ((uint64_t)data[6] << 48) | ((uint64_t)data[7] << 56);
#endif
}

uint64_t kinc_read_u64be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(uint64_t *)data;
#else
	return ((uint64_t)data[7] << 0) | ((uint64_t)data[6] << 8) | ((uint64_t)data[5] << 16) | ((uint64_t)data[4] << 24) | ((uint64_t)data[3] << 32) |
	       ((uint64_t)data[2] << 40) | ((uint64_t)data[1] << 48) | ((uint64_t)data[0] << 56);
#endif
}

int64_t kinc_read_s64le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(int64_t *)data;
#else
	return ((int64_t)data[0] << 0) | ((int64_t)data[1] << 8) | ((int64_t)data[2] << 16) | ((int64_t)data[3] << 24) | ((int64_t)data[4] << 32) |
	       ((int64_t)data[5] << 40) | ((int64_t)data[6] << 48) | ((int64_t)data[7] << 56);
#endif
}

int64_t kinc_read_s64be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(int64_t *)data;
#else
	return ((int64_t)data[7] << 0) | ((int64_t)data[6] << 8) | ((int64_t)data[5] << 16) | ((int64_t)data[4] << 24) | ((int64_t)data[3] << 32) |
	       ((int64_t)data[2] << 40) | ((int64_t)data[1] << 48) | ((int64_t)data[0] << 56);
#endif
}

uint32_t kinc_read_u32le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(uint32_t *)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

uint32_t kinc_read_u32be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(uint32_t *)data;
#else
	return (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
#endif
}

int32_t kinc_read_s32le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(int32_t *)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

int32_t kinc_read_s32be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(int32_t *)data;
#else
	return (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
#endif
}

uint16_t kinc_read_u16le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(uint16_t *)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

uint16_t kinc_read_u16be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(uint16_t *)data;
#else
	return (data[1] << 0) | (data[0] << 8);
#endif
}

int16_t kinc_read_s16le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(int16_t *)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

int16_t kinc_read_s16be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(int16_t *)data;
#else
	return (data[1] << 0) | (data[0] << 8);
#endif
}

uint8_t kinc_read_u8(uint8_t *data) {
	return *data;
}

int8_t kinc_read_s8(uint8_t *data) {
	return *(int8_t *)data;
}

#endif

#ifdef __cplusplus
}
#endif
