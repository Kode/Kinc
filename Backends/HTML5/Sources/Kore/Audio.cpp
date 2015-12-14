#include "pch.h"
#include <Kore/Audio/Audio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten.h>
#include <AL/al.h>
#include <AL/alc.h>

using namespace Kore;

namespace {
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
		float value = *(float*)&Audio::buffer.data[Audio::buffer.readLocation];
		Audio::buffer.readLocation += 4;
		if (Audio::buffer.readLocation >= Audio::buffer.dataSize) Audio::buffer.readLocation = 0;
		*(s16*)buffer = static_cast<s16>(value * 32767);
	}

	void streamBuffer(ALuint buffer) {
		if (Kore::Audio::audioCallback != nullptr) {
			Kore::Audio::audioCallback(bufsize);
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
		if (playing != AL_PLAYING) alSourcePlay(source);
	}
}

void Audio::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];

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

void Audio::update() {
	iter();
}

void Audio::shutdown() {
	audioRunning = false;
}
