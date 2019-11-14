#include "pch.h"

#include <CoreAudio/AudioHardware.h>
#include <CoreServices/CoreServices.h>

#include <kinc/audio2/audio.h>
#include <kinc/log.h>

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
			kinc_log(KINC_LOG_LEVEL_ERROR, "Error: %i\n", err);
		}
	}

	bool initialized;
	bool soundPlaying;
	AudioDeviceID device;
	UInt32 deviceBufferSize;
	AudioStreamBasicDescription deviceFormat;

	AudioDeviceIOProcID theIOProcID = nullptr;
	
	void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples) = nullptr;
	kinc_a2_buffer_t a2_buffer;

	void copySample(void* buffer) {
		float value = *(float*)&a2_buffer.data[a2_buffer.read_location];
		a2_buffer.read_location += 4;
		if (a2_buffer.read_location >= a2_buffer.data_size) a2_buffer.read_location = 0;
		*(float*)buffer = value;
	}

	OSStatus appIOProc(AudioDeviceID inDevice, const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime,
	                   AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime, void* userdata) {
		int numSamples = deviceBufferSize / deviceFormat.mBytesPerFrame;
		a2_callback(&a2_buffer, numSamples * 2);
		float* out = (float*)outOutputData->mBuffers[0].mData;
		for (int i = 0; i < numSamples; ++i) {
			copySample(out++); // left
			copySample(out++); // right
		}
		return kAudioHardwareNoError;
	}
}

void kinc_a2_init() {
	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = new u8[a2_buffer.data_size];

	device = kAudioDeviceUnknown;

	initialized = false;

	UInt32 size = sizeof(AudioDeviceID);
	AudioObjectPropertyAddress address = {kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster};
	affirm(AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, &device));

	size = sizeof(UInt32);
	address.mSelector = kAudioDevicePropertyBufferSize;
	address.mScope = kAudioDevicePropertyScopeOutput;
	affirm(AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &deviceBufferSize));

	kinc_log(KINC_LOG_LEVEL_INFO, "deviceBufferSize = %i\n", deviceBufferSize);

	size = sizeof(AudioStreamBasicDescription);
	address.mSelector = kAudioDevicePropertyStreamFormat;
	address.mScope = kAudioDevicePropertyScopeOutput;

	affirm(AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &deviceFormat));

	if (deviceFormat.mFormatID != kAudioFormatLinearPCM) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "mFormatID !=  kAudioFormatLinearPCM\n");
		return;
	}

	if (!(deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsFloat)) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Only works with float format.\n");
		return;
	}

	kinc_a2_samples_per_second = static_cast<int>(deviceFormat.mSampleRate);

	initialized = true;

	kinc_log(KINC_LOG_LEVEL_INFO, "mSampleRate = %g\n", deviceFormat.mSampleRate);
	kinc_log(KINC_LOG_LEVEL_INFO, "mFormatFlags = %08X\n", (unsigned int)deviceFormat.mFormatFlags);
	kinc_log(KINC_LOG_LEVEL_INFO, "mBytesPerPacket = %d\n", (unsigned int)deviceFormat.mBytesPerPacket);
	kinc_log(KINC_LOG_LEVEL_INFO, "mFramesPerPacket = %d\n", (unsigned int)deviceFormat.mFramesPerPacket);
	kinc_log(KINC_LOG_LEVEL_INFO, "mChannelsPerFrame = %d\n", (unsigned int)deviceFormat.mChannelsPerFrame);
	kinc_log(KINC_LOG_LEVEL_INFO, "mBytesPerFrame = %d\n", (unsigned int)deviceFormat.mBytesPerFrame);
	kinc_log(KINC_LOG_LEVEL_INFO, "mBitsPerChannel = %d\n", (unsigned int)deviceFormat.mBitsPerChannel);

	if (soundPlaying) return;

	affirm(AudioDeviceCreateIOProcID(device, appIOProc, nullptr, &theIOProcID));
	affirm(AudioDeviceStart(device, theIOProcID));

	soundPlaying = true;
}

void kinc_a2_update() {}

void kinc_a2_shutdown() {
	if (!initialized) return;
	if (!soundPlaying) return;

	affirm(AudioDeviceStop(device, theIOProcID));
	affirm(AudioDeviceDestroyIOProcID(device, theIOProcID));

	soundPlaying = false;
}

void kinc_a2_set_callback(void(*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples)) {
	a2_callback = kinc_a2_audio_callback;
}
