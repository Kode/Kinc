#include <Kore/Audio2/Audio.h>
#include <Kore/Audio3/Audio.h>

using namespace Kore;

namespace {
	const int channelCount = 64;
	Audio3::Channel channels[channelCount];

	void callback(int samples) {
		for (int i = 0; i < channelCount; ++i) {
			channels[i].callback(samples);
		}
		for (int i = 0; i < samples; ++i) {
			float value = 0;
			for (int i = 0; i < channelCount; ++i) {
				value += *(float *)&channels[i].buffer.data[Audio2::buffer.readLocation];
				channels[i].buffer.readLocation += 4;
				if (channels[i].buffer.readLocation >= channels[i].buffer.dataSize)
					channels[i].buffer.readLocation = 0;
			}

			*(float *)&Audio2::buffer.data[Audio2::buffer.writeLocation] = value;
			Audio2::buffer.writeLocation += 4;
			if (Audio2::buffer.writeLocation >= Audio2::buffer.dataSize)
				Audio2::buffer.writeLocation = 0;
		}
	}
}

void Audio3::init() {
	for (int i = 0; i < channelCount; ++i) {
		channels[i].active = false;
		channels[i].buffer.readLocation = 0;
		channels[i].buffer.writeLocation = 0;
		channels[i].buffer.dataSize = 128 * 1024;
		channels[i].buffer.data = new u8[channels[i].buffer.dataSize];
	}
	Audio2::init();
	Audio2::audioCallback = callback;
}

void Audio3::update() {
	Audio2::update();
}

void Audio3::shutdown() {
	Audio2::shutdown();
}

Audio3::Channel *Audio3::createChannel(vec3 origin, AudioCallback callback) {
	for (int i = 0; i < channelCount; ++i) {
		if (!channels[i].active) {
			return &channels[i];
		}
	}
	return nullptr;
}

void Audio3::destroyChannel(Channel *channel) {
	channel->active = false;
}
