#include "pch.h"

#include "Audio.h"

#include <stdio.h>

using namespace Kore;

void (*Audio2::audioCallback)(int samples) = nullptr;
Audio2::Buffer Audio2::buffer;
int Audio2::samplesPerSecond = 44100;
