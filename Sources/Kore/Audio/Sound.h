#pragma once

#include "Audio.h"

namespace Kore {
	struct Sound {
	public:
		Sound(const char* filename);
		~Sound();
		Audio::BufferFormat format;
		float volume();
		void setVolume(float value);
		u8* data;
		int size;
	private:
		float myVolume;
	};
}
