#include "pch.h"
#include "Audio.h"
#include <Kore/System.h>
#include <stdexcept>
#include <stdio.h>

using namespace Kore;

void (*Audio::audioCallback)(int samples);
Audio::Buffer Audio::buffer;

#ifdef SYS_OSX

#include <CoreServices/CoreServices.h>
#include <CoreAudio/AudioHardware.h>

namespace {
	const int samplesPerSecond = 44100;
	float* mixbuffer;
	const int bufferSize = samplesPerSecond * 1000;
	int bufferPosition = 0;
	
	void affirm(OSStatus err) {
		if (err != kAudioHardwareNoError) {
			fprintf(stderr, "Error: %i\n", err);
		}
	}
	
	bool initialized;
	bool soundPlaying;
	AudioDeviceID device;
	UInt32 deviceBufferSize;
	AudioStreamBasicDescription deviceFormat;
	
	AudioDeviceIOProcID theIOProcID = nullptr;
	
	OSStatus appIOProc(AudioDeviceID inDevice, const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime, AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime, void* userdata) {
		int numSamples = deviceBufferSize / deviceFormat.mBytesPerFrame;
		float *out = (float*)outOutputData->mBuffers[0].mData;
		for (int i = 0; i < numSamples; ++i) {
			*out++ = mixbuffer[bufferPosition++]; //left
			*out++ = mixbuffer[bufferPosition++]; //right
		}
		return kAudioHardwareNoError;
	}
}

void Audio::play(s16* data, int size) {
	int position = bufferPosition;
	for (int i = 0; i < size; ++i) {
		mixbuffer[position++] = (float)data[i] / (float)32767;
	}
}

void Audio::init() {
	mixbuffer = new float[bufferSize];
	for (int i = 0; i < bufferSize; ++i) mixbuffer[i] = 0;
	
	device = kAudioDeviceUnknown;

	initialized = false;

	UInt32 size = sizeof(AudioDeviceID);
	AudioObjectPropertyAddress address = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	affirm(AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, &device));
	
	size = sizeof(UInt32);
	address.mSelector = kAudioDevicePropertyBufferSize;
	address.mScope = kAudioDevicePropertyScopeOutput;
	affirm(AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &deviceBufferSize));
	
	fprintf(stderr, "deviceBufferSize = %i\n", deviceBufferSize);

	size = sizeof(AudioStreamBasicDescription);
	address.mSelector = kAudioDevicePropertyStreamFormat;
	address.mScope = kAudioDevicePropertyScopeOutput;
	
	affirm(AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &deviceFormat));

	if (deviceFormat.mFormatID != kAudioFormatLinearPCM) {
		fprintf(stderr, "mFormatID !=  kAudioFormatLinearPCM\n");
		return;
	}

	if (!(deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsFloat)) {
		fprintf(stderr, "Only works with float format.\n");
		return;
	}

	initialized = true;

	fprintf(stderr, "mSampleRate = %g\n", deviceFormat.mSampleRate);
	fprintf(stderr, "mFormatFlags = %08X\n", (unsigned int)deviceFormat.mFormatFlags);
	fprintf(stderr, "mBytesPerPacket = %d\n", (unsigned int)deviceFormat.mBytesPerPacket);
	fprintf(stderr, "mFramesPerPacket = %d\n", (unsigned int)deviceFormat.mFramesPerPacket);
	fprintf(stderr, "mChannelsPerFrame = %d\n", (unsigned int)deviceFormat.mChannelsPerFrame);
	fprintf(stderr, "mBytesPerFrame = %d\n", (unsigned int)deviceFormat.mBytesPerFrame);
	fprintf(stderr, "mBitsPerChannel = %d\n", (unsigned int)deviceFormat.mBitsPerChannel);
	
	if (soundPlaying) return;

	affirm(AudioDeviceCreateIOProcID(device, appIOProc, nullptr, &theIOProcID));
	affirm(AudioDeviceStart(device, theIOProcID));
	
	soundPlaying = true;
}

void Audio::update() {
	
}

void Audio::shutdown() {
	if (!initialized) return;
	if (!soundPlaying) return;

	affirm(AudioDeviceStop(device, theIOProcID));
	affirm(AudioDeviceDestroyIOProcID(device, theIOProcID));
	
	soundPlaying = false;
}

#endif

#ifdef SYS_WINDOWS

#include <dsound.h>

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
	
	float* mixbuffer;
	const int bufferSize = samplesPerSecond * 3000;
	int bufferPosition = 0;

	DWORD lastPlayPosition = 0;
	bool secondHalfFilled = false;

	const int gap = 10 * 1024;
	int writePos = gap;
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

	mixbuffer = new float[bufferSize];
	for (int i = 0; i < bufferSize; ++i) mixbuffer[i] = 0;
	
	DWORD size1;
	u8* buffer1;
	affirm(dbuffer->Lock(writePos, gap, (void**)&buffer1, &size1, nullptr, nullptr, 0));
	for (int i = 0; i < gap; ++i) buffer1[i] = 0;
	affirm(dbuffer->Unlock(buffer1, size1, nullptr, 0));

	affirm(dbuffer->Play(0, 0, DSBPLAY_LOOPING));
}

/*void Audio::play(s16* data, int size) {
	int position = bufferPosition;
	for (int i = 0; i < size; ++i) {
		mixbuffer[position++] = (float)data[i] / (float)32767;
	}
}*/

namespace {
	void copySample(u8* buffer, DWORD& index) {
		/*float value = mixbuffer[bufferPosition++];
		s16 sample = static_cast<s16>(value * 32767);
		s16* hmm = (s16*)&buffer[index];
		*hmm = sample;
		index += 2;*/

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
#endif
