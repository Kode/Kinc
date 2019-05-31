#include "pch.h"

#include "Audio.h"

#include <kinc/audio2/audio.h>

#include <stdio.h>

using namespace Kore;

void (*Audio2::audioCallback)(int samples) = nullptr;
Audio2::Buffer Audio2::buffer;
int Audio2::samplesPerSecond = 44100;

namespace {
	void audio(Kinc_A2_Buffer *buffer, int samples) {
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
	Kinc_A2_Init();
	Audio2::samplesPerSecond = Kinc_A2_SamplesPerSecond;
	Kinc_A2_SetCallback(audio);
}

void Audio2::update() {
	Kinc_A2_Update();
}

void Audio2::shutdown() {
	Kinc_A2_Shutdown();
}
