#include "stdafx.h"
#include <Kt/Sound/Sound.h>
#include <Kt/Graphics/Graphics.h>
#include <Kt/System.h>
#include "WinException.h"
#include <windows.h>
#include <dsound.h>

namespace {
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

	
}

void Kt::Sound::init() {
	
}

void Kt::Sound::shutdown() {
	
}

Kt::Sound::SoundHandle::SoundHandle(Kt::Text filename, bool loops) : loops(loops), secondaryBuffer(nullptr) {
	
 }

Kt::Sound::SoundHandle::~SoundHandle() {
	
}

void Kt::Sound::SoundHandle::play() {
	
}

void Kt::Sound::SoundHandle::setVolume(float volume) {
	
}

int Kt::Sound::SoundHandle::length() {
	return 0;
}

int Kt::Sound::SoundHandle::position() {
	return 0;
}