#include "pch.h"

#include "SoundStream.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#include <Kore/IO/FileReader.h>

#include <string.h>
#include <stdlib.h>

using namespace Kore;

static Kinc_A1_SoundStream streams[256];
static int nextStream = 0;
static u8 *buffer;
static int bufferIndex;

void initSoundStreams() {
	buffer = (u8*)malloc(1024 * 10);
}

Kinc_A1_SoundStream *Kinc_A1_CreateSoundStream(const char *filename, bool looping) {
	Kinc_A1_SoundStream *stream = &streams[nextStream];
	stream->decoded = false;
	stream->myLooping = looping;
	stream->myVolume = 1;
	stream->rateDecodedHack = false;
	stream->end = false;
	FileReader file(filename);
	stream->buffer = &buffer[bufferIndex];
	bufferIndex += file.size();
	u8* filecontent = (u8*)file.readAll();
	memcpy(stream->buffer, filecontent, file.size());
	stream->vorbis = stb_vorbis_open_memory(buffer, file.size(), nullptr, nullptr);
	if (stream->vorbis != NULL) {
		stb_vorbis_info info = stb_vorbis_get_info(stream->vorbis);
		stream->chans = info.channels;
		stream->rate = info.sample_rate;
	}
	else {
		stream->chans = 2;
		stream->rate = 22050;
	}
	++nextStream;
	return stream;
}

int Kinc_A1_SoundStreamChannels(Kinc_A1_SoundStream *stream) {
	return stream->chans;
}

int Kinc_A1_SoundStreamSampleRate(Kinc_A1_SoundStream *stream) {
	return stream->rate;
}

bool Kinc_A1_SoundStreamLooping(Kinc_A1_SoundStream *stream) {
	return stream->myLooping;
}

void Kinc_A1_SoundStreamSetLooping(Kinc_A1_SoundStream *stream, bool loop) {
	stream->myLooping = loop;
}

float Kinc_A1_SoundStreamVolume(Kinc_A1_SoundStream *stream) {
	return stream->myVolume;
}

void Kinc_A1_SoundStreamSetVolume(Kinc_A1_SoundStream *stream, float value) {
	stream->myVolume = value;
}

bool Kinc_A1_SoundStreamEnded(Kinc_A1_SoundStream *stream) {
	return stream->end;
}

float Kinc_A1_SoundStreamLength(Kinc_A1_SoundStream *stream) {
	if (stream->vorbis == nullptr) return 0;
	return stb_vorbis_stream_length_in_seconds(stream->vorbis);
}

float Kinc_A1_SoundStreamPosition(Kinc_A1_SoundStream *stream) {
	if (stream->vorbis == nullptr) return 0;
	return stb_vorbis_get_sample_offset(stream->vorbis) / stb_vorbis_stream_length_in_samples(stream->vorbis) * Kinc_A1_SoundStreamLength(stream);
}

void Kinc_A1_SoundStreamReset(Kinc_A1_SoundStream *stream) {
	if (stream->vorbis != nullptr) stb_vorbis_seek_start(stream->vorbis);
	stream->end = false;
	stream->rateDecodedHack = false;
	stream->decoded = false;
}

float Kinc_A1_SoundStreamNextSample(Kinc_A1_SoundStream *stream) {
	if (stream->vorbis == nullptr) return 0;
	if (stream->rate == 22050) {
		if (stream->rateDecodedHack) {
			if (stream->decoded) {
				stream->decoded = false;
				return stream->samples[0];
			}
			else {
				stream->rateDecodedHack = false;
				stream->decoded = true;
				return stream->samples[1];
			}
		}
	}
	if (stream->decoded) {
		stream->decoded = false;
		if (stream->chans == 1) {
			return stream->samples[0];
		}
		else {
			return stream->samples[1];
		}
	}
	else {
		int read = stb_vorbis_get_samples_float_interleaved(stream->vorbis, stream->chans, &stream->samples[0], stream->chans);
		if (read == 0) {
			if (Kinc_A1_SoundStreamLooping(stream)) {
				stb_vorbis_seek_start(stream->vorbis);
				stb_vorbis_get_samples_float_interleaved(stream->vorbis, stream->chans, &stream->samples[0], stream->chans);
			}
			else {
				stream->end = true;
				return 0.0f;
			}
		}
		stream->decoded = true;
		stream->rateDecodedHack = true;
		return stream->samples[0];
	}
}
