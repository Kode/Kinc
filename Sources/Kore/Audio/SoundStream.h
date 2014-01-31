#pragma once

struct stb_vorbis;

namespace Kore {
	class SoundStream {
	public:
		SoundStream(const char* filename, bool looping);
		float nextSample();
		int channels();
		int sampleRate();
	private:
		stb_vorbis* vorbis;
		int chans;
		int rate;
		bool looping;
		bool decoded;
		bool rateDecodedHack;
		float samples[2];
		u8* buffer;
	};
}
