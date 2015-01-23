#pragma once

#include "Sound.h"
#include "SoundStream.h"

namespace Kore {
#ifdef KOREVIDEO
	class VideoSoundStream;
#endif

	namespace Mixer {
		void init();
		void play(Sound* sound);
		void stop(Sound* sound);
		void play(SoundStream* stream);
		void stop(SoundStream* stream);
#ifdef KOREVIDEO
		void play(VideoSoundStream* stream);
		void stop(VideoSoundStream* stream);
#endif
	}
}
