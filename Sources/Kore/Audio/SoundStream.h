#pragma once

struct stb_vorbis;

namespace Kore {
	class SoundStream {
	public:
		SoundStream(const char* filename, bool looping);
		float nextSample();
		int channels();
		int sampleRate();
		void setLooping(bool loop);
		bool ended();
		float length();
		float position();
	private:
		stb_vorbis* vorbis;
		int chans;
		int rate;
		bool looping;
		bool decoded;
		bool rateDecodedHack;
		bool end;
		float samples[2];
		u8* buffer;
	};
}
