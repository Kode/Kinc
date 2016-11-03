#include "pch.h"

#include "Mixer.h"
#include "Audio.h"

#include <Kore/Math/Core.h>
#include <Kore/Threads/Mutex.h>
#if 0
#include <xmmintrin.h>
#endif
#include <Kore/VideoSoundStream.h>

using namespace Kore;

namespace {
	Mutex mutex;

	struct Channel {
		Sound* sound;
		float position;
		float pitch;
	};

	struct StreamChannel {
		SoundStream* stream;
		int position;
	};

	struct VideoChannel {
		VideoSoundStream* stream;
		int position;
	};

	const int channelCount = 16;
	Channel channels[channelCount];
	StreamChannel streams[channelCount];
	VideoChannel videos[channelCount];

	float sampleLinear(s16* data, float position) {
		int pos1 = (int)position;
		int pos2 = (int)(position + 1);
		float sample1 = data[pos1] / 32767.0f;
		float sample2 = data[pos2] / 32767.0f;
		float a = position - pos1;
		return sample1 * (1 - a) + sample2 * a;
	}

	float sampleHermite4pt3oX(s16* data, float position) {
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
	}

	void mix(int samples) {
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
			mutex.Lock();
			for (int i = 0; i < channelCount; ++i) {
				if (channels[i].sound != nullptr) {
					// value += *(s16*)&channels[i].sound->data[(int)channels[i].position] / 32767.0f * channels[i].sound->volume();
					if (left)
						value += sampleLinear(channels[i].sound->left, channels[i].position) * channels[i].sound->volume();
					else
						value += sampleLinear(channels[i].sound->right, channels[i].position) * channels[i].sound->volume();
					value = max(min(value, 1.0f), -1.0f);
					if (!left) channels[i].position += channels[i].pitch / channels[i].sound->sampleRatePos;
					// channels[i].position += 2;
					if (channels[i].position >= channels[i].sound->size / 4) channels[i].sound = nullptr;
				}
			}
			for (int i = 0; i < channelCount; ++i) {
				if (streams[i].stream != nullptr) {
					value += streams[i].stream->nextSample() * streams[i].stream->volume();
					value = max(min(value, 1.0f), -1.0f);
					if (streams[i].stream->ended()) streams[i].stream = nullptr;
				}
			}
			for (int i = 0; i < channelCount; ++i) {
				if (videos[i].stream != nullptr) {
					value += videos[i].stream->nextSample();
					value = max(min(value, 1.0f), -1.0f);
					if (videos[i].stream->ended()) videos[i].stream = nullptr;
				}
			}
			mutex.Unlock();
#endif
			*(float*)&Audio::buffer.data[Audio::buffer.writeLocation] = value;
			Audio::buffer.writeLocation += 4;
			if (Audio::buffer.writeLocation >= Audio::buffer.dataSize) Audio::buffer.writeLocation = 0;
		}
	}
}

void Mixer::init() {
	for (int i = 0; i < channelCount; ++i) {
		channels[i].sound = nullptr;
		channels[i].position = 0;
	}
	for (int i = 0; i < channelCount; ++i) {
		streams[i].stream = nullptr;
		streams[i].position = 0;
	}
	mutex.Create();
	Audio::audioCallback = mix;
}

void Mixer::play(Sound* sound, float pitch) {
	mutex.Lock();
	for (int i = 0; i < channelCount; ++i) {
		if (channels[i].sound == nullptr) {
			channels[i].sound = sound;
			channels[i].position = 0;
			channels[i].pitch = pitch;
			break;
		}
	}
	mutex.Unlock();
}

void Mixer::stop(Sound* sound) {
	mutex.Lock();
	for (int i = 0; i < channelCount; ++i) {
		if (channels[i].sound == sound) {
			channels[i].sound = nullptr;
			channels[i].position = 0;
			break;
		}
	}
	mutex.Unlock();
}

void Mixer::play(SoundStream* stream) {
	mutex.Lock();

	for (int i = 0; i < channelCount; ++i) {
		if (streams[i].stream == stream) {
			streams[i].stream = nullptr;
			streams[i].position = 0;
			break;
		}
	}

	for (int i = 0; i < channelCount; ++i) {
		if (streams[i].stream == nullptr) {
			streams[i].stream = stream;
			streams[i].position = 0;
			break;
		}
	}

	mutex.Unlock();
}

void Mixer::stop(SoundStream* stream) {
	mutex.Lock();
	for (int i = 0; i < channelCount; ++i) {
		if (streams[i].stream == stream) {
			streams[i].stream = nullptr;
			streams[i].position = 0;
			break;
		}
	}
	mutex.Unlock();
}

void Mixer::play(VideoSoundStream* stream) {
	mutex.Lock();
	for (int i = 0; i < channelCount; ++i) {
		if (videos[i].stream == nullptr) {
			videos[i].stream = stream;
			videos[i].position = 0;
			break;
		}
	}
	mutex.Unlock();
}

void Mixer::stop(VideoSoundStream* stream) {
	mutex.Lock();
	for (int i = 0; i < channelCount; ++i) {
		if (videos[i].stream == stream) {
			videos[i].stream = nullptr;
			videos[i].position = 0;
			break;
		}
	}
	mutex.Unlock();
}
