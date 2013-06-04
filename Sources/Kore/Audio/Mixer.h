#pragma once

#include "Sound.h"
#include "SoundStream.h"

namespace Kore {
	namespace Mixer {
		void init();
		void play(Sound* sound);
		void play(SoundStream* stream);
	}
}
