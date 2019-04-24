#include "pch.h"

#include "FileReader.h"

#include <Kinc/System.h>

#ifdef KORE_ANDROID
#include <Kore/Android.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef KORE_WINDOWS
#include <malloc.h>
#include <memory.h>
#endif

#ifndef KORE_CONSOLE

#ifdef KORE_IOS
const char* iphonegetresourcepath();
#endif

#ifdef KORE_MACOS
const char* macgetresourcepath();
#endif

#ifdef KORE_ANDROID
#include <android/asset_manager.h>

#include <JNIHelper.h>
#endif

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
#define NOMINMAX
#include <Windows.h>
#endif

#ifdef KORE_TIZEN
#include <FApp.h>
#endif

#ifdef KORE_PI
#define KORE_LINUX
#endif

using namespace Kore;

namespace {
	char* fileslocation = nullptr;
}

void Kinc_Internal_SetFilesLocation(char *dir) {
	fileslocation = dir;
}

char *Kinc_Internal_GetFilesLocation() {
	return fileslocation;
}

#ifdef KORE_ANDROID
namespace {
	char* externalFilesDir;
}

void initAndroidFileReader() {
	std::string dir = ndk_helper::JNIHelper::GetInstance()->GetExternalFilesDir();
	externalFilesDir = new char[dir.size() + 1];
	strcpy(externalFilesDir, dir.c_str());
}
#endif

#ifdef KORE_ANDROID
bool FileReader::open(const char* filename, FileType type) {
	data.pos = 0;
	if (type == Save) {
		char filepath[1001];

		strcpy(filepath, System::savePath());
		strcat(filepath, filename);

		data.file = fopen(filepath, "rb");
		if (data.file == nullptr) {
			return false;
		}
		fseek(data.file, 0, SEEK_END);
		data.size = static_cast<int>(ftell(data.file));
		fseek(data.file, 0, SEEK_SET);
		return true;
	}
	else {
		char filepath[1001];
		strcpy(filepath, externalFilesDir);
		strcat(filepath, "/");
		strcat(filepath, filename);

		data.file = fopen(filepath, "rb");
		if (data.file != nullptr) {
			fseek(data.file, 0, SEEK_END);
			data.size = static_cast<int>(ftell(data.file));
			fseek(data.file, 0, SEEK_SET);
			return true;
		}
		else {
			data.asset = AAssetManager_open(KoreAndroid::getAssetManager(), filename, AASSET_MODE_RANDOM);
			if (data.asset == nullptr) return false;
			data.size = AAsset_getLength(data.asset);
			return true;
		}
	}
}
#endif

#ifndef KORE_ANDROID
bool kinc_file_reader_open(kinc_file_reader_t *reader, const char *filename, int type) {
	memset(reader, 0, sizeof(kinc_file_reader_t));
	char filepath[1001];
#ifdef KORE_IOS
	strcpy(filepath, type == KINC_FILE_TYPE_SAVE ? Kinc_Internal_SavePath() : iphonegetresourcepath());
	if (type != Save) {
		strcat(filepath, "/");
		strcat(filepath, KORE_DEBUGDIR);
		strcat(filepath, "/");
	}

	strcat(filepath, filename);
#endif
#ifdef KORE_MACOS
	strcpy(filepath, type == KINC_FILE_TYPE_SAVE ? Kinc_Internal_SavePath() : macgetresourcepath());
	if (type != KINC_FILE_TYPE_SAVE) {
		strcat(filepath, "/");
		strcat(filepath, KORE_DEBUGDIR);
		strcat(filepath, "/");
	}
	strcat(filepath, filename);
#endif
#ifdef KORE_WINDOWS
	if (type == KINC_FILE_TYPE_SAVE) {
		strcpy(filepath, Kinc_Internal_SavePath());
		strcat(filepath, filename);
	}
	else {
		strcpy(filepath, filename);
	}
	size_t filepathlength = strlen(filepath);
	for (size_t i = 0; i < filepathlength; ++i)
		if (filepath[i] == '/') filepath[i] = '\\';
#endif
#ifdef KORE_WINDOWSAPP
	Platform::String^ locationString = Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
	WideCharToMultiByte(CP_UTF8, 0, locationString->Begin(), -1, filepath, 1000, nullptr, nullptr);
	strcat(filepath, "\\");
	strcat(filepath, filename);
#endif
#ifdef KORE_LINUX
	strcpy(filepath, filename);
#endif
#ifdef KORE_HTML5
	strcpy(filepath, KORE_DEBUGDIR);
	strcat(filepath, "/");
	strcat(filepath, filename);
#endif
#ifdef KORE_TIZEN
	for (int i = 0; i < Tizen::App::App::GetInstance()->GetAppDataPath().GetLength(); ++i) {
		wchar_t c;
		Tizen::App::App::GetInstance()->GetAppDataPath().GetCharAt(i, c);
		filepath[i] = (char)c;
	}
	filepath[Tizen::App::App::GetInstance()->GetAppDataPath().GetLength()] = 0;
	strcat(filepath, "/");
	strcat(filepath, filename);
#endif

#ifdef KORE_WINDOWS
	bool isAbsolute = filename[1] == ':' && filename[2] == '\\';
#else
	bool isAbsolute = filename[0] == '/';
#endif

	if (isAbsolute) {
		strcpy(filepath, filename);
	}
	else if (fileslocation != nullptr && type != KINC_FILE_TYPE_SAVE) {
		strcpy(filepath, fileslocation);
		strcat(filepath, "/");
		strcat(filepath, filename);
	}

	reader->file = fopen(filepath, "rb");
	if (reader->file == nullptr) {
		return false;
	}
	fseek((FILE*)reader->file, 0, SEEK_END);
	reader->size = static_cast<int>(ftell((FILE*)reader->file));
	fseek((FILE*)reader->file, 0, SEEK_SET);
	return true;
}
#endif

int kinc_file_reader_read(kinc_file_reader_t *reader, void *data, size_t size) {
#ifdef KORE_ANDROID
	if (this->data.file != nullptr) {
		return static_cast<int>(fread(data, 1, size, this->data.file));
	}
	else {
		int read = AAsset_read(this->data.asset, data, size);
		this->data.pos += read;
		return read;
	}
#else
	return static_cast<int>(fread(data, 1, size, (FILE*)reader->file));
#endif
}

void kinc_file_reader_seek(kinc_file_reader_t *reader, int pos) {
#ifdef KORE_ANDROID
	if (data.file != nullptr) {
		fseek(data.file, pos, SEEK_SET);
	}
	else {
		AAsset_seek(data.asset, pos, SEEK_SET);
		data.pos = pos;
	}
#else
	fseek((FILE*)reader->file, pos, SEEK_SET);
#endif
}

void kinc_file_reader_close(kinc_file_reader_t *reader) {
#ifdef KORE_ANDROID
	if (data.file != nullptr) {
		fclose(data.file);
		data.file = nullptr;
	}
	if (data.asset != nullptr) {
		AAsset_close(data.asset);
		data.asset = nullptr;
	}
#else
	if (reader->file == nullptr) return;
	fclose((FILE*)reader->file);
	reader->file = nullptr;
#endif
}

int kinc_file_reader_pos(kinc_file_reader_t *reader) {
#ifdef KORE_ANDROID
	if (data.file != nullptr)
		return static_cast<int>(ftell(data.file));
	else
		return data.pos;
#else
	return static_cast<int>(ftell((FILE*)reader->file));
#endif
}

size_t kinc_file_reader_size(kinc_file_reader_t *reader) {
	return (size_t)reader->size;
}

#endif

float Kinc_ReadF32LE(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN // speed optimization
	return *(float *)data;
#else // works on all architectures
	int i = (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	return *(float *)&i;
#endif
}

float Kinc_ReadF32BE(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN // speed optimization
	return *(float *)data;
#else // works on all architectures
	int i = (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
	return *(float *)&i;
#endif
}

uint64_t Kinc_ReadU64LE(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(u64 *)data;
#else
	return ((u64)data[0] << 0) | ((u64)data[1] << 8) | ((u64)data[2] << 16) | ((u64)data[3] << 24) | ((u64)data[4] << 32) | ((u64)data[5] << 40) |
	       ((u64)data[6] << 48) | ((u64)data[7] << 56);
#endif
}

uint64_t Kinc_ReadU64BE(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(u64 *)data;
#else
	return ((u64)data[7] << 0) | ((u64)data[6] << 8) | ((u64)data[5] << 16) | ((u64)data[4] << 24) | ((u64)data[3] << 32) | ((u64)data[2] << 40) |
	       ((u64)data[1] << 48) | ((u64)data[0] << 56);
#endif
}

int64_t Kinc_ReadS64LE(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(s64 *)data;
#else
	return ((s64)data[0] << 0) | ((s64)data[1] << 8) | ((s64)data[2] << 16) | ((s64)data[3] << 24) | ((s64)data[4] << 32) | ((s64)data[5] << 40) |
	       ((s64)data[6] << 48) | ((s64)data[7] << 56);
#endif
}

int64_t Kinc_ReadS64BE(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(s64 *)data;
#else
	return ((s64)data[7] << 0) | ((s64)data[6] << 8) | ((s64)data[5] << 16) | ((s64)data[4] << 24) | ((s64)data[3] << 32) | ((s64)data[2] << 40) |
	       ((s64)data[1] << 48) | ((s64)data[0] << 56);
#endif
}

uint32_t Kinc_ReadU32LE(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(u32 *)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

uint32_t Kinc_ReadU32BE(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(u32 *)data;
#else
	return (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
#endif
}

int32_t Kinc_ReadS32LE(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(s32 *)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

int32_t Kinc_ReadS32BE(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(s32 *)data;
#else
	return (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
#endif
}

uint16_t Kinc_ReadU16LE(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(u16 *)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

uint16_t Kinc_ReadU16BE(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(u16 *)data;
#else
	return (data[1] << 0) | (data[0] << 8);
#endif
}

int16_t Kinc_ReadS16LE(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(s16 *)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

int16_t Kinc_ReadS16BE(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(s16 *)data;
#else
	return (data[1] << 0) | (data[0] << 8);
#endif
}
