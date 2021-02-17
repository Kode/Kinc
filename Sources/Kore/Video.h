#pragma once

#include <kinc/video.h>

namespace Kore {
	class Video {
	public:
		Video(const char *filename);
		~Video();
		void play();
		void pause();
		void stop();
		int width();
		int height();
		kinc_g4_texture_t *currentImage();
		double duration();
		double position();
		bool finished();
		bool paused();
		void update(double time);

	private:
		kinc_video_t video;
	};
}
