#pragma once

#include "Reader.h"

#if defined SYS_WINDOWS || defined SYS_WINDOWSRT
typedef struct _iobuf FILE;
#endif

namespace Kore {
#ifdef SYS_ANDROID
	struct FileReaderData {
		void* all;
		int pos;
		int size;
	};
#else
	struct FileReaderData {
		FILE* file;
		int size;
	};
#endif

	class FileReader : public Reader {
	public:
		FileReader();
		FileReader(const char* filename);
		~FileReader();
		bool open(const char* filename);
		void close();
		int read(void* data, int size) override;
		void* readAll() override;
		int size() const override;
		int pos() const override;
		void seek(int pos) override;

		FileReaderData data;
	};
}
