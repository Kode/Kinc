#include "pch.h"

#include "Sound.h"

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"
#include <Kore/Audio2/Audio.h>
#include <Kore/Error.h>
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
			Kore::affirm(*data == fourcc[i]);
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
			affirm(wave.data != nullptr);
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
			left[i]  = convert8to16(data[i * 2 + 0]);
			right[i] = convert8to16(data[i * 2 + 1]);
		}
	}

	void splitStereo16(s16* data, int size, s16* left, s16* right) {
		for (int i = 0; i < size; ++i) {
			left[i]  = data[i * 2 + 0];
			right[i] = data[i * 2 + 1];
		}
	}

	void splitMono8(u8* data, int size, s16* left, s16* right) {
		for (int i = 0; i < size; ++i) {
			left[i]  = convert8to16(data[i]);
			right[i] = convert8to16(data[i]);
		}
	}
	
	void splitMono16(s16* data, int size, s16* left, s16* right) {
		for (int i = 0; i < size; ++i) {
			left[i]  = data[i];
			right[i] = data[i];
		}
	}
}

Sound::Sound(const char* filename) : myVolume(1), size(0), left(0), right(0) {
	size_t filenameLength = strlen(filename);
	u8* data = nullptr;

	if (strncmp(&filename[filenameLength - 4], ".ogg", 4) == 0) {
		FileReader file(filename);
		u8* filedata = (u8*)file.readAll();
		int samples = stb_vorbis_decode_memory(filedata, file.size(), &format.channels, &format.samplesPerSecond, (short**)&data);
		size = samples * 2 * format.channels;
		format.bitsPerSample = 16;
	}
	else if (strncmp(&filename[filenameLength - 4], ".wav", 4) == 0) {
		WaveData wave = { 0 };
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

		format.bitsPerSample = wave.bitsPerSample;
		format.channels = wave.numChannels;
		format.samplesPerSecond = wave.sampleRate;
		data = wave.data;
		size = wave.dataSize;
	}
	else {
		assert(false);
	}

	if (format.channels == 1) {
		if (format.bitsPerSample == 8) {
			left = new s16[size];
			right = new s16[size];
			splitMono8(data, size, left, right);
		}
		else if (format.bitsPerSample == 16) {
			size /= 2;
			left = new s16[size];
			right = new s16[size];
			splitMono16((s16*)data, size, left, right);
		}
		else {
			assert(false);
		}
	}
	else {
		// Left and right channel are in s16 audio stream, alternating.
		if (format.bitsPerSample == 8) {
			size /= 2;
			left = new s16[size];
			right = new s16[size];
			splitStereo8(data, size, left, right);
		}
		else if (format.bitsPerSample == 16) {
			size /= 4;
			left = new s16[size];
			right = new s16[size];
			splitStereo16((s16*)data, size, left, right);
		}
		else {
			assert(false);
		}
	}
	sampleRatePos = 44100 / (float)format.samplesPerSecond;
	delete[] data;
}

Sound::~Sound() {
	delete[] left;
	delete[] right;
	left = nullptr;
	right = nullptr;
}

float Sound::volume() {
	return myVolume;
}

void Sound::setVolume(float value) {
	myVolume = value;
}
