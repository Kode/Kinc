#include <Kore/Graphics/Texture.h>
#include <Kore/IO/FileReader.h>
#include <objc/runtime.h>

namespace Kore {
	class VideoSoundStream;
	
	class Video {
	private:
		id videoAsset;
		id assetReader;
		id videoTrackOutput;
		id audioTrackOutput;
		void updateImage();
		double start;
		double videoStart;
		double next;
		//double audioTime;
		unsigned long long audioTime;
		bool playing;
		VideoSoundStream* sound;
		id url;
		void load(double startTime);
	public:
		Video(const char* filename);
		~Video();
		void play();
		void pause();
		void stop();
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
