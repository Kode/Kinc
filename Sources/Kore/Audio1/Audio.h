#pragma once

#include "Sound.h"
#include "SoundStream.h"

struct kinc_internal_video_sound_stream;

namespace Kore {
	namespace Audio1 {
		struct Channel {
			Sound *sound;
			float position;
			bool loop;
			float volume;
			float pitch;
		};

		struct StreamChannel {
			SoundStream *stream;
			int position;
		};

		struct VideoChannel {
			kinc_internal_video_sound_stream *stream;
			int position;
		};

		void init();
		Channel *play(Sound *sound, bool loop = false, float pitch = 1.0f, bool unique = false);
		void stop(Sound *sound);
		void stop(Channel *channel);
		void play(SoundStream *stream);
		void stop(SoundStream *stream);
		void play(kinc_internal_video_sound_stream *stream);
		void stop(kinc_internal_video_sound_stream *stream);
		void mix(int samples);
	}
}
