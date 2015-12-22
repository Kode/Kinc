#include "pch.h"
#include "Sound.h"
#include "Audio.h"
#include "stb_vorbis.h"
#include <Kore/IO/FileReader.h>
#include <Kore/Error.h>
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
		u32 chunksize = Reader::readU32LE(data); data += 4;
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
}

Sound::Sound(const char* filename) : myVolume(1), size(0), data(0) {
	size_t filenameLength = strlen(filename);
	
	if (strncmp(&filename[filenameLength - 4], ".ogg", 4) == 0) {
		FileReader file(filename);
		u8* filedata = (u8*)file.readAll();
		size = 4 * stb_vorbis_decode_memory(filedata, file.size(), &format.channels, (short**)&data);
		format.bitsPerSample = 16;
		format.samplesPerSecond = 44100;
	}
	else if (strncmp(&filename[filenameLength - 4], ".wav", 4) == 0) {
		WaveData wave = { 0 };
		{
			FileReader file(filename);
			u8* filedata = (u8*)file.readAll();
			u8* data = filedata;

			checkFOURCC(data, "RIFF");
			u32 filesize = Reader::readU32LE(data); data += 4;
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
}

Sound::~Sound() {
	delete[] data;
	data = nullptr;
}

float Sound::volume() {
	return myVolume;
}

void Sound::setVolume(float value) {
	myVolume = value;
}
