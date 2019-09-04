#include "pch.h"

#include "filereader.h"

#include <kinc/system.h>

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

using namespace Kore;

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

namespace {
	char* fileslocation = nullptr;
}

void kinc_internal_set_files_location(char *dir) {
	fileslocation = dir;
}

char *kinc_internal_get_files_location() {
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
bool kinc_file_reader_open(kinc_file_reader_t *reader, const char *filename, int type) {
	reader->pos = 0;
	reader->file = NULL;
	reader->asset = NULL;
	if (type == KINC_FILE_TYPE_SAVE) {
		char filepath[1001];

		strcpy(filepath, kinc_internal_save_path());
		strcat(filepath, filename);

		reader->file = fopen(filepath, "rb");
		if (reader->file == nullptr) {
			return false;
		}
		fseek(reader->file, 0, SEEK_END);
		reader->size = static_cast<int>(ftell(reader->file));
		fseek(reader->file, 0, SEEK_SET);
		return true;
	}
	else {
		char filepath[1001];
		strcpy(filepath, externalFilesDir);
		strcat(filepath, "/");
		strcat(filepath, filename);

		reader->file = fopen(filepath, "rb");
		if (reader->file != nullptr) {
			fseek(reader->file, 0, SEEK_END);
			reader->size = static_cast<int>(ftell(reader->file));
			fseek(reader->file, 0, SEEK_SET);
			return true;
		}
		else {
			reader->asset = AAssetManager_open(KoreAndroid::getAssetManager(), filename, AASSET_MODE_RANDOM);
			if (reader->asset == nullptr) return false;
			reader->size = AAsset_getLength(reader->asset);
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
	strcpy(filepath, type == KINC_FILE_TYPE_SAVE ? kinc_internal_save_path() : iphonegetresourcepath());
	if (type != KINC_FILE_TYPE_SAVE) {
		strcat(filepath, "/");
		strcat(filepath, KORE_DEBUGDIR);
		strcat(filepath, "/");
	}

	strcat(filepath, filename);
#endif
#ifdef KORE_MACOS
	strcpy(filepath, type == KINC_FILE_TYPE_SAVE ? kinc_internal_save_path() : macgetresourcepath());
	if (type != KINC_FILE_TYPE_SAVE) {
		strcat(filepath, "/");
		strcat(filepath, KORE_DEBUGDIR);
		strcat(filepath, "/");
	}
	strcat(filepath, filename);
#endif
#ifdef KORE_WINDOWS
	if (type == KINC_FILE_TYPE_SAVE) {
		strcpy(filepath, kinc_internal_save_path());
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
	if (type == KINC_FILE_TYPE_SAVE) {
		strcpy(filepath, kinc_internal_save_path());
		strcat(filepath, filename);
	}
	else {
		strcpy(filepath, filename);
	}
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
	if (reader->file != nullptr) {
		return static_cast<int>(fread(data, 1, size, reader->file));
	}
	else {
		int read = AAsset_read(reader->asset, data, size);
		reader->pos += read;
		return read;
	}
#else
	return static_cast<int>(fread(data, 1, size, (FILE*)reader->file));
#endif
}

void kinc_file_reader_seek(kinc_file_reader_t *reader, int pos) {
#ifdef KORE_ANDROID
	if (reader->file != nullptr) {
		fseek(reader->file, pos, SEEK_SET);
	}
	else {
		AAsset_seek(reader->asset, pos, SEEK_SET);
		reader->pos = pos;
	}
#else
	fseek((FILE*)reader->file, pos, SEEK_SET);
#endif
}

void kinc_file_reader_close(kinc_file_reader_t *reader) {
#ifdef KORE_ANDROID
	if (reader->file != nullptr) {
		fclose(reader->file);
		reader->file = nullptr;
	}
	if (reader->asset != nullptr) {
		AAsset_close(reader->asset);
		reader->asset = nullptr;
	}
#else
	if (reader->file == nullptr) return;
	fclose((FILE*)reader->file);
	reader->file = nullptr;
#endif
}

int kinc_file_reader_pos(kinc_file_reader_t *reader) {
#ifdef KORE_ANDROID
	if (reader->file != nullptr)
		return static_cast<int>(ftell(reader->file));
	else
		return reader->pos;
#else
	return static_cast<int>(ftell((FILE*)reader->file));
#endif
}

size_t kinc_file_reader_size(kinc_file_reader_t *reader) {
	return (size_t)reader->size;
}

#endif

float kinc_read_f32le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN // speed optimization
	return *(float *)data;
#else // works on all architectures
	int i = (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	return *(float *)&i;
#endif
}

float kinc_read_f32be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN // speed optimization
	return *(float *)data;
#else // works on all architectures
	int i = (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
	return *(float *)&i;
#endif
}

uint64_t kinc_read_u64le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(u64 *)data;
#else
	return ((u64)data[0] << 0) | ((u64)data[1] << 8) | ((u64)data[2] << 16) | ((u64)data[3] << 24) | ((u64)data[4] << 32) | ((u64)data[5] << 40) |
	       ((u64)data[6] << 48) | ((u64)data[7] << 56);
#endif
}

uint64_t kinc_read_u64be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(u64 *)data;
#else
	return ((u64)data[7] << 0) | ((u64)data[6] << 8) | ((u64)data[5] << 16) | ((u64)data[4] << 24) | ((u64)data[3] << 32) | ((u64)data[2] << 40) |
	       ((u64)data[1] << 48) | ((u64)data[0] << 56);
#endif
}

int64_t kinc_read_s64le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(s64 *)data;
#else
	return ((s64)data[0] << 0) | ((s64)data[1] << 8) | ((s64)data[2] << 16) | ((s64)data[3] << 24) | ((s64)data[4] << 32) | ((s64)data[5] << 40) |
	       ((s64)data[6] << 48) | ((s64)data[7] << 56);
#endif
}

int64_t kinc_read_s64be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(s64 *)data;
#else
	return ((s64)data[7] << 0) | ((s64)data[6] << 8) | ((s64)data[5] << 16) | ((s64)data[4] << 24) | ((s64)data[3] << 32) | ((s64)data[2] << 40) |
	       ((s64)data[1] << 48) | ((s64)data[0] << 56);
#endif
}

uint32_t kinc_read_u32le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(u32 *)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

uint32_t kinc_read_u32be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(u32 *)data;
#else
	return (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
#endif
}

int32_t kinc_read_s32le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(s32 *)data;
#else
	return (data[0] << 0) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
#endif
}

int32_t kinc_read_s32be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(s32 *)data;
#else
	return (data[3] << 0) | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
#endif
}

uint16_t kinc_read_u16le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(u16 *)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

uint16_t kinc_read_u16be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(u16 *)data;
#else
	return (data[1] << 0) | (data[0] << 8);
#endif
}

int16_t kinc_read_s16le(uint8_t *data) {
#ifdef KORE_LITTLE_ENDIAN
	return *(s16 *)data;
#else
	return (data[0] << 0) | (data[1] << 8);
#endif
}

int16_t kinc_read_s16be(uint8_t *data) {
#ifdef KORE_BIG_ENDIAN
	return *(s16 *)data;
#else
	return (data[1] << 0) | (data[0] << 8);
#endif
}

uint8_t kinc_read_u8(uint8_t *data) {
	return *data;
}

int8_t kinc_read_s8(uint8_t *data) {
	return *(int8_t*)data;
}
