#pragma once

#include <Kore/Audio2/Audio.h>

namespace Kore {
	struct Sound {
	public:
		Sound(const char* filename);
		~Sound();
		Audio2::BufferFormat format;
		float volume();
		void setVolume(float value);
		s16* left;
		s16* right;
		int size;
		float sampleRatePos;
		float length;
	private:
		float myVolume;
	};
}
