#pragma once

namespace Kore {
	namespace Audio {
		void init();
		void update();
		void shutdown();

		extern void (*audioCallback)(int samples);

		struct BufferFormat {
			int channels;
			int samplesPerSecond;
			int bitsPerSample;
		};

		struct Buffer {
			BufferFormat format;
			u8* data;
			int dataSize;
			int readLocation;
			int writeLocation;
		};

		extern Buffer buffer;
	}
}
