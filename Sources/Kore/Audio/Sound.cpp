#include "pch.h"
#include "Sound.h"
#include "Audio.h"
#include <Kore/Files/File.h>
#include <string.h>

using namespace Kore;

namespace {
	void affirm(bool) { }
	void affirm(bool, const char*) { }

	struct WaveHeaderType {
		s8 chunkId[4];
		u32 chunkSize;
		s8 format[4];
		s8 subChunkId[4];
		u32 subChunkSize;
		u16 audioFormat;
		u16 numChannels;
		u32 sampleRate;
		u32 bytesPerSecond;
		u16 blockAlign;
		u16 bitsPerSample;
		s8 dataChunkId[4];
		u32 dataSize;
	};
}

Sound::Sound(const char* filename) {
	size_t filenameLength = strlen(filename);
	if (filename[filenameLength - 4] != '.' || filename[filenameLength - 3] != 'w' || filename[filenameLength - 2] != 'a' || filename[filenameLength - 1] != 'v') return;
	DiskFile file;
	file.open(filename, DiskFile::ReadMode);
	char errormsg[100];
	strcpy(errormsg, "Could not load file ");
	strcat(errormsg, filename);
	affirm(file.isOpened(), errormsg);
 
	WaveHeaderType waveFileHeader;
	uint read = file.read(&waveFileHeader, sizeof(waveFileHeader));
	affirm(read == sizeof(waveFileHeader));
 
	affirm(waveFileHeader.chunkId[0] == 'R' && waveFileHeader.chunkId[1] == 'I' && waveFileHeader.chunkId[2] == 'F' && waveFileHeader.chunkId[3] == 'F');
	affirm(waveFileHeader.format[0] == 'W' && waveFileHeader.format[1] == 'A' && waveFileHeader.format[2] == 'V' && waveFileHeader.format[3] == 'E');
	affirm(waveFileHeader.subChunkId[0] == 'f' && waveFileHeader.subChunkId[1] == 'm' && waveFileHeader.subChunkId[2] == 't' && waveFileHeader.subChunkId[3] == ' ');
 
	//affirm(waveFileHeader.audioFormat == WAVE_FORMAT_PCM); 
 
	affirm(waveFileHeader.dataChunkId[0] == 'd' && waveFileHeader.dataChunkId[1] == 'a' && waveFileHeader.dataChunkId[2] == 't' && waveFileHeader.dataChunkId[3] == 'a');

	file.seek(sizeof(WaveHeaderType));
 
	u8* waveData = new u8[waveFileHeader.dataSize];
	affirm(waveData != nullptr);
 
	read = file.read(waveData, waveFileHeader.dataSize);
	affirm(read == waveFileHeader.dataSize);
	
	file.close();

	format.bitsPerSample = waveFileHeader.bitsPerSample;
	format.channels = waveFileHeader.numChannels;
	format.samplesPerSecond = waveFileHeader.sampleRate;
	data = waveData;
	size = waveFileHeader.dataSize;
}
