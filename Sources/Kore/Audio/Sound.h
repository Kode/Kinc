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
		u8* left;
		u8* right;
		int size;
	private:
		float myVolume;
	};
}
