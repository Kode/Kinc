#include "pch.h"
#include <Kore/Sound/Music.h>
#include "SoundOgg.h"

extern IDirectSound8* dsound;

namespace {
	CDirectSoundOgg* music = nullptr;
}

void Kore::Music::play(const char* filename) {
	delete music;
	music = new CDirectSoundOgg(filename, dsound);
	music->Play(true);
}

void Kore::Music::stop() {
	if (music != nullptr) music->Stop();
}
