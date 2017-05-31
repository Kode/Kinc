#pragma once

#include <Kore/Math/Vector.h>

namespace Kore {
	namespace Audio3 {
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

		typedef void(*AudioCallback)(int samples);

		struct Channel {
			vec3 origin;
			AudioCallback callback;
			Buffer buffer;
			bool active;
		};
				
		void init();
		void update();
		void shutdown();

		Channel* createChannel(vec3 origin, AudioCallback callback);
		void destroyChannel(Channel* channel);
	}
}
