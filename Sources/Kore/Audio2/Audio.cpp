#include "pch.h"

#include "Audio.h"

#include <kinc/audio2/audio.h>

#include <stdio.h>

using namespace Kore;

void (*Audio2::audioCallback)(int samples) = nullptr;
Audio2::Buffer Audio2::buffer;
int Audio2::samplesPerSecond = 44100;

namespace {
	void audio(kinc_a2_buffer_t *buffer, int samples) {
		if (Audio2::audioCallback != nullptr) {
			Audio2::audioCallback(samples);
		}
		else {
			for (int i = 0; i < samples; ++i) {
				*(float *)&Audio2::buffer.data[Audio2::buffer.writeLocation] = 0.0f;
				Audio2::buffer.writeLocation += 4;
				if (Audio2::buffer.writeLocation >= Audio2::buffer.dataSize) {
					Audio2::buffer.writeLocation = 0;
				}
			}
		}
		for (int i = 0; i < samples; ++i) {
			float sample = *(float *)&Audio2::buffer.data[Audio2::buffer.readLocation];
			Audio2::buffer.readLocation += 4;
			if (Audio2::buffer.readLocation >= Audio2::buffer.dataSize) {
				Audio2::buffer.readLocation = 0;
			}

			*(float *)&buffer->data[buffer->write_location] = sample;
			buffer->write_location += 4;
			if (buffer->write_location >= buffer->data_size) {
				buffer->write_location = 0;
			}
		}
	}

#ifdef KORE_IOS
	void sample_rate_changed() {
		Audio2::samplesPerSecond = kinc_a2_samples_per_second;
	}
#endif
}

void Audio2::init() {
	kinc_a2_init();
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];
	Audio2::samplesPerSecond = kinc_a2_samples_per_second;
	kinc_a2_set_callback(audio);
#ifdef KORE_IOS
	kinc_a2_set_sample_rate_callback(sample_rate_changed);
#endif
}

void Audio2::update() {
	kinc_a2_update();
}

void Audio2::shutdown() {
	kinc_a2_shutdown();
}
