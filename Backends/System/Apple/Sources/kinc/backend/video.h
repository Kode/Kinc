#pragma once

#include <kinc/graphics4/texture.h>

namespace Kore {
	class VideoSoundStream;

	class Video {
	private:
		void updateImage();
		double start;
		double videoStart;
		double next;
		// double audioTime;
		unsigned long long audioTime;
		bool playing;
		VideoSoundStream* sound;
		void load(double startTime);

	public:
		Video(const char* filename);
		~Video();
		void play();
		void pause();
		void stop();
		int width();
		int height();
		kinc_g4_texture_t *currentImage();
		double duration; // milliseconds
		double position; // milliseconds
		bool finished;
		bool paused;
		void update(double time);

	private:
		bool image_initialized;
		kinc_g4_texture_t image;
		double lastTime;
		int myWidth;
		int myHeight;

	private:

		struct Impl;
		Impl* impl;
	};
}
