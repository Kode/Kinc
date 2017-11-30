#pragma once

#include "Sound.h"
#include "SoundStream.h"

namespace Kore {
	class VideoSoundStream;

	namespace Audio1 {
		struct Channel {
			Sound* sound;
			float position;
			bool loop;
			float volume;
			float pitch;
		};

		struct StreamChannel {
			SoundStream* stream;
			int position;
		};

		struct VideoChannel {
			VideoSoundStream* stream;
			int position;
		};

		void init();
		Channel* play(Sound* sound, bool loop = false, float pitch = 1.0f);
		void stop(Sound* sound);
		void play(SoundStream* stream);
		void stop(SoundStream* stream);
		void play(VideoSoundStream* stream);
		void stop(VideoSoundStream* stream);
		void mix(int samples);
	}
}
