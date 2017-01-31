#pragma once

#include <Kore/IO/Reader.h>

namespace Kore {

	class BufferReader : public Reader {
		u8* buffer;
		int bufferSize;
		int position;
		void* readAllBuffer;
	public:
		BufferReader(void const* buffer, int size);
		virtual ~BufferReader();
		int read(void* data, int size) override;
		void* readAll() override;
		int size() const override;
		int pos() const override;
		void seek(int pos) override;
	};

}
