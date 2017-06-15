#include "pch.h"

#include <CoreAudio/AudioHardware.h>
#include <CoreServices/CoreServices.h>

#include <Kore/Audio2/Audio.h>
#include <Kore/Log.h>

#include <Kore/VideoSoundStream.h>

#include <stdio.h>

using namespace Kore;

namespace {
	VideoSoundStream* video = nullptr;
}

void macPlayVideoSoundStream(VideoSoundStream* video) {
	::video = video;
}

void macStopVideoSoundStream() {
	video = nullptr;
}

namespace {
	// const int samplesPerSecond = 44100;

	void affirm(OSStatus err) {
		if (err != kAudioHardwareNoError) {
			log(Error, "Error: %i\n", err);
		}
	}

	bool initialized;
	bool soundPlaying;
	AudioDeviceID device;
	UInt32 deviceBufferSize;
	AudioStreamBasicDescription deviceFormat;

	AudioDeviceIOProcID theIOProcID = nullptr;

	void copySample(void* buffer) {
		float value = *(float*)&Audio2::buffer.data[Audio2::buffer.readLocation];
		Audio2::buffer.readLocation += 4;
		if (Audio2::buffer.readLocation >= Audio2::buffer.dataSize) Audio2::buffer.readLocation = 0;
		*(float*)buffer = value;
	}

	OSStatus appIOProc(AudioDeviceID inDevice, const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime,
	                   AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime, void* userdata) {
		int numSamples = deviceBufferSize / deviceFormat.mBytesPerFrame;
		Audio2::audioCallback(numSamples * 2);
		float* out = (float*)outOutputData->mBuffers[0].mData;
		for (int i = 0; i < numSamples; ++i) {
			copySample(out++); // left
			copySample(out++); // right
		}
		return kAudioHardwareNoError;
	}
}

void Audio2::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];

	device = kAudioDeviceUnknown;

	initialized = false;

	UInt32 size = sizeof(AudioDeviceID);
	AudioObjectPropertyAddress address = {kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};
	affirm(AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, &device));

	size = sizeof(UInt32);
	address.mSelector = kAudioDevicePropertyBufferSize;
	address.mScope = kAudioDevicePropertyScopeOutput;
	affirm(AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &deviceBufferSize));

	log(Info, "deviceBufferSize = %i\n", deviceBufferSize);

	size = sizeof(AudioStreamBasicDescription);
	address.mSelector = kAudioDevicePropertyStreamFormat;
	address.mScope = kAudioDevicePropertyScopeOutput;

	affirm(AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &deviceFormat));

	if (deviceFormat.mFormatID != kAudioFormatLinearPCM) {
		log(Error, "mFormatID !=  kAudioFormatLinearPCM\n");
		return;
	}

	if (!(deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsFloat)) {
		log(Error, "Only works with float format.\n");
		return;
	}

	initialized = true;

	log(Info, "mSampleRate = %g\n", deviceFormat.mSampleRate);
	log(Info, "mFormatFlags = %08X\n", (unsigned int)deviceFormat.mFormatFlags);
	log(Info, "mBytesPerPacket = %d\n", (unsigned int)deviceFormat.mBytesPerPacket);
	log(Info, "mFramesPerPacket = %d\n", (unsigned int)deviceFormat.mFramesPerPacket);
	log(Info, "mChannelsPerFrame = %d\n", (unsigned int)deviceFormat.mChannelsPerFrame);
	log(Info, "mBytesPerFrame = %d\n", (unsigned int)deviceFormat.mBytesPerFrame);
	log(Info, "mBitsPerChannel = %d\n", (unsigned int)deviceFormat.mBitsPerChannel);

	if (soundPlaying) return;

	affirm(AudioDeviceCreateIOProcID(device, appIOProc, nullptr, &theIOProcID));
	affirm(AudioDeviceStart(device, theIOProcID));

	soundPlaying = true;
}

void Audio2::update() {}

void Audio2::shutdown() {
	if (!initialized) return;
	if (!soundPlaying) return;

	affirm(AudioDeviceStop(device, theIOProcID));
	affirm(AudioDeviceDestroyIOProcID(device, theIOProcID));

	soundPlaying = false;
}
