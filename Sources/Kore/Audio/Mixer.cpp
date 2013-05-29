#include "pch.h"
#include "Mixer.h"
#include "Audio.h"
#include <Kore/Math/Core.h>

using namespace Kore;

namespace {
	struct Channel {
		Sound* sound;
		int position;
	};

	const int channelCount = 16;
	Channel channels[channelCount];

	void mix(int samples) {
		for (int i = 0; i < samples; ++i) {
			float value = 0;
			for (int i = 0; i < channelCount; ++i) {
				if (channels[i].sound != nullptr) {
					value += *(s16*)&channels[i].sound->data[channels[i].position] / 32767.0f;
					value = max(min(value, 1.0f), -1.0f);
					channels[i].position += 2;
					if (channels[i].position >= channels[i].sound->size) channels[i].sound = nullptr;
				}
			}
			*(float*)&Audio::buffer.data[Audio::buffer.writeLocation] = value;
			Audio::buffer.writeLocation += 4;
			if (Audio::buffer.writeLocation >= Audio::buffer.dataSize) Audio::buffer.writeLocation = 0;
		}
	}
}

void Mixer::init() {
	Audio::audioCallback = mix;
}

void Mixer::play(Sound* sound) {
	for (int i = 0; i < channelCount; ++i) {
		if (channels[i].sound == nullptr) {
			channels[i].sound = sound;
			channels[i].position = 0;
			break;
		}
	}
}
