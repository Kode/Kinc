#include "pch.h"

#include "FileReader.h"

#include <Kore/Error.h>
#include <Kore/Log.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>

#include <stdlib.h>

using namespace Kore;

FileReader::FileReader() : readdata(nullptr) {

}

FileReader::FileReader(const char* filename, FileType type) : readdata(nullptr) {
	if (!open(filename, type)) {
		error("Could not open file %s.", filename);
	}
}

bool FileReader::open(const char* filename, FileType type) {
	return Kinc_FileReader_Open(&reader, filename, (int)type);
}

int FileReader::read(void* data, int size) {
	return Kinc_FileReader_Read(&reader, data, size);
}

void* FileReader::readAll() {
	Kinc_FileReader_Seek(&reader, 0);
	free(readdata);
	int size = Kinc_FileReader_Size(&reader);
	readdata = malloc(size);
	read(readdata, size);
	return readdata;
}

void FileReader::seek(int pos) {
	Kinc_FileReader_Seek(&reader, pos);
}

void FileReader::close() {
	Kinc_FileReader_Close(&reader);
	free(readdata);
	readdata = nullptr;
}

FileReader::~FileReader() {
	close();
}

int FileReader::pos() {
	return Kinc_FileReader_Pos(&reader);
}

int FileReader::size() {
	return Kinc_FileReader_Size(&reader);
}
