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
KINC_FUNC int kinc_file_reader_read(kinc_file_reader_t *reader, void *data, size_t size);

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
KINC_FUNC int kinc_file_reader_pos(kinc_file_reader_t *reader);

/// <summary>
/// Sets the reading-position manually.
/// </summary>
/// <param name="reader">The reader which's reading-position to set</param>
/// <param name="pos">The reading-position to set</param>
KINC_FUNC void kinc_file_reader_seek(kinc_file_reader_t *reader, int pos);

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

#ifdef __cplusplus
}
#endif
