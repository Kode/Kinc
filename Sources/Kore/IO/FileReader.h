#pragma once

#include <Kinc/IO/FileReader.h>

#include "Reader.h"

namespace Kore {
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
		int size() override;
		int pos() override;
		void seek(int pos) override;

		kinc_file_reader_t reader;
		FileType type;
		void* readdata;
	};
}
