#pragma once

#include "Reader.h"

#ifdef SYS_ANDROID
struct AAsset;
struct __sFILE;
typedef __sFILE FILE;
#endif

namespace Kore {
#ifdef SYS_ANDROID
	struct FileReaderData {
		int pos;
		int size;
		FILE* file;
		AAsset* asset;
	};
#elif defined(SYS_WIIU)
	struct FileReaderData {
		int file;
		int pos;
	};
#else
	struct FileReaderData {
		void* file;
		int size;
	};
#endif

	class FileReader : public Reader {
	public:
		enum FileType {
			Asset, Save
		};

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
}
