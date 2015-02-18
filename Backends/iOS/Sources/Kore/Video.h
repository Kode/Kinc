#include <Kore/Graphics/Texture.h>
#include <Kore/IO/FileReader.h>

namespace Kore {
	class VideoSoundStream;
	
	class Video {
	private:
		void* assetReader;
		void* videoTrackOutput;
		void* audioTrackOutput;
		void updateImage();
		double start;
		double next;
		double audioTime;
		bool playing;
		VideoSoundStream* sound;
	public:
		Video(const char* filename);
		~Video();
		void play();
		void pause();
		int width();
		int height();
		Texture* currentImage();
		double duration; //milliseconds
		double position; //milliseconds
		bool finished;
		bool paused;
		void update(double time);
		
	private:
		Texture* image;
		double lastTime;
		int myWidth;
		int myHeight;
	private:
		void video_write();
		Kore::Image* videoImage;
		FileReader* infile;
	};
}
