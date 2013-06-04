#pragma once

struct stb_vorbis;

namespace Kore {
	class SoundStream {
	public:
		SoundStream(const char* filename, bool looping);
		float nextSample();
	private:
		stb_vorbis* vorbis;
		bool looping;
		bool decoded;
		float samples[2];
		u8* buffer;
	};
}
