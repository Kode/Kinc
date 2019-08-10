#include "pch.h"

#include "sound.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#include <kinc/audio2/audio.h>
#include <kinc/error.h>
#include <Kore/IO/FileReader.h>

#include <assert.h>
#include <string.h>

using namespace Kore;

namespace {
	struct WaveData {
		u16 audioFormat;
		u16 numChannels;
		u32 sampleRate;
		u32 bytesPerSecond;
		u16 bitsPerSample;
		u32 dataSize;
		u8* data;
	};

	void checkFOURCC(u8*& data, const char* fourcc) {
		for (int i = 0; i < 4; ++i) {
			kinc_affirm(*data == fourcc[i]);
			++data;
		}
	}

	void readFOURCC(u8*& data, char* fourcc) {
		for (int i = 0; i < 4; ++i) {
			fourcc[i] = *data;
			++data;
		}
		fourcc[4] = 0;
	}

	void readChunk(u8*& data, WaveData& wave) {
		char fourcc[5];
		readFOURCC(data, fourcc);
		u32 chunksize = Reader::readU32LE(data);
		data += 4;
		if (strcmp(fourcc, "fmt ") == 0) {
			wave.audioFormat = Reader::readU16LE(data + 0);
			wave.numChannels = Reader::readU16LE(data + 2);
			wave.sampleRate = Reader::readU32LE(data + 4);
			wave.bytesPerSecond = Reader::readU32LE(data + 8);
			wave.bitsPerSample = Reader::readU16LE(data + 14);
			data += chunksize;
		}
		else if (strcmp(fourcc, "data") == 0) {
			wave.dataSize = chunksize;
			wave.data = new u8[chunksize];
			kinc_affirm(wave.data != nullptr);
			memcpy(wave.data, data, chunksize);
			data += chunksize;
		}
		else {
			data += chunksize;
		}
	}

	s16 convert8to16(u8 sample) {
		return (sample - 127) << 8;
	}

	void splitStereo8(u8* data, int size, s16* left, s16* right) {
		for (int i = 0; i < size; ++i) {
			left[i] = convert8to16(data[i * 2 + 0]);
			right[i] = convert8to16(data[i * 2 + 1]);
		}
	}

	void splitStereo16(s16* data, int size, s16* left, s16* right) {
		for (int i = 0; i < size; ++i) {
			left[i] = data[i * 2 + 0];
			right[i] = data[i * 2 + 1];
		}
	}

	void splitMono8(u8* data, int size, s16* left, s16* right) {
		for (int i = 0; i < size; ++i) {
			left[i] = convert8to16(data[i]);
			right[i] = convert8to16(data[i]);
		}
	}

	void splitMono16(s16* data, int size, s16* left, s16* right) {
		for (int i = 0; i < size; ++i) {
			left[i] = data[i];
			right[i] = data[i];
		}
	}
}

#define MAXIMUM_SOUNDS 256
static kinc_a1_sound_t sounds[MAXIMUM_SOUNDS];
static int nextSoundIndex = 0;

kinc_a1_sound_t *kinc_a1_sound_create(const char *filename) {
	assert(nextSoundIndex < MAXIMUM_SOUNDS);
	kinc_a1_sound_t *sound = &sounds[nextSoundIndex];
	sound->my_volume = 1;
	sound->size = 0;
	sound->left = NULL;
	sound->right = NULL;
	size_t filenameLength = strlen(filename);
	u8* data = nullptr;

	if (strncmp(&filename[filenameLength - 4], ".ogg", 4) == 0) {
		FileReader file(filename);
		u8* filedata = (u8*)file.readAll();
		int samples = stb_vorbis_decode_memory(filedata, file.size(), &sound->format.channels, &sound->format.samples_per_second, (short**)&data);
		sound->size = samples * 2 * sound->format.channels;
		sound->format.bits_per_sample = 16;
	}
	else if (strncmp(&filename[filenameLength - 4], ".wav", 4) == 0) {
		WaveData wave = {0};
		{
			FileReader file(filename);
			u8* filedata = (u8*)file.readAll();
			u8* data = filedata;

			checkFOURCC(data, "RIFF");
			u32 filesize = Reader::readU32LE(data);
			data += 4;
			checkFOURCC(data, "WAVE");
			while (data + 8 - filedata < (spint)filesize) {
				readChunk(data, wave);
			}

			file.close();
		}

		sound->format.bits_per_sample = wave.bitsPerSample;
		sound->format.channels = wave.numChannels;
		sound->format.samples_per_second = wave.sampleRate;
		data = wave.data;
		sound->size = wave.dataSize;
	}
	else {
		assert(false);
	}

	if (sound->format.channels == 1) {
		if (sound->format.bits_per_sample == 8) {
			sound->left = new s16[sound->size];
			sound->right = new s16[sound->size];
			splitMono8(data, sound->size, sound->left, sound->right);
		}
		else if (sound->format.bits_per_sample == 16) {
			sound->size /= 2;
			sound->left = new s16[sound->size];
			sound->right = new s16[sound->size];
			splitMono16((s16*)data, sound->size, sound->left, sound->right);
		}
		else {
			kinc_affirm(false);
		}
	}
	else {
		// Left and right channel are in s16 audio stream, alternating.
		if (sound->format.bits_per_sample == 8) {
			sound->size /= 2;
			sound->left = new s16[sound->size];
			sound->right = new s16[sound->size];
			splitStereo8(data, sound->size, sound->left, sound->right);
		}
		else if (sound->format.bits_per_sample == 16) {
			sound->size /= 4;
			sound->left = new s16[sound->size];
			sound->right = new s16[sound->size];
			splitStereo16((s16*)data, sound->size, sound->left, sound->right);
		}
		else {
			kinc_affirm(false);
		}
	}
	sound->sample_rate_pos = 44100 / (float)sound->format.samples_per_second;
	delete[] data;

	return sound;
}

void kinc_a1_sound_destroy(kinc_a1_sound_t *sound) {
	delete[] sound->left;
	delete[] sound->right;
	sound->left = NULL;
	sound->right = NULL;
}

float kinc_a1_sound_volume(kinc_a1_sound_t *sound) {
	return sound->my_volume;
}

void kinc_a1_sound_set_volume(kinc_a1_sound_t *sound, float value) {
	sound->my_volume = value;
}
