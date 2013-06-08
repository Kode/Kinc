#include "pch.h"
#include <Kore/Audio/Audio.h>
#include <Kore/System.h>
#include <Kore/WinError.h>
#include <dsound.h>

using namespace Kore;

namespace {
	IDirectSound8* dsound = nullptr;
	IDirectSoundBuffer* dbuffer = nullptr;
	const DWORD dsize = 50 * 1024;
	const int samplesPerSecond = 44100;
	const int bitsPerSample = 16;
	
	DWORD lastPlayPosition = 0;
	bool secondHalfFilled = false;

	const int gap = 10 * 1024;
	DWORD writePos = gap;
}

void Audio::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];

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
	
	DWORD size1;
	u8* buffer1;
	affirm(dbuffer->Lock(writePos, gap, (void**)&buffer1, &size1, nullptr, nullptr, 0));
	for (int i = 0; i < gap; ++i) buffer1[i] = 0;
	affirm(dbuffer->Unlock(buffer1, size1, nullptr, 0));

	affirm(dbuffer->Play(0, 0, DSBPLAY_LOOPING));
}

namespace {
	void copySample(u8* buffer, DWORD& index) {
		float value = *(float*)&Audio::buffer.data[Audio::buffer.readLocation];
		Audio::buffer.readLocation += 4;
		if (Audio::buffer.readLocation >= Audio::buffer.dataSize) Audio::buffer.readLocation = 0;
		*(s16*)&buffer[index] = static_cast<s16>(value * 32767);
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

	audioCallback(gap / 2);

	DWORD size1, size2;
	u8 *buffer1, *buffer2;
	affirm(dbuffer->Lock(writePos, gap, (void**)&buffer1, &size1, (void**)&buffer2, &size2, 0));
	
	for (DWORD i = 0; i < size1 - (bitsPerSample / 8 - 1); ) {
		copySample(buffer1, i);	
	}
	writePos += size1;
	if (buffer2 != nullptr) {
		for (DWORD i = 0; i < size2 - (bitsPerSample / 8 - 1); ) {
			copySample(buffer2, i);
		}
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
