#include "pch.h"
#include "Audio.h"
#include <CoreServices/CoreServices.h>
#include <CoreAudio/AudioHardware.h>
#include <stdio.h>

using namespace Kore;

namespace {
	const int samplesPerSecond = 44100;
	
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
	
	void copySample(void* buffer) {
		float value = *(float*)&Audio::buffer.data[Audio::buffer.readLocation];
		Audio::buffer.readLocation += 4;
		if (Audio::buffer.readLocation >= Audio::buffer.dataSize) Audio::buffer.readLocation = 0;
		*(float*)buffer = value;
	}
	
	OSStatus appIOProc(AudioDeviceID inDevice, const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime, AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime, void* userdata) {
		int numSamples = deviceBufferSize / deviceFormat.mBytesPerFrame;
		Audio::audioCallback(numSamples * 2);
		float *out = (float*)outOutputData->mBuffers[0].mData;
		for (int i = 0; i < numSamples; ++i) {
			copySample(out++); //left
			copySample(out++); //right
		}
		return kAudioHardwareNoError;
	}
}

void Audio::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];
	
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
