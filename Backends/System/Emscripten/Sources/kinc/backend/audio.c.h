#include <AL/al.h>
#include <AL/alc.h>
#include <assert.h>
#include <emscripten.h>
#include <kinc/audio2/audio.h>
#include <stdio.h>
#include <stdlib.h>

static kinc_a2_buffer_t a2_buffer;

static ALCdevice *device = NULL;
static ALCcontext *context = NULL;
static unsigned int channels = 0;
static unsigned int bits = 0;
static ALenum format = 0;
static ALuint source = 0;

static bool audioRunning = false;
#define BUFSIZE 4096
static short buf[BUFSIZE];
#define NUM_BUFFERS 3

static uint32_t samples_per_second = 44100;

static void copySample(void *buffer) {
	float left_value = *(float *)&a2_buffer.channels[0][a2_buffer.read_location];
	float right_value = *(float *)&a2_buffer.channels[1][a2_buffer.read_location];
	a2_buffer.read_location += 1;
	if (a2_buffer.read_location >= a2_buffer.data_size) {
		a2_buffer.read_location = 0;
	}
	((int16_t *)buffer)[0] = (int16_t)(left_value * 32767);
	((int16_t *)buffer)[1] = (int16_t)(right_value * 32767);
}

static void streamBuffer(ALuint buffer) {
	if (kinc_a2_internal_callback(&a2_buffer, BUFSIZE / 2)) {
		for (int i = 0; i < BUFSIZE; i += 2) {
			copySample(&buf[i]);
		}
	}

	alBufferData(buffer, format, buf, BUFSIZE * 2, samples_per_second);
}

static void iter() {
	if (!audioRunning) {
		return;
	}

	ALint processed;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

	if (processed <= 0) {
		return;
	}
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

static bool a2_initialized = false;

void kinc_a2_init() {
	if (a2_initialized) {
		return;
	}

	kinc_a2_internal_init();
	a2_initialized = true;

	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.channel_count = 2;
	a2_buffer.channels[0] = (float*)malloc(a2_buffer.data_size * sizeof(float));
	a2_buffer.channels[1] = (float*)malloc(a2_buffer.data_size * sizeof(float));

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

uint32_t kinc_a2_samples_per_second(void) {
	return samples_per_second;
}
