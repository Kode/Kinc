#include "pch.h"

#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>

#include <kinc/audio2/audio.h>
#include <kinc/math/core.h>
#include <Kore/VideoSoundStream.h>

#include <stdio.h>

using namespace Kore;

#define kOutputBus 0

namespace {
	VideoSoundStream* video = nullptr;
}

void iosPlayVideoSoundStream(VideoSoundStream* video) {
	::video = video;
}

void iosStopVideoSoundStream() {
	video = nullptr;
}

namespace {
	// const int samplesPerSecond = 44100;

	void affirm(OSStatus err) {
		if (err) {
			fprintf(stderr, "Error: %i\n", (int)err);
		}
	}

	bool initialized;
	bool soundPlaying;
	AudioStreamBasicDescription deviceFormat;
	AudioComponentInstance audioUnit;
	bool isFloat = false;
	bool isInterleaved = true;
	
	void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples) = nullptr;
	void (*a2_sample_rate_callback)() = nullptr;
	kinc_a2_buffer_t a2_buffer;

	void copySample(void* buffer) {
		float value = *(float*)&a2_buffer.data[a2_buffer.read_location];
		a2_buffer.read_location += 4;
		if (a2_buffer.read_location >= a2_buffer.data_size) a2_buffer.read_location = 0;

		if (video != nullptr) {
			value += video->nextSample();
			value = kinc_max(kinc_min(value, 1.0f), -1.0f);
			if (video->ended()) video = nullptr;
		}

		if (isFloat)
			*(float*)buffer = value;
		else
			*(s16*)buffer = static_cast<s16>(value * 32767);
	}

	OSStatus renderInput(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber,
	                     UInt32 inNumberFrames, AudioBufferList* outOutputData) {
		a2_callback(&a2_buffer, inNumberFrames * 2);
		if (isInterleaved) {
			if (isFloat) {
				float* out = (float*)outOutputData->mBuffers[0].mData;
				for (int i = 0; i < inNumberFrames; ++i) {
					copySample(out++); // left
					copySample(out++); // right
				}
			}
			else {
				s16* out = (s16*)outOutputData->mBuffers[0].mData;
				for (int i = 0; i < inNumberFrames; ++i) {
					copySample(out++); // left
					copySample(out++); // right
				}
			}
		}
		else {
			if (isFloat) {
				float* out1 = (float*)outOutputData->mBuffers[0].mData;
				float* out2 = (float*)outOutputData->mBuffers[1].mData;
				for (int i = 0; i < inNumberFrames; ++i) {
					copySample(out1++); // left
					copySample(out2++); // right
				}
			}
			else {
				s16* out1 = (s16*)outOutputData->mBuffers[0].mData;
				s16* out2 = (s16*)outOutputData->mBuffers[1].mData;
				for (int i = 0; i < inNumberFrames; ++i) {
					copySample(out1++); // left
					copySample(out2++); // right
				}
			}
		}
		return noErr;
	}

	void sampleRateListener(void *inRefCon, AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement) {
		Float64 sampleRate;
		UInt32 size = sizeof(sampleRate);
		affirm(AudioUnitGetProperty(inUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, 0, &sampleRate, &size));

		kinc_a2_samples_per_second = static_cast<int>(sampleRate);
		if (a2_sample_rate_callback != nullptr) {
				a2_sample_rate_callback();
		}
	}
}

void kinc_a2_init() {
	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = new u8[a2_buffer.data_size];

	initialized = false;

	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;

	AudioComponent comp = AudioComponentFindNext(nullptr, &desc);

	// Get audio units
	affirm(AudioComponentInstanceNew(comp, &audioUnit));
	UInt32 flag = 1;
	affirm(AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, kOutputBus, &flag, sizeof(UInt32)));

	UInt32 size = sizeof(AudioStreamBasicDescription);
	affirm(AudioUnitGetProperty(audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &deviceFormat, &size));

	if (deviceFormat.mFormatID != kAudioFormatLinearPCM) {
		fprintf(stderr, "mFormatID !=  kAudioFormatLinearPCM\n");
		return;
	}

	if (deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsFloat) {
		isFloat = true;
	}

	if (deviceFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved) {
		isInterleaved = false;
	}

	AudioUnitAddPropertyListener(audioUnit, kAudioUnitProperty_StreamFormat, sampleRateListener, nil);

	initialized = true;

	printf("mSampleRate = %g\n", deviceFormat.mSampleRate);
	printf("mFormatFlags = %08X\n", (unsigned int)deviceFormat.mFormatFlags);
	printf("mBytesPerPacket = %d\n", (unsigned int)deviceFormat.mBytesPerPacket);
	printf("mFramesPerPacket = %d\n", (unsigned int)deviceFormat.mFramesPerPacket);
	printf("mChannelsPerFrame = %d\n", (unsigned int)deviceFormat.mChannelsPerFrame);
	printf("mBytesPerFrame = %d\n", (unsigned int)deviceFormat.mBytesPerFrame);
	printf("mBitsPerChannel = %d\n", (unsigned int)deviceFormat.mBitsPerChannel);

	if (soundPlaying) return;

	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = renderInput;
	callbackStruct.inputProcRefCon = nullptr;
	affirm(AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, kOutputBus, &callbackStruct, sizeof(callbackStruct)));

	affirm(AudioOutputUnitStart(audioUnit));

	soundPlaying = true;
}

void kinc_a2_update() {}

void kinc_a2_shutdown() {
	if (!initialized) return;
	if (!soundPlaying) return;

	affirm(AudioOutputUnitStop(audioUnit));

	soundPlaying = false;
}

void kinc_a2_set_callback(void(*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples)) {
	a2_callback = kinc_a2_audio_callback;
}

void kinc_a2_set_sample_rate_callback(void (*kinc_a2_sample_rate_callback)()) {
	a2_sample_rate_callback = kinc_a2_sample_rate_callback;
}
