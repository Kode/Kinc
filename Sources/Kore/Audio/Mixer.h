#pragma once

#include "Sound.h"
#include "SoundStream.h"

namespace Kore {
	class VideoSoundStream;

	namespace Mixer {
		void init();
		void play(Sound* sound);
		void stop(Sound* sound);
		void play(SoundStream* stream);
		void stop(SoundStream* stream);
		void play(VideoSoundStream* stream);
		void stop(VideoSoundStream* stream);
	}
}
