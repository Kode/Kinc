#include <Kore/Graphics4/Texture.h>
#include <Kore/IO/FileReader.h>

class CTextureRenderer;

namespace Kore {
	class VideoSoundStream;

	class Video {
	public:
		Video(const char* filename);
		~Video() {
			//delete image;
		}
		void play();
		void pause();
		void stop();
		int width();
		int height();
		Graphics4::Texture* currentImage();
		double duration;
		double position;
		bool finished;
		bool paused;
		void update(double time);

	private:
		//Graphics4::Texture* image;
		CTextureRenderer* renderer;
	};
}
