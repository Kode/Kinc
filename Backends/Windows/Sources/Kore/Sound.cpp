#include "pch.h"
#include <Kore/Files/File.h>
#include <Kore/Sound/Sound.h>
#include <Kore/System.h>
#include <dsound.h>

using namespace Kore;

#define HRESULT uint

namespace {
	void affirm(bool, const char*) { }
	void affirm(HRESULT) { }

	struct WaveHeaderType {
		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};

	//IDirectSoundBuffer* primaryBuffer = nullptr;
}

IDirectSound8* dsound = nullptr;

void Kore::Sound::init() {
	HRESULT result;
	result = DirectSoundCreate8(nullptr, &dsound, nullptr);
	affirm(result == DS_OK);
	result = dsound->SetCooperativeLevel((HWND)System::windowHandle(), DSSCL_PRIORITY);
	affirm(result == DS_OK);

	/*DSBUFFERDESC bufferDesc;
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;
 
	result = dsound->CreateSoundBuffer(&bufferDesc, &primaryBuffer, nullptr);
	affirm(result == DS_OK);

	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;
 
	result = primaryBuffer->SetFormat(&waveFormat);
	affirm(result == DS_OK);*/
}

void Kore::Sound::shutdown() {
	/*if (primaryBuffer != nullptr) {
		primaryBuffer->Release();
		primaryBuffer = nullptr;
	}*/
	if (dsound != nullptr) {
		dsound->Release();
		dsound = nullptr;
	}
}

Kore::Sound::SoundHandle::SoundHandle(const char* filename, bool loops) : loops(loops), secondaryBuffer(nullptr), playing(false) {
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
 
	affirm(waveFileHeader.audioFormat == WAVE_FORMAT_PCM); 
 
	affirm(waveFileHeader.dataChunkId[0] == 'd' && waveFileHeader.dataChunkId[1] == 'a' && waveFileHeader.dataChunkId[2] == 't' && waveFileHeader.dataChunkId[3] == 'a');

	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = waveFileHeader.sampleRate;
	waveFormat.wBitsPerSample = waveFileHeader.bitsPerSample;
	waveFormat.nChannels = waveFileHeader.numChannels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	sampleRate = waveFileHeader.sampleRate;
	bitsPerSample = waveFileHeader.bitsPerSample;
 
	DSBUFFERDESC bufferDesc;
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;
	
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;
	result = dsound->CreateSoundBuffer(&bufferDesc, &tempBuffer, nullptr);
	affirm(result == DS_OK);
 
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&secondaryBuffer);
	affirm(result == DS_OK);
 
	tempBuffer->Release();
	tempBuffer = nullptr;

	file.seek(sizeof(WaveHeaderType));
 
	unsigned char* waveData = new unsigned char[waveFileHeader.dataSize];
	affirm(waveData != nullptr);
 
	read = file.read(waveData, waveFileHeader.dataSize);
	affirm(read == waveFileHeader.dataSize);
	
	file.close();

	size = waveFileHeader.dataSize;
	
	unsigned char* bufferPtr;
	unsigned long bufferSize;
	result = secondaryBuffer->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, nullptr, 0, 0);
	affirm(result == DS_OK);
 
	memcpy(bufferPtr, waveData, waveFileHeader.dataSize);
 
	result = secondaryBuffer->Unlock((void*)bufferPtr, bufferSize, nullptr, 0);
	affirm(result == DS_OK);
	
	delete[] waveData;
	waveData = nullptr;
 }

Kore::Sound::SoundHandle::~SoundHandle() {
	if (secondaryBuffer != nullptr) {
		secondaryBuffer->Release();
		secondaryBuffer = nullptr;
	}
}

int Kore::Sound::SoundHandle::length() {
	return static_cast<int>(size / (bitsPerSample / 8) / sampleRate);
}

int Kore::Sound::SoundHandle::position() {
	DWORD position;
	secondaryBuffer->GetCurrentPosition(&position, nullptr);
	DWORD status;
	secondaryBuffer->GetStatus(&status);
	if (((status & DSBSTATUS_PLAYING) == 0) && position == 0) return length();
	return static_cast<int>(position / (bitsPerSample / 8) / sampleRate);
}

void Kore::Sound::SoundHandle::play() {
	if (secondaryBuffer == nullptr) return;
	
	HRESULT result;
	result = secondaryBuffer->SetCurrentPosition(0);
	affirm(result == DS_OK);
	
	result = secondaryBuffer->SetVolume(DSBVOLUME_MAX);
	affirm(result == DS_OK);
 
	result = secondaryBuffer->Play(0, 0, loops ? DSBPLAY_LOOPING : 0);
	affirm(result == DS_OK);

	playing = true;
}

void Kore::Sound::SoundHandle::setVolume(float volume) {
	if (secondaryBuffer == nullptr) return;
	volume = min(abs(volume), 1.0f);
	HRESULT result = secondaryBuffer->SetVolume((LONG)(DSBVOLUME_MIN + (DSBVOLUME_MAX - DSBVOLUME_MIN) * volume));
	affirm(result == DS_OK);
}
