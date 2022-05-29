#pragma once

#include <kinc/global.h>

#include <stdbool.h>

/*! \file filewriter.h
    \brief Provides an API very similar to fwrite and friends but uses a directory that can actually used for persistant file storage. This can later be read
   using the kinc_file_reader-functions and KINC_FILE_TYPE_SAVE.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_file_writer {
	void *file;
	const char *filename;
	bool mounted;
} kinc_file_writer_t;

/// <summary>
/// Opens a file for writing.
/// </summary>
/// <param name="reader">The writer to initialize for writing</param>
/// <param name="filepath">A filepath to identify a file</param>
/// <returns>Whether the file could be opened</returns>
KINC_FUNC bool kinc_file_writer_open(kinc_file_writer_t *writer, const char *filepath);

/// <summary>
/// Writes data to a file starting from the current writing-position and increases the writing-position accordingly.
/// </summary>
/// <param name="reader">The writer to write to</param>
/// <param name="data">A pointer to read the data from</param>
/// <param name="size">The amount of data to write in bytes</param>
KINC_FUNC void kinc_file_writer_write(kinc_file_writer_t *writer, void *data, int size);

/// <summary>
/// Closes a file.
/// </summary>
/// <param name="reader">The file to close</param>
KINC_FUNC void kinc_file_writer_close(kinc_file_writer_t *writer);

#ifdef __cplusplus
}
#endif
