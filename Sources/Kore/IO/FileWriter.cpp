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

#if defined(KORE_PS4) || defined(KORE_SWITCH)
#define MOUNT_SAVES
bool mountSaveData(bool);
void unmountSaveData();
#endif

bool FileWriter::open(const char* filepath) {
	return Kinc_FileWriter_Open(&writer, filepath);
}

void FileWriter::close() {
	return Kinc_FileWriter_Close(&writer);
}

FileWriter::~FileWriter() {
	close();
}

void FileWriter::write(void* data, int size) {
	return Kinc_FileWriter_Write(&writer, data, size);
}
