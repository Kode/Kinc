#include "stdafx.h"
#include <Kt/Sound/Sound.h>
#include <Kt/Sound/Music.h>

using namespace Kt;

Sound::SoundHandle::SoundHandle(Kt::Text filename, bool loops) {

}
			
Sound::SoundHandle::~SoundHandle() {

}

void Sound::SoundHandle::play() {

}

void Sound::SoundHandle::setVolume(float volume) {

}

int Kt::Sound::SoundHandle::position() {
	return 0;
}

int Kt::Sound::SoundHandle::length() {
	return 0;
}

void Sound::init() {

}

void Sound::shutdown() {

}

void Music::play(Kt::Text filename) {

}

void Music::stop() {

}
