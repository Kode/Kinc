#pragma once

#include "Writer.h"

#include <kinc/io/filewriter.h>

namespace Kore {
	class FileWriter : public Writer {
	public:
		FileWriter();
		FileWriter(const char* filename);
		~FileWriter();
		bool open(const char* filename);
		void close();
		void write(void* data, int size) override;
	private:
		Kinc_FileWriter writer;
	};
}
