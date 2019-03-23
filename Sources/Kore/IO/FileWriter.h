#pragma once

#include "Writer.h"

#include <Kinc/IO/FileWriter.h>

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
