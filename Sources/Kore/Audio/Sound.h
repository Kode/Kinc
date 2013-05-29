#pragma once

#include "Audio.h"

namespace Kore {
	struct Sound {
	public:
		Sound(const char* filename);
		Audio::BufferFormat format;
		u8* data;
		int size;
	};
}
