#include <Kore/Graphics4/Texture.h>
#include <Kore/IO/FileReader.h>
//#include <objc/runtime.h>

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
		Graphics4::Texture* currentImage();
		double duration; // milliseconds
		double position; // milliseconds
		bool finished;
		bool paused;
		void update(double time);
		
	private:
		Graphics4::Texture* image;
		double lastTime;
		int myWidth;
		int myHeight;
		
	private:
		void video_write();
		Kore::Graphics4::Image* videoImage;
		FileReader* infile;
		
		struct Impl;
		Impl* impl;
	};
}
