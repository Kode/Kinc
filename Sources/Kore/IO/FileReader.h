#pragma once

#include "Reader.h"

#ifndef KORE_DEBUGDIR
#define KORE_DEBUGDIR "Deployment"
#endif

#ifdef KORE_ANDROID
struct AAsset;
struct __sFILE;
typedef __sFILE FILE;
#endif

namespace Kore {
#ifdef KORE_ANDROID
	struct FileReaderData {
		int pos;
		int size;
		FILE* file;
		AAsset* asset;
	};
#else
	struct FileReaderData {
		void* file;
		int size;
	};
#endif

	class FileReader : public Reader {
	public:
		enum FileType { Asset, Save };

		FileReader();
		FileReader(const char* filename, FileType type = Asset);
		~FileReader();
		bool open(const char* filename, FileType type = Asset);
		void close();
		int read(void* data, int size) override;
		void* readAll() override;
		int size() const override;
		int pos() const override;
		void seek(int pos) override;

		FileReaderData data;
		void* readdata;
	};

	void setFilesLocation(char* dir);
}
