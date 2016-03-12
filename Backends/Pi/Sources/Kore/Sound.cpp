#include "pch.h"
#include <Kore/Audio/Audio.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
//#include <alsa/asoundlib.h>
#include <pthread.h>

//apt-get install libasound2-dev

using namespace Kore;


void Audio::init() {
    buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];

    //audioRunning = true;
    //pthread_create(&threadid, nullptr, &doAudio, nullptr);
}

void Audio::update() {

}

void Audio::shutdown() {
    //audioRunning = false;
}
