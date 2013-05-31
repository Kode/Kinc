#include "pch.h"
#include "Audio.h"
#include <stdio.h>

using namespace Kore;

void (*Audio::audioCallback)(int samples);
Audio::Buffer Audio::buffer;
