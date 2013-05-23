#include "pch.h"
#include <Kore/Sound/Sound.h>
#include <Kore/Sound/Music.h>

using namespace Kore;

Sound::SoundHandle::SoundHandle(const char* filename, bool loops) {

}

Sound::SoundHandle::~SoundHandle() {

}

void Sound::SoundHandle::play() {

}

void Sound::SoundHandle::setVolume(float volume) {

}

int Sound::SoundHandle::position() {
    return 0;
}

int Sound::SoundHandle::length() {
    return 0;
}

void Sound::init() {

}

void Sound::shutdown() {

}

void Music::play(const char* filename) {

}

void Music::stop() {

}
