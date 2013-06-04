#include "pch.h"
#include "SoundStream.h"
#include "stb_vorbis.h"
#include <Kore/Files/File.h>

using namespace Kore;

SoundStream::SoundStream(const char* filename, bool looping) : decoded(false), looping(looping) {
	DiskFile file;
	file.open(filename, DiskFile::ReadMode);
	buffer = new u8[file.getSize()];
	u8* filecontent = (u8*)file.readAll();
	for (uint i = 0; i < file.getSize(); ++i) {
		buffer[i] = filecontent[i];
	}
	vorbis = stb_vorbis_open_memory(buffer, file.getSize(), nullptr, nullptr);
}

float SoundStream::nextSample() {
	if (decoded) {
		decoded = false;
		return samples[1];
	}
	else {
		int read = stb_vorbis_get_samples_float_interleaved(vorbis, 2, &samples[0], 2);
		if (read == 0) {
			if (looping) {
				stb_vorbis_seek_start(vorbis);
				stb_vorbis_get_samples_float_interleaved(vorbis, 2, &samples[0], 2);
			}
			else return 0.0f;
		}
		decoded = true;
		return samples[0];
	}
}
