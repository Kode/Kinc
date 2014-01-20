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

#ifdef SYS_TIZEN
#include <FApp.h>
#endif

using namespace Kore;

FileReader::FileReader() {
#ifdef SYS_ANDROID
	data.all = nullptr;
	data.size = 0;
	data.pos = 0;
#else
	data.file = nullptr;
	data.size = 0;
#endif
}

FileReader::FileReader(const char* filename) {
#ifdef SYS_ANDROID
	data.all = nullptr;
	data.size = 0;
	data.pos = 0;
#else
	data.file = nullptr;
	data.size = 0;
#endif
	if (!open(filename)) {
		error("Could not open file %s.", filename);
	}
}

#ifdef SYS_ANDROID
bool FileReader::open(const char* filename) {
	data.pos = 0;
	char file[1001];
	strcpy(file, "assets/");
	strcat(file, filename);
	size_t size;
	data.all = mz_zip_reader_extract_file_to_heap(getApk(), file, &size, 0);
	data.size = static_cast<int>(size);
	if (data.all == nullptr) {
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
#ifdef SYS_TIZEN
	for (int i = 0; i < Tizen::App::App::GetInstance()->GetAppDataPath().GetLength(); ++i) {
		wchar_t c;
		Tizen::App::App::GetInstance()->GetAppDataPath().GetCharAt(i, c);
		filepath[i] = (char)c;
	}
	filepath[Tizen::App::App::GetInstance()->GetAppDataPath().GetLength()] = 0;
	strcat(filepath, "/");
	strcat(filepath, filename);
#endif
	data.file = fopen(filepath, "rb");
	if (data.file == nullptr) {
		printf("Could not open %s\n", filepath);
		return false;
	}
	fseek((FILE*)data.file, 0, SEEK_END);
	data.size = static_cast<int>(ftell((FILE*)data.file));
	fseek((FILE*)data.file, 0, SEEK_SET);
	return true;
}
#endif

int FileReader::read(void* data, int size) {
#ifdef SYS_ANDROID
	int memsize = Kore::min(size, this->data.size - this->data.pos);
	memcpy(data, (u8*)this->data.all + this->data.pos, memsize);
	this->data.pos += memsize;
	return memsize;
#else
	return static_cast<int>(fread(data, 1, size, (FILE*)this->data.file));
#endif
}

void* FileReader::readAll() {
#ifdef SYS_ANDROID
	return data.all;
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
	fseek((FILE*)data.file, pos, SEEK_SET);
#endif
}

void FileReader::close() {
#ifdef SYS_ANDROID
	free(data.all);
	data.all = nullptr;
#else
	if (data.file == nullptr) return;
	fclose((FILE*)data.file);
	data.file = nullptr;
#endif
}

FileReader::~FileReader() {
	close();
}

int FileReader::pos() const {
#ifdef SYS_ANDROID
	return data.pos;
#else
	return static_cast<int>(ftell((FILE*)data.file));
#endif
}

int FileReader::size() const {
#ifdef SYS_ANDROID
	return data.size;
#else
	return data.size;
#endif
}
