#pragma once

#include <Kore/global.h>

namespace Kore {
	namespace Audio2 {
		void init();
		void update();
		void shutdown();

		extern int samplesPerSecond;

		extern void (*audioCallback)(int samples);

		struct BufferFormat {
			int channels;
			int samplesPerSecond;
			int bitsPerSample;
		};

		struct Buffer {
			BufferFormat format;
			u8 *data;
			int dataSize;
			int readLocation;
			int writeLocation;
		};

		extern Buffer buffer;
	}
}
