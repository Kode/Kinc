#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>

#include <kinc/audio2/audio.h>
#include <kinc/backend/video.h>
#include <kinc/math/core.h>

#include <stdio.h>

#define kOutputBus 0

static kinc_internal_video_sound_stream_t *video = NULL;

void iosPlayVideoSoundStream(kinc_internal_video_sound_stream_t *v) {
	video = v;
}

void iosStopVideoSoundStream(void) {
	video = NULL;
}

static void affirm(OSStatus err) {
	if (err) {
		fprintf(stderr, "Error: %i\n", (int)err);
	}
}

static bool initialized;
static bool soundPlaying;
static AudioStreamBasicDescription deviceFormat;
static AudioComponentInstance audioUnit;
static bool isFloat = false;
static bool isInterleaved = true;

static void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples) = NULL;
static void (*a2_sample_rate_callback)(void) = NULL;
static kinc_a2_buffer_t a2_buffer;

static void copySample(void *buffer) {
	float value = *(float *)&a2_buffer.data[a2_buffer.read_location];
	a2_buffer.read_location += 4;
	if (a2_buffer.read_location >= a2_buffer.data_size)
		a2_buffer.read_location = 0;

	if (video != NULL) {
		value += kinc_internal_video_sound_stream_next_sample(video);
		value = kinc_max(kinc_min(value, 1.0f), -1.0f);
		if (kinc_internal_video_sound_stream_ended(video)) {
			video = NULL;
		}
	}

	if (isFloat)
		*(float *)buffer = value;
	else
		*(int16_t *)buffer = (int16_t)(value * 32767);
}

static OSStatus renderInput(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
                            UInt32 inNumberFrames, AudioBufferList *outOutputData) {
	a2_callback(&a2_buffer, inNumberFrames * 2);
	if (isInterleaved) {
		if (isFloat) {
			float *out = (float *)outOutputData->mBuffers[0].mData;
			for (int i = 0; i < inNumberFrames; ++i) {
				copySample(out++); // left
				copySample(out++); // right
			}
		}
		else {
			int16_t *out = (int16_t *)outOutputData->mBuffers[0].mData;
			for (int i = 0; i < inNumberFrames; ++i) {
				copySample(out++); // left
				copySample(out++); // right
			}
		}
	}
	else {
		if (isFloat) {
			float *out1 = (float *)outOutputData->mBuffers[0].mData;
			float *out2 = (float *)outOutputData->mBuffers[1].mData;
			for (int i = 0; i < inNumberFrames; ++i) {
				copySample(out1++); // left
				copySample(out2++); // right
			}
		}
		else {
			int16_t *out1 = (int16_t *)outOutputData->mBuffers[0].mData;
			int16_t *out2 = (int16_t *)outOutputData->mBuffers[1].mData;
			for (int i = 0; i < inNumberFrames; ++i) {
				copySample(out1++); // left
				copySample(out2++); // right
			}
		}
	}
	return noErr;
}

static void sampleRateListener(void *inRefCon, AudioUnit inUnit, AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement) {
	Float64 sampleRate;
	UInt32 size = sizeof(sampleRate);
	affirm(AudioUnitGetProperty(inUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, 0, &sampleRate, &size));

	kinc_a2_samples_per_second = (int)sampleRate;
	if (a2_sample_rate_callback != NULL) {
		a2_sample_rate_callback();
	}
}

static bool initialized = false;

void kinc_a2_init() {
	if (initialized) {
		return;
	}

	initialized = true;

	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = (uint8_t *)malloc(a2_buffer.data_size);

	initialized = false;

	AudioComponentDescription desc;
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;

	AudioComponent comp = AudioComponentFindNext(NULL, &desc);

	// Get audio units
	affirm(AudioComponentInstanceNew(comp, &audioUnit));
	UInt32 flag = 1;
	affirm(AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, kOutputBus, &flag, sizeof(UInt32)));

	if (soundPlaying)
		return;

	affirm(AudioOutputUnitStart(audioUnit));

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

	kinc_a2_samples_per_second = deviceFormat.mSampleRate;
	a2_buffer.format.samples_per_second = kinc_a2_samples_per_second;
	a2_buffer.format.bits_per_sample = 32;
	a2_buffer.format.channels = 2;

	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = renderInput;
	callbackStruct.inputProcRefCon = NULL;
	affirm(AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, kOutputBus, &callbackStruct, sizeof(callbackStruct)));

	soundPlaying = true;
}

void kinc_a2_update() {}

void kinc_a2_shutdown() {
	if (!initialized)
		return;
	if (!soundPlaying)
		return;

	affirm(AudioOutputUnitStop(audioUnit));

	soundPlaying = false;
}

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples)) {
	a2_callback = kinc_a2_audio_callback;
}

void kinc_a2_set_sample_rate_callback(void (*kinc_a2_sample_rate_callback)(void)) {
	a2_sample_rate_callback = kinc_a2_sample_rate_callback;
}
