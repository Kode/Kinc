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
		int position;
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

	void mix(int samples) {
		for (int i = 0; i < samples; ++i) {
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
					value += *(s16*)&channels[i].sound->data[channels[i].position] / 32767.0f * channels[i].sound->volume();
					value = max(min(value, 1.0f), -1.0f);
					channels[i].position += 2;
					if (channels[i].position >= channels[i].sound->size) channels[i].sound = nullptr;
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

void Mixer::play(Sound* sound) {
	mutex.Lock();
	for (int i = 0; i < channelCount; ++i) {
		if (channels[i].sound == nullptr) {
			channels[i].sound = sound;
			channels[i].position = 0;
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
