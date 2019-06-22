#pragma once

#include <Kore/Graphics4/Texture.h>

namespace Kore {
	class VideoSoundStream;

	class Video {
	public:
		Video(const char* filename) : image(100, 100, Kore::Graphics1::Image::RGBA32) {
			duration = 1000 * 10;
			position = 0;
			finished = false;
			paused = false;
		}
		~Video() {
			
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
		Kore::Graphics4::Texture *currentImage() {
			return &image;
		}
		double duration; // milliseconds
		double position; // milliseconds
		bool finished;
		bool paused;
		void update(double time) {}

	private:
		Kore::Graphics4::Texture image;
	};
}
