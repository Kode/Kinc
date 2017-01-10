#include "pch.h"

#include "Audio.h"
#include "Sound.h"

#include "stb_vorbis.h"
#include <Kore/Error.h>
#include <Kore/IO/FileReader.h>
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

	void split(s16* data, int size, s16* left, s16* right) {
		for (int i = 0; i < size / 4; ++i) {
			left[i] = data[i * 2];
			right[i] = data[i * 2 + 1];
		}
	}
	
	void splitMono(s16* data, int size, s16* left, s16* right) {
		for (int i = 0; i < size / 4; ++i) {
			left[i] = data[i];
			right[i] = data[i];
		}
	}
}

Sound::Sound(const char* filename) : myVolume(1), size(0), data(0), left(0), right(0) {
	size_t filenameLength = strlen(filename);

	if (strncmp(&filename[filenameLength - 4], ".ogg", 4) == 0) {
		FileReader file(filename);
		u8* filedata = (u8*)file.readAll();
		size = 4 * stb_vorbis_decode_memory(filedata, file.size(), &format.channels, (short**)&data);
		format.bitsPerSample = 16;
		format.samplesPerSecond = 44100;
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

		format.bitsPerSample = wave.bitsPerSample;
		format.channels = wave.numChannels;
		format.samplesPerSecond = wave.sampleRate;
		data = wave.data;
		size = wave.dataSize;
		left = new s16[size / 2];
		right = new s16[size / 2];
		if (format.channels == 1) {
			splitMono((s16*)data, size, left, right);
		}
		else {
			// Left and right channel are in s16 audio stream, alternating.
			split((s16*)data, size, left, right);
		}
		sampleRatePos = 44100 / (float)format.samplesPerSecond;
	}
}

Sound::~Sound() {
	delete[] data;
	delete[] left;
	delete[] right;
	data = nullptr;
	left = nullptr;
	right = nullptr;
}

float Sound::volume() {
	return myVolume;
}

void Sound::setVolume(float value) {
	myVolume = value;
}
