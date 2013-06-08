#include "pch.h"
#include "FileReader.h"
#include <Kore/Error.h>
#include <Kore/Math/Core.h>
#include "miniz.h"
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#ifdef SYS_WINDOWS
#include <malloc.h>
#include <memory.h>
#endif

#ifndef KORE_DEBUGDIR
#define KORE_DEBUGDIR "Deployment"
#endif

#ifdef SYS_IOS
const char* iphonegetresourcepath();
#endif

#ifdef SYS_OSX
const char* macgetresourcepath();
#endif

#ifdef SYS_ANDROID
extern mz_zip_archive* getApk();
#endif

#ifdef SYS_WINDOWS
#define NOMINMAX
#include <Windows.h>
#endif

using namespace Kore;

FileReader::FileReader() {
	data.file = nullptr;
	data.size = 0;
}

FileReader::FileReader(const char* filename) {
	data.file = nullptr;
	data.size = 0;
	if (!open(filename)) {
		char message[101];
		sprintf(message, "Could not open file %s.", filename);
		error(message);
	}
}

#ifdef SYS_ANDROID
bool FileReader::open(const char* filename) {
	pos = 0;
	char file[1001];
	strcpy(file, "assets/");
	strcat(file, filename);
	obj = mz_zip_reader_extract_file_to_heap(getApk(), file, &size, 0);
	if (obj == nullptr) {
		mz_zip_reader_end(getApk());
		return false;
	}
	return true;
}
#endif

#ifndef SYS_ANDROID
bool FileReader::open(const char* filename) {
	char filepath[1001];
#ifdef SYS_IOS
	strcpy(filepath, iphonegetresourcepath());
	strcat(filepath, "/");
	strcat(filepath, KORE_DEBUGDIR);
	strcat(filepath, "/");
	strcat(filepath, filename);
#endif
#ifdef SYS_OSX
	strcpy(filepath, macgetresourcepath());
	strcat(filepath, "/");
	strcat(filepath, KORE_DEBUGDIR);
	strcat(filepath, "/");
	strcat(filepath, filename);
#endif
#ifdef SYS_XBOX360
	filepath = Kt::Text(L"game:\\media\\") + filepath;
	filepath.replace(Kt::Char('/'), Kt::Char('\\'));
#endif
#ifdef SYS_PS3
	filepath = Kt::Text(SYS_APP_HOME) + "/" + filepath;
#endif
#ifdef SYS_WINDOWS
	strcpy(filepath, filename);
	size_t filepathlength = strlen(filepath);
	for (size_t i = 0; i < filepathlength; ++i)
		if (filepath[i] == '/') filepath[i] = '\\';
#endif
#ifdef SYS_WINDOWSRT
	const wchar_t* location = Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
	int i;
	for (i = 0; location[i] != 0; ++i) {
		filepath[i] = (char)location[i];
	}
	int len = (int)strlen(filename);
	int index;
	for (index = len; index > 0; --index) {
		if (filename[index] == '/' || filename[index] == '\\') {
			++index;
			break;
		}
	}
	filepath[i++] = '\\';
	while (index < len) {
		filepath[i++] = filename[index++];
	}
	filepath[i] = 0;
#endif
#ifdef SYS_LINUX
	strcpy(filepath, filename);
#endif
#ifdef SYS_HTML5
	strcpy(filepath, filename);
#endif
	data.file = fopen(filepath, "rb");
	if (data.file == nullptr) {
		printf("Could not open %s\n", filepath);
		return false;
	}
	fseek(data.file, 0, SEEK_END);
	data.size = static_cast<int>(ftell(data.file));
	fseek(data.file, 0, SEEK_SET);
	return true;
}
#endif

int FileReader::read(void* data, int size) {
#ifdef SYS_ANDROID
	Kore::uint memsize = Kore::min(size, data.size - pos);
	memcpy(data, (u8*)obj + pos, memsize);
	pos += psize_;
	return psize_;
#else
	return static_cast<int>(fread(data, 1, size, this->data.file));
#endif
}

void* FileReader::readAll() {
#ifdef SYS_ANDROID
	return obj;
#else
	seek(0);
	void* data = new Kore::u8[this->data.size];
	read(data, this->data.size);
	return data;
#endif
}

void FileReader::seek(int pos) {
#ifdef SYS_ANDROID
	data.pos = pos;
#else
	fseek(data.file, pos, SEEK_SET);
#endif
}

void FileReader::close() {
#ifdef SYS_ANDROID
	free(obj);
	obj = nullptr;
#else
	if (data.file == nullptr) return;
	fclose(data.file);
	data.file = nullptr;
#endif
}

FileReader::~FileReader() {
	close();
}

int FileReader::pos() const {
#ifdef SYS_ANDROID
	return pos;
#else
	return static_cast<int>(ftell(data.file));
#endif
}

int FileReader::size() const {
	#ifdef SYS_ANDROID
	return pos;
#else
	return data.size;
#endif
}
