#pragma once

#include <kinc/graphics4/texture.h>
#include <kinc/io/filereader.h>

class CTextureRenderer;

namespace Kore {
	class VideoSoundStream;

	class Video {
	public:
		Video(const char *filename);
		~Video() {
			// delete image;
		}
		void play();
		void pause();
		void stop();
		int width();
		int height();
		kinc_g4_texture_t *currentImage();
		double duration;
		double position;
		bool finished;
		bool paused;
		void update(double time);

	private:
		// Graphics4::Texture* image;
		CTextureRenderer *renderer;
	};
}
