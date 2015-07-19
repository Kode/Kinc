#include "pch.h"
#include "SoundStream.h"
#include "stb_vorbis.h"
#include <Kore/IO/FileReader.h>
#include <string.h>

using namespace Kore;

SoundStream::SoundStream(const char* filename, bool looping) : decoded(false), myLooping(looping), myVolume(1), rateDecodedHack(false), end(false) {
	FileReader file(filename);
	buffer = new u8[file.size()];
	u8* filecontent = (u8*)file.readAll();
	memcpy(buffer, filecontent, file.size());
	vorbis = stb_vorbis_open_memory(buffer, file.size(), nullptr, nullptr);
    if (vorbis != nullptr) {
        stb_vorbis_info info = stb_vorbis_get_info(vorbis);
        chans = info.channels;
        rate = info.sample_rate;
    }
    else {
        chans = 2;
        rate = 22050;
    }
}

int SoundStream::channels() {
	return chans;
}

int SoundStream::sampleRate() {
	return rate;
}

bool SoundStream::looping() {
	return myLooping;
}

void SoundStream::setLooping(bool loop) {
	myLooping = loop;
}

float SoundStream::volume() {
	return myVolume;
}

void SoundStream::setVolume(float value) {
	myVolume = value;
}

bool SoundStream::ended() {
	return end;
}

float SoundStream::length() {
    if (vorbis == nullptr) return 0;
	return stb_vorbis_stream_length_in_seconds(vorbis);
}

float SoundStream::position() {
    if (vorbis == nullptr) return 0;
	return stb_vorbis_get_sample_offset(vorbis) / stb_vorbis_stream_length_in_samples(vorbis) * length();
}

void SoundStream::reset() {
    if (vorbis != nullptr) stb_vorbis_seek_start(vorbis);
	end = false;
	rateDecodedHack = false;
	decoded = false;
}

float SoundStream::nextSample() {
    if (vorbis == nullptr) return 0;
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
			if (looping()) {
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
