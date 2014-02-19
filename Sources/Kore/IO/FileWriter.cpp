#include "pch.h"
#include "FileWriter.h"
#include <Kore/Error.h>
#include <Kore/Log.h>
#include <Kore/System.h>
#include <stdio.h>
#include <string.h>

using namespace Kore;

FileWriter::FileWriter() : file(nullptr) {
	
}

FileWriter::FileWriter(const char* filepath) : file(nullptr) {
	if (!open(filepath)) {
		error("Could not open file %s.", filepath);
	}
}

bool FileWriter::open(const char* filepath) {
	char path[1001];
	strcpy(path, System::savePath());
	strcat(path, filepath);
	file = fopen(path, "wb");
	if (file == nullptr) {
		log(Warning, "Could not open file %s.", filepath);
		return false;
	}
	return true;
}

void FileWriter::close() {
	if (file == nullptr) return;
	fclose((FILE*)file);
	file = nullptr;
}

FileWriter::~FileWriter() {
	close();
}

void FileWriter::write(void* data, int size) {
	fwrite(data, 1, size, (FILE*)file);
}
