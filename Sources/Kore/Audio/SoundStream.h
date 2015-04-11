#pragma once

struct stb_vorbis;

namespace Kore {
	class SoundStream {
	public:
		SoundStream(const char* filename, bool looping);
		float nextSample();
		int channels();
		int sampleRate();
		bool looping();
		void setLooping(bool loop);
		bool ended();
		float length();
		float position();
		void reset();
		float volume();
		void setVolume(float value);
	private:
		stb_vorbis* vorbis;
		int chans;
		int rate;
		bool myLooping;
		float myVolume;
		bool decoded;
		bool rateDecodedHack;
		bool end;
		float samples[2];
		u8* buffer;
	};
}
