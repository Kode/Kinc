#include <Kore/Graphics/Texture.h>
#include <Kore/IO/FileReader.h>

namespace Kore {
	class VideoSoundStream;
	
	class Video {
	public:
		Video(const char* filename) {
			duration = 1000 * 10;
			position = 0;
			finished = false;
			paused = false;
			image = new Texture(100, 100, Image::RGBA32, false);
		}
		~Video() {
			delete image;
		}
		void play() { }
		void pause() { }
		void stop() { }
		int width() { return 100; }
		int height() { return 100; }
		Texture* currentImage() { return image; }
		double duration; //milliseconds
		double position; //milliseconds
		bool finished;
		bool paused;
		void update(double time) { }
	private:
		Texture* image;
	};
}
