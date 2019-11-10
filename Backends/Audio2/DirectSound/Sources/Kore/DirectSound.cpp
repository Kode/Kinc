#include "pch.h"

#include <kinc/audio2/audio.h>
#include <kinc/system.h>

#include <Kore/SystemMicrosoft.h>
#include <Kore/Windows.h>

#include <dsound.h>

namespace {
	IDirectSound8 *dsound = nullptr;
	IDirectSoundBuffer *dbuffer = nullptr;
	const DWORD dsize = 50 * 1024;
	const int samplesPerSecond = 44100;
	const int bitsPerSample = 16;

	DWORD lastPlayPosition = 0;
	bool secondHalfFilled = false;

	const int gap = 10 * 1024;
	DWORD writePos = gap;

	void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples) = nullptr;
	kinc_a2_buffer_t a2_buffer;
}

void kinc_a2_init() {
	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = new uint8_t[a2_buffer.data_size];

	kinc_microsoft_affirm(DirectSoundCreate8(nullptr, &dsound, nullptr));
	// TODO (DK) only for the main window?
	kinc_microsoft_affirm(dsound->SetCooperativeLevel(kinc_windows_window_handle(0), DSSCL_PRIORITY));

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

	kinc_microsoft_affirm(dsound->CreateSoundBuffer(&bufferDesc, &dbuffer, nullptr));

	DWORD size1;
	uint8_t *buffer1;
	kinc_microsoft_affirm(dbuffer->Lock(writePos, gap, (void **)&buffer1, &size1, nullptr, nullptr, 0));
	for (int i = 0; i < gap; ++i) buffer1[i] = 0;
	kinc_microsoft_affirm(dbuffer->Unlock(buffer1, size1, nullptr, 0));

	kinc_microsoft_affirm(dbuffer->Play(0, 0, DSBPLAY_LOOPING));
}

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples)) {
	a2_callback = kinc_a2_audio_callback;
}

namespace {
	void copySample(uint8_t *buffer, DWORD &index) {
		float value = *(float *)&a2_buffer.data[a2_buffer.read_location];
		a2_buffer.read_location += 4;
		if (a2_buffer.read_location >= a2_buffer.data_size) {
			a2_buffer.read_location = 0;
		}
		*(int16_t *)&buffer[index] = static_cast<int16_t>(value * 32767);
		index += 2;
	}
}

void kinc_a2_update() {
	DWORD playPosition;
	DWORD writePosition;
	kinc_microsoft_affirm(dbuffer->GetCurrentPosition(&playPosition, &writePosition));

	int dif;
	if (writePos >= writePosition)
		dif = writePos - writePosition;
	else
		dif = dsize - writePosition + writePos;

	if (dif < gap) return;
	if (writePos + gap >= dsize) {
		if (playPosition >= writePos || playPosition <= gap) return;
		if (writePosition >= writePos || writePosition <= gap) return;
	}
	else {
		if (playPosition >= writePos && playPosition <= writePos + gap) return;
		if (writePosition >= writePos && writePosition <= writePos + gap) return;
	}

	a2_callback(&a2_buffer, gap / 2);

	DWORD size1, size2;
	uint8_t *buffer1, *buffer2;
	kinc_microsoft_affirm(dbuffer->Lock(writePos, gap, (void **)&buffer1, &size1, (void **)&buffer2, &size2, 0));

	for (DWORD i = 0; i < size1 - (bitsPerSample / 8 - 1);) {
		copySample(buffer1, i);
	}
	writePos += size1;
	if (buffer2 != nullptr) {
		for (DWORD i = 0; i < size2 - (bitsPerSample / 8 - 1);) {
			copySample(buffer2, i);
		}
		writePos = size2;
	}

	kinc_microsoft_affirm(dbuffer->Unlock(buffer1, size1, buffer2, size2));

	if (writePos >= dsize) writePos -= dsize;
}

void kinc_a2_shutdown() {
	if (dbuffer != nullptr) {
		dbuffer->Release();
		dbuffer = nullptr;
	}
	if (dsound != nullptr) {
		dsound->Release();
		dsound = nullptr;
	}
}
