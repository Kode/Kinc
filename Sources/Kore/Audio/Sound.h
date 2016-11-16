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
		s16* left;
		s16* right;
		int size;
		float sampleRatePos;

	private:
		float myVolume;
	};
}
