#include "pch.h"

#include "Audio.h"

#include <stdint.h>

#include <Kore/Threads/Mutex.h>
#include <Kore/Math/Core.h>

#include <C/Kore/Audio2/Audio.h>

/*#include <Kore/Audio2/Audio.h>
#if 0
#include <xmmintrin.h>
#endif
#include <Kore/VideoSoundStream.h>*/

static Kore::Mutex mutex;

#define CHANNEL_COUNT 16
static Kore_A1_Channel channels[CHANNEL_COUNT];
static Kore_A1_StreamChannel streams[CHANNEL_COUNT];
static Kore_A1_VideoChannel videos[CHANNEL_COUNT];

float sampleLinear(int16_t *data, float position) {
	int pos1 = (int)position;
	int pos2 = (int)(position + 1);
	float sample1 = data[pos1] / 32767.0f;
	float sample2 = data[pos2] / 32767.0f;
	float a = position - pos1;
	return sample1 * (1 - a) + sample2 * a;
}

/*float sampleHermite4pt3oX(s16* data, float position) {
	float s0 = data[(int)(position - 1)] / 32767.0f;
	float s1 = data[(int)(position + 0)] / 32767.0f;
	float s2 = data[(int)(position + 1)] / 32767.0f;
	float s3 = data[(int)(position + 2)] / 32767.0f;

	float x = position - (int)(position);

	// 4-point, 3rd-order Hermite (x-form)
	float c0 = s1;
	float c1 = 0.5f * (s2 - s0);
	float c2 = s0 - 2.5f * s1 + 2 * s2 - 0.5f * s3;
	float c3 = 0.5f * (s3 - s0) + 1.5f * (s1 - s2);
	return ((c3 * x + c2) * x + c1) * x + c0;
}*/

void Kore_Internal_A1_Mix(Kore_A2_Buffer *buffer, int samples) {
	for (int i = 0; i < samples; ++i) {
		bool left = (i % 2) == 0;
		float value = 0;
#if 0
		__m128 sseSamples[4];
		for (int i = 0; i < channelCount; i += 4) {
			s16 data[4];
			for (int i2 = 0; i2 < 4; ++i2) {
				if (channels[i + i2].sound != nullptr) {
					data[i2] = *(s16*)&channels[i + i2].sound->data[channels[i + i2].position];
					channels[i + i2].position += 2;
					if (channels[i + i2].position >= channels[i + i2].sound->size) channels[i + i2].sound = nullptr;
				}
				else {
					data[i2] = 0;
				}
			}
			sseSamples[i / 4] = _mm_set_ps(data[3] / 32767.0f, data[2] / 32767.0f, data[1] / 32767.0f, data[0] / 32767.0f);
		}
		__m128 a = _mm_add_ps(sseSamples[0], sseSamples[1]);
		__m128 b = _mm_add_ps(sseSamples[2], sseSamples[3]);
		__m128 c = _mm_add_ps(a, b);
		value = c.m128_f32[0] + c.m128_f32[1] + c.m128_f32[2] + c.m128_f32[3];
		value = max(min(value, 1.0f), -1.0f);
#else
		mutex.lock();
		for (int i = 0; i < CHANNEL_COUNT; ++i) {
			if (channels[i].sound != NULL) {
				// value += *(s16*)&channels[i].sound->data[(int)channels[i].position] / 32767.0f * channels[i].sound->volume();
				if (left)
					value += sampleLinear(channels[i].sound->left, channels[i].position) * channels[i].volume * channels[i].volume;
				else
					value += sampleLinear(channels[i].sound->right, channels[i].position) * channels[i].volume * channels[i].volume;
				value = Kore::max(Kore::min(value, 1.0f), -1.0f);
				//**if (!left) channels[i].position += channels[i].pitch / channels[i].sound->sampleRatePos;
				// channels[i].position += 2;
				if (channels[i].position + 1 >= channels[i].sound->size) {
					if (channels[i].loop) {
						channels[i].position = 0;
					}
					else {
						channels[i].sound = NULL;
					}
				}
			}
		}
		//**
		/*for (int i = 0; i < CHANNEL_COUNT; ++i) {
			if (streams[i].stream != NULL) {
				value += streams[i].stream->nextSample() * streams[i].stream->volume();
				value = Kore::max(Kore::min(value, 1.0f), -1.0f);
				if (streams[i].stream->ended()) streams[i].stream = NULL;
			}
		}*/
		/*for (int i = 0; i < CHANNEL_COUNT; ++i) {
			if (videos[i].stream != NULL) {
				value += videos[i].stream->nextSample();
				value = Kore::max(Kore::min(value, 1.0f), -1.0f);
				if (videos[i].stream->ended()) videos[i].stream = NULL;
			}
		}*/
		mutex.unlock();
#endif
		*(float*)&buffer->data[buffer->write_location] = value;
		buffer->write_location += 4;
		if (buffer->write_location >= buffer->data_size) buffer->write_location = 0;
	}
}

void Kore_A1_Init() {
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		channels[i].sound = NULL;
		channels[i].position = 0;
	}
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		streams[i].stream = NULL;
		streams[i].position = 0;
	}
	mutex.create();
	Kore_A2_SetCallback(Kore_Internal_A1_Mix);
}

Kore_A1_Channel *Kore_A1_PlaySound(Kore_A1_Sound *sound, bool loop, float pitch, bool unique) {
	Kore_A1_Channel *channel = NULL;
	mutex.lock();
	bool found = false;
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (channels[i].sound == sound) {
			found = true;
			break;
		}
	}
	if (!found || !unique) {
		for (int i = 0; i < CHANNEL_COUNT; ++i) {
			if (channels[i].sound == NULL) {
				channels[i].sound = sound;
				channels[i].position = 0;
				channels[i].loop = loop;
				channels[i].pitch = pitch;
				channels[i].volume = 1.0f;
				channel = &channels[i];
				break;
			}
		}
	}
	mutex.unlock();
	return channel;
}

void Kore_A1_StopSound(Kore_A1_Sound *sound) {
	mutex.lock();
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (channels[i].sound == sound) {
			channels[i].sound = NULL;
			channels[i].position = 0;
			break;
		}
	}
	mutex.unlock();
}

void Kore_A1_PlaySoundStream(Kore_A1_SoundStream *stream) {
	mutex.lock();

	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (streams[i].stream == stream) {
			streams[i].stream = NULL;
			streams[i].position = 0;
			break;
		}
	}

	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (streams[i].stream == NULL) {
			streams[i].stream = stream;
			streams[i].position = 0;
			break;
		}
	}

	mutex.unlock();
}

void Kore_A1_StopSoundStream(Kore_A1_SoundStream *stream) {
	mutex.lock();
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (streams[i].stream == stream) {
			streams[i].stream = NULL;
			streams[i].position = 0;
			break;
		}
	}
	mutex.unlock();
}

void Kore_A1_PlayVideoSoundStream(struct Kore_A1_VideoSoundStream *stream) {
	mutex.lock();
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (videos[i].stream == NULL) {
			videos[i].stream = stream;
			videos[i].position = 0;
			break;
		}
	}
	mutex.unlock();
}

void Kore_A1_StopVideoSoundStream(struct Kore_A1_VideoSoundStream *stream) {
	mutex.lock();
	for (int i = 0; i < CHANNEL_COUNT; ++i) {
		if (videos[i].stream == stream) {
			videos[i].stream = NULL;
			videos[i].position = 0;
			break;
		}
	}
	mutex.unlock();
}
