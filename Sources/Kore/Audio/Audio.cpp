#include "pch.h"
#include "Audio.h"
#include <stdio.h>

using namespace Kore;

void (*Audio::audioCallback)(int samples) = nullptr;
Audio::Buffer Audio::buffer;
