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
	return kinc_file_reader_open(&reader, filename, (int)type);
}

int FileReader::read(void* data, int size) {
	return kinc_file_reader_read(&reader, data, size);
}

void* FileReader::readAll() {
	kinc_file_reader_seek(&reader, 0);
	free(readdata);
	int size = (int)kinc_file_reader_size(&reader);
	readdata = malloc(size);
	read(readdata, size);
	return readdata;
}

void FileReader::seek(int pos) {
	kinc_file_reader_seek(&reader, pos);
}

void FileReader::close() {
	kinc_file_reader_close(&reader);
	free(readdata);
	readdata = nullptr;
}

FileReader::~FileReader() {
	close();
}

int FileReader::pos() {
	return kinc_file_reader_pos(&reader);
}

int FileReader::size() {
	return (int)kinc_file_reader_size(&reader);
}
