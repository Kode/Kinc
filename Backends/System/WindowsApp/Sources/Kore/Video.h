#pragma once

#include <kinc/graphics4/texture.h>
#include <kinc/io/filereader.h>

namespace Kore {
	class VideoSoundStream;

	class Video {
	public:
		Video(const char* filename) {
			duration = 1000 * 10;
			position = 0;
			finished = false;
			paused = false;
			kinc_g4_texture_init(&image, 100, 100, KINC_IMAGE_FORMAT_RGBA32);
		}
		~Video() {
			kinc_g4_texture_destroy(&image);
		}
		void play() {}
		void pause() {}
		void stop() {}
		int width() {
			return 100;
		}
		int height() {
			return 100;
		}
		kinc_g4_texture_t *currentImage() {
			return &image;
		}
		double duration; // milliseconds
		double position; // milliseconds
		bool finished;
		bool paused;
		void update(double time) {}

	private:
		kinc_g4_texture_t image;
	};
}
