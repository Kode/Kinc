#include "pch.h"
#include <Kore/Audio/Audio.h>

using namespace Kore;

void Kore::Audio::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];
}

void Kore::Audio::update() {

}

void Kore::Audio::shutdown() {

}
