#include "pch.h"
#include "File.h"
#include <Kore/Math/Core.h>
#include "miniz.h"
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#ifdef SYS_WINDOWS
#include <malloc.h>
#include <memory.h>
#endif

using namespace Kore;

#ifdef SYS_IOS
const char* iphonegetresourcepath();
#endif

#ifdef SYS_OSX
const char* macgetresourcepath();
#endif

#ifdef SYS_ANDROID
//#include <android/asset_manager.h>
//AAssetManager* getAssetManager();
extern mz_zip_archive* getApk();
#endif

float GenFile::readFloatFromLittleEndian(u8* data) {
#ifdef SYS_LITTLE_ENDIAN //speed optimization
	return *(float*)data;
#else //works on all architectures
	int i = (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	return *(float*)&i;
#endif
}

u32 GenFile::readU32FromLittleEndian(u8* data) {
#ifdef SYS_LITTLE_ENDIAN
	return *(u32*)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

s32 GenFile::readS32FromLittleEndian(u8* data) {
#ifdef SYS_LITTLE_ENDIAN
	return *(s32*)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

void GenFile::writeString8(const char *c) {
	uint len = strlen(c);
	write(c, len);
}

float GenFile::readFloat() {
	u8 data[4];
	read(data, 4);
#ifdef SYS_LITTLE_ENDIAN
	return *(float*)data;
#else
	int i = (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	return *(float*)&i;
#endif
}

u32 GenFile::readU32() {
	u8 data[4];
	read(data, 4);
#ifdef SYS_LITTLE_ENDIAN
	return *(u32*)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

s32 GenFile::readS32() {
	u8 data[4];
	read(data, 4);
#ifdef SYS_LITTLE_ENDIAN
	return *(s32*)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

s32 GenFile::readS32BE() {
	u8 data[4];
	read(data, 4);
#ifdef SYS_BIG_ENDIAN
	return *(s32*)data;
#else
	return (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
#endif
}

u16 GenFile::readU16() {
	u8 data[2];
	read(data, 2);
#ifdef SYS_LITTLE_ENDIAN
	return *(u16*)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

s16 GenFile::readS16() {
	u8 data[2];
	read(data, 2);
#ifdef SYS_LITTLE_ENDIAN
	return *(s16*)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

u8 GenFile::readU8() {
	u8 data;
	read(&data, 1);
	return data;
}

s8 GenFile::readS8() {
	s8 data;
	read(&data, 1);
	return data;
}

namespace {
	const char* fileModeToFopenString(DiskFile::FileMode mode) {
		switch (mode) {
		case DiskFile::ReadMode:
			return "rb";
		case DiskFile::WriteMode:
			return "wb";
		case DiskFile::AppendMode:
			return "ab";
		default:
			return "";
		}
	}
}

#ifdef SYS_WINDOWS
#define NOMINMAX
#include <Windows.h>
#endif

#ifdef SYS_PS3
#include <sys/paths.h>
#endif

DiskFile::DiskFile() {
	obj = nullptr;
}

#ifdef SYS_ANDROID
bool DiskFile::open(const char* filename, FileMode mode) {
	this->mode  = mode;
	pos = 0;
	char file[1001];
	strcpy(file, "assets/");
	strcat(file, filename);
	obj = mz_zip_reader_extract_file_to_heap(getApk(), file, &size, 0);
	if (obj == nullptr) {
		mz_zip_reader_end(getApk());
		return false;
	}
	/*this->mode  = mode;
	this->flags = flags;
	Kt::Text filepath(filename);
	obj = AAssetManager_open(getAssetManager(), filepath.c_str(), AASSET_MODE_UNKNOWN);
	if (!obj) {
		if (!(flags & FL_DO_NOT_EXCEPT)) {
			throw Exception(Text("Could not open ") + filename);
		}
		else {
			return;
		}
	}
	size = AAsset_getLength((AAsset*)obj);*/
	return true;
}
#endif

#ifndef SYS_ANDROID
bool DiskFile::open(const char* filename, FileMode mode) {
	this->mode  = mode;
	char filepath[1001];
#ifdef SYS_IOS
	strcpy(filepath, iphonegetresourcepath());
	strcat(filepath, "/Deployment/");
	strcat(filepath, filename);
#endif
#ifdef SYS_OSX
	strcpy(filepath, macgetresourcepath());
	strcat(filepath, "/Deployment/");
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
#ifdef SYS_LINUX
	strcpy(filepath, filename);
#endif

#if defined SYS_WINDOWS && !defined SYS_WINDOWS8
	if (mode == DiskFile::ReadMode) {
		//printf("%s\n", filepath.c_str());
		file = CreateFileA(filepath, mode == DiskFile::ReadMode ? GENERIC_READ : GENERIC_WRITE, 0, nullptr, mode == DiskFile::ReadMode ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE) {
			return false;
		}
		size       = GetFileSize(file, nullptr);
		mappedFile = CreateFileMapping(file, nullptr, mode == DiskFile::ReadMode ? PAGE_READONLY : PAGE_READWRITE, 0, 0, nullptr);
		obj        = MapViewOfFile(mappedFile, mode == DiskFile::ReadMode ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);
		pos        = 0;
	}
	else { // Use fopen() for write-mode
#endif
	obj = fopen(filepath, fileModeToFopenString(mode));
	if (obj == nullptr) {
		printf("Could not open %s\n", filepath);
		char errormsg[100];
		strcpy(errormsg, "Could not open ");
		strcat(errormsg, filename);
		return false;
	}
	fseek((FILE*)obj, 0, SEEK_END);
	size = static_cast<uint>(ftell((FILE*)obj));
	fseek((FILE*)obj, 0, SEEK_SET);
#if defined SYS_WINDOWS && !defined SYS_WINDOWS8
	}
#endif
	return true;
}
#endif

uint DiskFile::write(const void* data, uint psize) {
	uint size_written = static_cast<uint>(fwrite(data, 1, psize, (FILE*)obj));
	return size_written;
}

uint DiskFile::read(void* data, uint psize) {
#if defined SYS_WINDOWS && !defined SYS_WINDOWS8
	uint psize_ = Kore::min(psize, size - pos);
	memcpy(data, (u8*)obj + pos, psize_);
	pos += psize_;
	return psize_;
#elif defined SYS_ANDROID
	//return AAsset_read((AAsset*)obj, data, psize);
	Kore::uint psize_ = Kore::min(psize, size - pos);
	memcpy(data, (u8*)obj + pos, psize_);
	pos += psize_;
	return psize_;
#else
	return static_cast<uint>(fread(data, 1, psize, (FILE*)obj));
#endif
}

void* DiskFile::readAll() {
#if defined SYS_WINDOWS && !defined SYS_WINDOWS8
	return obj;
#elif defined SYS_ANDROID
	//return (void*)AAsset_getBuffer((AAsset*)obj);
	return obj;
#else
	seek(0);
	void* data = new Kore::u8[size];
	read(data, size);
	return data;
#endif
}

void DiskFile::flush() {
#if defined SYS_WINDOWS && !defined SYS_WINDOWS8
	if (mode == DiskFile::WriteMode) {
		fflush((FILE*)obj);
	}
	if (mode == DiskFile::ReadMode) {
		FlushViewOfFile(obj, 0);
	}
	else
#endif
	{
#ifndef SYS_ANDROID
		fflush((FILE*)obj);
#else

#endif
	}
}

void DiskFile::seek(uint ppos) {
#if defined SYS_WINDOWS && !defined SYS_WINDOWS8
	if (mode == DiskFile::ReadMode) pos = ppos;
	else
#endif
#ifdef SYS_ANDROID
	//AAsset_seek((AAsset*)obj, ppos, SEEK_SET);
	pos = ppos;
#else
	fseek((FILE*)obj, ppos, SEEK_SET);
#endif
}

void DiskFile::close() {
	if (obj == nullptr) return;
#if defined SYS_WINDOWS && !defined SYS_WINDOWS8
	if (mode == DiskFile::ReadMode) {
		UnmapViewOfFile(obj);
		CloseHandle(mappedFile);
		CloseHandle(file);
	}
	else
#endif
	{
#ifdef SYS_ANDROID
		//AAsset_close((AAsset*)obj);
		free(obj);
		obj = nullptr;
#else
		fclose((FILE*)obj);
#endif
	}
	obj = nullptr;
}

DiskFile::~DiskFile() {
	close();
}

uint DiskFile::getPos() {
#if defined SYS_WINDOWS && !defined SYS_WINDOWS8
	if (mode == DiskFile::ReadMode) {
		return pos;
	}
	else
#endif
#ifdef SYS_ANDROID
	//return 0; //TODO
	return pos;
#else
	return static_cast<uint>(ftell((FILE*)obj));
#endif
}

u64 DiskFile::getLastWriteTimeStamp64() const {
#if defined SYS_WINDOWS && !defined SYS_WINDOWS8
	BY_HANDLE_FILE_INFORMATION info;
	GetFileInformationByHandle(file, &info);
	return (u64)info.ftLastWriteTime.dwLowDateTime + ((u64)info.ftLastWriteTime.dwHighDateTime << 32);
#else
	//affirm(0);
	return 0;
#endif
}

MemReadFile::MemReadFile(void* mem, uint mem_size) {
	obj     = mem;
	size    = mem_size;
	pos     = 0;
	own_mem = 0;
}

MemReadFile::MemReadFile(uint mem_size) {
	obj     = std::malloc(mem_size);
	size    = mem_size;
	pos     = 0;
	own_mem = obj;
	//if (obj == nullptr) throw Exception("Could not allocate memory for MemReadFile.");
}

uint MemReadFile::write(const void* data, uint psize) {
	return 0;
}

uint MemReadFile::read(void* data, uint psize) {
	uint psize_ = Kore::min(psize, size - pos);
	std::memcpy(data, (u8*)obj + pos, psize_);
	pos += psize_;
	return psize_;
}

void* MemReadFile::readAll() {
	return obj;
}

void MemReadFile::flush() {

}

void MemReadFile::seek(uint ppos) {
	pos = ppos;
}

MemReadFile::~MemReadFile() {
	if (own_mem) {
		std::free(own_mem);
		own_mem = nullptr;
	}
}

uint MemReadFile::getPos() {
	return pos;
}

///////////////////////////////////////////////////////////////////////////////

MemWriteFile::MemWriteFile(uint min_size, uint pblock_size) {
	alloc_size = min_size;
	block_size_ = pblock_size - 1;
	obj  = std::malloc(alloc_size);
	size = 0;
	pos  = 0;
	//if (obj == nullptr) throw Exception("Could not allocate memory for MemWriteFile.");
}

uint MemWriteFile::write(const void* data, uint psize) {
	if (pos + psize > alloc_size) {
		uint new_alloc_size = pos + psize + block_size_;
		new_alloc_size -= (new_alloc_size % block_size_);
		void *new_obj = new u8[new_alloc_size];
		std::memcpy(new_obj, obj, size);
		delete[] (u8*)obj;
		obj = new_obj;
		alloc_size = new_alloc_size;
	}
	std::memcpy((u8*)obj + pos, data, psize);
	size = Kore::max(size, pos + psize);
	pos += psize;
	return psize;
}

uint MemWriteFile::read(void* data, uint psize) {
	return 0;
}

void* MemWriteFile::readAll() {
	return nullptr;
}

void MemWriteFile::flush() {

}

void MemWriteFile::seek(uint ppos) {
	pos = ppos;
}

MemWriteFile::~MemWriteFile() {
	std::free(obj);
	obj = nullptr;
}

uint MemWriteFile::getPos() {
	return pos;
}

#if defined SYS_WINDOWS

#include <Windows.h>

WindowsResourceFile::WindowsResourceFile(unsigned int resource) : MemReadFile(nullptr, 0) {
	HRSRC hrsrc = nullptr; //**FindResource(GetModuleHandle(nullptr), MAKEINTRESOURCE(resource), RT_RCDATA);
	//if (hrsrc == nullptr) throw Exception("Resource not found");
	HGLOBAL hglobal = LoadResource(GetModuleHandle(nullptr), hrsrc);
	obj = LockResource(hglobal);
	size = SizeofResource(GetModuleHandle(nullptr), hrsrc);
}

#endif
