#include "pch.h"

#include "FileWriter.h"

#include <Kore/Error.h>
#include <Kore/Log.h>
#include <Kore/System.h>
#include <stdio.h>
#include <string.h>

using namespace Kore;

FileWriter::FileWriter() {}

FileWriter::FileWriter(const char* filepath) {
	if (!open(filepath)) {
		error("Could not open file %s.", filepath);
	}
}

bool FileWriter::open(const char* filepath) {
	return kinc_file_writer_open(&writer, filepath);
}

void FileWriter::close() {
	return kinc_file_writer_close(&writer);
}

FileWriter::~FileWriter() {
	close();
}

void FileWriter::write(void* data, int size) {
	return kinc_file_writer_write(&writer, data, size);
}
