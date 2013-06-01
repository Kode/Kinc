#include "pch.h"
#include <Kore/Audio/Audio.h>

using namespace Kore;

void Audio::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];


}

void Audio::update() {

}

void Audio::shutdown() {

}
