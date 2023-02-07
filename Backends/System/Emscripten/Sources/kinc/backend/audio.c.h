#include <AL/al.h>
#include <AL/alc.h>
#include <assert.h>
#include <emscripten.h>
#include <kinc/audio2/audio.h>
#include <stdio.h>
#include <stdlib.h>

static void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples) = NULL;
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

static void copySample(void *buffer) {
	float value = *(float *)&a2_buffer.data[a2_buffer.read_location];
	a2_buffer.read_location += 4;
	if (a2_buffer.read_location >= a2_buffer.data_size) {
		a2_buffer.read_location = 0;
	}
	*(int16_t *)buffer = (int16_t)(value * 32767);
}

static void streamBuffer(ALuint buffer) {
	if (a2_callback != NULL) {
		a2_callback(&a2_buffer, BUFSIZE);
		for (int i = 0; i < BUFSIZE; ++i) {
			copySample(&buf[i]);
		}
	}

	alBufferData(buffer, format, buf, BUFSIZE * 2, 44100);
}

static void iter() {
	if (!audioRunning)
		return;

	ALint processed;
	alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

	if (processed <= 0)
		return;
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

static bool initialized = false;

void kinc_a2_init() {
	if (initialized) {
		return;
	}

	initialized = true;

	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = malloc(a2_buffer.data_size);

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

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples)) {
	a2_callback = kinc_a2_audio_callback;
}
