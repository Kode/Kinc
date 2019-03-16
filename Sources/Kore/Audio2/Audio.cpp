#include "pch.h"

#include "Audio.h"

#include <C/Kore/Audio2/Audio.h>

#include <stdio.h>

using namespace Kore;

void (*Audio2::audioCallback)(int samples) = nullptr;
Audio2::Buffer Audio2::buffer;
int Audio2::samplesPerSecond = 44100;

namespace {
	void audio(Kore_A2_Buffer* buffer, int samples) {
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
	Kore_A2_Init();
	Kore_A2_SetCallback(audio);
}

void Audio2::update() {
	Kore_A2_Update();
}

void Audio2::shutdown() {
	Kore_A2_Shutdown();
}
