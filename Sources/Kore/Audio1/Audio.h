#pragma once

#include "Sound.h"
#include "SoundStream.h"

namespace Kore {
	class VideoSoundStream;

	namespace Audio1 {
		void init();
		void play(Sound* sound, float pitch = 1.0f);
		void stop(Sound* sound);
		void play(SoundStream* stream);
		void stop(SoundStream* stream);
		void play(VideoSoundStream* stream);
		void stop(VideoSoundStream* stream);
	}
}
