#include "pch.h"
#include "Audio.h"
#include <Kore/System.h>
#include <dsound.h>
#include <stdexcept>
#include <stdio.h>

using namespace Kore;

namespace {
	void affirm(HRESULT result) {
		if (result != DS_OK) {
			throw std::runtime_error("arr");
		}
	}

	IDirectSound8* dsound = nullptr;
	IDirectSoundBuffer* dbuffer = nullptr;
	const DWORD dsize = 50 * 1024;
	const int samplesPerSecond = 44100;
	const int bitsPerSample = 16;
	
	s16* mixbuffer;
	const int bufferSize = samplesPerSecond * 3000;
	int bufferPosition = 0;

	DWORD lastPlayPosition = 0;
	bool secondHalfFilled = false;

	const int gap = 10 * 1024;
	int writePos = gap;
}

void Audio::init() {
	affirm(DirectSoundCreate8(nullptr, &dsound, nullptr));
	affirm(dsound->SetCooperativeLevel((HWND)System::windowHandle(), DSSCL_PRIORITY));

	WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = samplesPerSecond;
	waveFormat.wBitsPerSample = bitsPerSample;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	DSBUFFERDESC bufferDesc;
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_GLOBALFOCUS;
	bufferDesc.dwBufferBytes = dsize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;
 
	affirm(dsound->CreateSoundBuffer(&bufferDesc, &dbuffer, nullptr));

	mixbuffer = new s16[bufferSize];
	for (int i = 0; i < bufferSize; ++i) mixbuffer[i] = 0;
	
	DWORD size1;
	u8* buffer1;
	affirm(dbuffer->Lock(writePos, gap, (void**)&buffer1, &size1, nullptr, nullptr, 0));
	for (int i = 0; i < gap; ++i) buffer1[i] = 0;
	affirm(dbuffer->Unlock(buffer1, size1, nullptr, 0));

	affirm(dbuffer->Play(0, 0, DSBPLAY_LOOPING));
}

void Audio::play(s16* data, int size) {
	int position = bufferPosition;
	for (int i = 0; i < size; ++i) {
		mixbuffer[position++] = data[i];//(float)data[i] / (float)32767;
	}
}

namespace {
	void copySample(u8* buffer, DWORD& index) {
		//float value = ::buffer[bufferPosition++];
		//s16 sample = static_cast<s16>(value * 32767);
		s16* hmm = (s16*)&buffer[index];
		*hmm = mixbuffer[bufferPosition++];
		if (*hmm != 0) {
			int a = 3;
			++a;
		}
		index += 2;
	}
}

void Audio::update() {
	DWORD playPosition;
	DWORD writePosition;
	affirm(dbuffer->GetCurrentPosition(&playPosition, &writePosition));

	int dif;
	if (writePos >= writePosition) dif = writePos - writePosition;
	else dif = dsize - writePosition + writePos;

	if (dif < gap) return;
	if (writePos + gap >= dsize) {
		if (playPosition >= writePos || playPosition <= gap) return;
		if (writePosition >= writePos || writePosition <= gap) return;
	}
	else {
		if (playPosition >= writePos && playPosition <= writePos + gap) return;
		if (writePosition >= writePos && writePosition <= writePos + gap) return;
	}

	DWORD size1, size2;
	u8 *buffer1, *buffer2;
	affirm(dbuffer->Lock(writePos, gap, (void**)&buffer1, &size1, (void**)&buffer2, &size2, 0));
	
	/*for (DWORD i = 0; i < size1 - (bitsPerSample / 8 - 1); ) {
		copySample(buffer1, i);
		
	}*/
	memcpy(buffer1, &mixbuffer[bufferPosition], size1);
	bufferPosition += size1 / 2;
	writePos += size1;
	if (buffer2 != nullptr) {
		/*for (DWORD i = 0; i < size2 - (bitsPerSample / 8 - 1); ) {
			copySample(buffer2, i);
		}*/
		memcpy(buffer2, &mixbuffer[bufferPosition], size2);
		bufferPosition += size2 / 2;
		writePos = size2;
	}

	affirm(dbuffer->Unlock(buffer1, size1, buffer2, size2));

	if (writePos >= dsize) writePos -= dsize;
}

void Audio::shutdown() {
	if (dbuffer != nullptr) {
		dbuffer->Release();
		dbuffer = nullptr;
	}
	if (dsound != nullptr) {
		dsound->Release();
		dsound = nullptr;
	}
}
