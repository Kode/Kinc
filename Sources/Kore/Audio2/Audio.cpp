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
				Audio2::buffer.data[Audio2::buffer.writeLocation++] = 0;
				if (Audio2::buffer.writeLocation >= Audio2::buffer.dataSize) {
					Audio2::buffer.writeLocation = 0;
				}
			}
		}
		for (int i = 0; i < samples; ++i) {
			u8 sample = Audio2::buffer.data[Audio2::buffer.readLocation++];
			if (Audio2::buffer.readLocation >= Audio2::buffer.dataSize) {
				Audio2::buffer.readLocation = 0;
			}
			
			buffer->data[buffer->write_location] = sample;
			if (buffer->write_location >= buffer->data_size) {
				buffer->write_location = 0;
			}
		}
	}
}

void Audio2::init() {
	kinc_a2_init();
	Audio2::samplesPerSecond = kinc_a2_samples_per_second;
	kinc_a2_set_callback(audio);
}

void Audio2::update() {
	kinc_a2_update();
}

void Audio2::shutdown() {
	kinc_a2_shutdown();
}
