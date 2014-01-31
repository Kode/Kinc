#include "pch.h"
#include "SoundStream.h"
#include "stb_vorbis.h"
#include <Kore/IO/FileReader.h>

using namespace Kore;

SoundStream::SoundStream(const char* filename, bool looping) : decoded(false), looping(looping), rateDecodedHack(false), end(false) {
	FileReader file(filename);
	buffer = new u8[file.size()];
	u8* filecontent = (u8*)file.readAll();
	for (int i = 0; i < file.size(); ++i) {
		buffer[i] = filecontent[i];
	}
	vorbis = stb_vorbis_open_memory(buffer, file.size(), nullptr, nullptr);
	stb_vorbis_info info = stb_vorbis_get_info(vorbis);
	chans = info.channels;
	rate = info.sample_rate;
}

int SoundStream::channels() {
	return chans;
}

int SoundStream::sampleRate() {
	return rate;
}

void SoundStream::setLooping(bool loop) {
	looping = loop;
}

bool SoundStream::ended() {
	return end;
}

float SoundStream::nextSample() {
	if (rate == 22050) {
		if (rateDecodedHack) {
			if (decoded) {
				decoded = false;
				return samples[0];
			}
			else {
				rateDecodedHack = false;
				decoded = true;
				return samples[1];
			}
		}
	}
	if (decoded) {
		decoded = false; 
		if (chans == 1) {
			return samples[0];
		}
		else {
			return samples[1];
		}
	}
	else {
		int read = stb_vorbis_get_samples_float_interleaved(vorbis, chans, &samples[0], chans);
		if (read == 0) {
			if (looping) {
				stb_vorbis_seek_start(vorbis);
				stb_vorbis_get_samples_float_interleaved(vorbis, chans, &samples[0], chans);
			}
			else {
				end = true;
				return 0.0f;
			}
		}
		decoded = true;
		rateDecodedHack = true;
		return samples[0];
	}
}
