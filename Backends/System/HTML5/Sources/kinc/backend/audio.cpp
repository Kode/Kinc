#include "pch.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <kinc/audio2/audio.h>
#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

using namespace Kore;

namespace {
	void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples) = nullptr;
    kinc_a2_buffer_t a2_buffer;

	ALCdevice* device = NULL;
	ALCcontext* context = NULL;
	unsigned int channels = 0;
	unsigned int bits = 0;
	ALenum format = 0;
	ALuint source = 0;

	bool audioRunning = false;
	const int bufsize = 4096;
	short buf[bufsize];
#define NUM_BUFFERS 3

	void copySample(void* buffer) {
		float value = *(float*)&a2_buffer.data[a2_buffer.read_location];
		a2_buffer.read_location += 4;
		if (a2_buffer.read_location >= a2_buffer.data_size) {
			a2_buffer.read_location = 0;
		}
		*(s16*)buffer = static_cast<s16>(value * 32767);
	}

	void streamBuffer(ALuint buffer) {
		if (a2_callback != nullptr) {
			a2_callback(&a2_buffer, bufsize);
			for (int i = 0; i < bufsize; ++i) {
				copySample(&buf[i]);
			}
		}

		alBufferData(buffer, format, buf, bufsize * 2, 44100);
	}

	void iter() {
		if (!audioRunning) return;

		ALint processed;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

		if (processed <= 0) return;
		while (processed--) {
			ALuint buffer;
			alSourceUnqueueBuffers(source, 1, &buffer);
			streamBuffer(buffer);
			alSourceQueueBuffers(source, 1, &buffer);
		}

		ALint playing;
		alGetSourcei(source, AL_SOURCE_STATE, &playing);
		if (playing != AL_PLAYING) {
			alSourcePlay(source);
		}
	}
}

void kinc_a2_init() {
	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = new u8[a2_buffer.data_size];

	audioRunning = true;

	device = alcOpenDevice(NULL);
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	format = AL_FORMAT_STEREO16;

	ALuint buffers[NUM_BUFFERS];
	alGenBuffers(NUM_BUFFERS, buffers);
	alGenSources(1, &source);

	streamBuffer(buffers[0]);
	streamBuffer(buffers[1]);
	streamBuffer(buffers[2]);

	alSourceQueueBuffers(source, NUM_BUFFERS, buffers);

	alSourcePlay(source);
}

void kinc_a2_update() {
	iter();
}

void kinc_a2_shutdown() {
	audioRunning = false;
}

void kinc_a2_set_callback(void(*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples)) {
	a2_callback = kinc_a2_audio_callback;
}
