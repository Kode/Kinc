#include "pch.h"
#include "FileReader.h"
#include <Kore/Error.h>
#include <Kore/Log.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>
#ifdef SYS_ANDROID
#include <Kore/Android.h>
#endif
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#ifdef SYS_WINDOWS
#include <malloc.h>
#include <memory.h>
#endif

#ifndef SYS_CONSOLE

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
#include <android/asset_manager.h>
#include <JNIHelper.h>
#endif

#ifdef SYS_WINDOWS
#define NOMINMAX
#include <Windows.h>
#endif

#ifdef SYS_TIZEN
#include <FApp.h>
#endif

using namespace Kore;

#ifdef SYS_ANDROID
namespace {
	char* externalFilesDir;
}

void initAndroidFileReader() {
	std::string dir = ndk_helper::JNIHelper::GetInstance()->GetExternalFilesDir();
	externalFilesDir = new char[dir.size() + 1];
	strcpy(externalFilesDir, dir.c_str());
}
#endif

FileReader::FileReader() : readdata(nullptr) {
#ifdef SYS_ANDROID
	data.size = 0;
	data.pos = 0;
	data.file = nullptr;
	data.asset = nullptr;
#else
	data.file = nullptr;
	data.size = 0;
#endif
}

FileReader::FileReader(const char* filename, FileType type) : readdata(nullptr) {
#ifdef SYS_ANDROID
	data.size = 0;
	data.pos = 0;
	data.file = nullptr;
	data.asset = nullptr;
#else
	data.file = nullptr;
	data.size = 0;
#endif
	if (!open(filename, type)) {
		error("Could not open file %s.", filename);
	}
}

#ifdef SYS_ANDROID
bool FileReader::open(const char* filename, FileType type) {
	data.pos = 0;
	if (type == Save) {
		char filepath[1001];

		strcpy(filepath, System::savePath());
		strcat(filepath, filename);

		data.file = fopen(filepath, "rb");
		if (data.file == nullptr) {
			log(Warning, "Could not open file %s.", filepath);
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

#ifndef SYS_ANDROID
bool FileReader::open(const char* filename, FileType type) {
	char filepath[1001];
#ifdef SYS_IOS
	strcpy(filepath, type == Save ? System::savePath() : iphonegetresourcepath());
	if (type != Save) {
		strcat(filepath, "/");
		strcat(filepath, KORE_DEBUGDIR);
		strcat(filepath, "/");
	}

	strcat(filepath, filename);
#endif
#ifdef SYS_OSX
	strcpy(filepath, type == Save ? System::savePath() : macgetresourcepath());
	if (type != Save) {
		strcat(filepath, "/");
		strcat(filepath, KORE_DEBUGDIR);
		strcat(filepath, "/");
	}
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
	if (type == Save) {
		strcpy(filepath, System::savePath());
		strcat(filepath, filename);
	}
	else {
		strcpy(filepath, filename);
	}
	size_t filepathlength = strlen(filepath);
	for (size_t i = 0; i < filepathlength; ++i)
		if (filepath[i] == '/') filepath[i] = '\\';
#endif
#ifdef SYS_WINDOWSAPP
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
	strcpy(filepath, KORE_DEBUGDIR);
	strcat(filepath, "/");
	strcat(filepath, filename);
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
		log(Warning, "Could not open file %s.", filepath);
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
	if (this->data.file != nullptr) {
		return static_cast<int>(fread(data, 1, size, this->data.file));
	}
	else {
		int read = AAsset_read(this->data.asset, data, size);
		this->data.pos += read;
		return read;
	}
#else
	return static_cast<int>(fread(data, 1, size, (FILE*)this->data.file));
#endif
}

void* FileReader::readAll() {
	seek(0);
	delete[] readdata;
	readdata = new Kore::u8[this->data.size];
	read(readdata, this->data.size);
	return readdata;
}

void FileReader::seek(int pos) {
#ifdef SYS_ANDROID
	if (data.file != nullptr) {
		fseek(data.file, pos, SEEK_SET);
	}
	else {
		AAsset_seek(data.asset, pos, SEEK_SET);
		data.pos = pos;
	}
#else
	fseek((FILE*)data.file, pos, SEEK_SET);
#endif
}

void FileReader::close() {
#ifdef SYS_ANDROID
	if (data.file != nullptr) {
		fclose(data.file);
		data.file = nullptr;
	}
	if (data.asset != nullptr) {
		AAsset_close(data.asset);
		data.asset = nullptr;
	}
#else
	if (data.file == nullptr) return;
	fclose((FILE*)data.file);
	data.file = nullptr;
#endif
	delete[] readdata;
	readdata = nullptr;
}

FileReader::~FileReader() {
	close();
}

int FileReader::pos() const {
#ifdef SYS_ANDROID
	if (data.file != nullptr) return static_cast<int>(ftell(data.file));
	else return data.pos;
#else
	return static_cast<int>(ftell((FILE*)data.file));
#endif
}

int FileReader::size() const {
	return data.size;
}

#endif
