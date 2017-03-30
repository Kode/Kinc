#include <Kore/Graphics4/Texture.h>
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
		// double audioTime;
		unsigned long long audioTime;
		bool playing;
		VideoSoundStream* sound;

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
		void* androidVideo;
		int id;

	private:
		Graphics4::Texture* image;
		double lastTime;
		int myWidth;
		int myHeight;

	private:
		void video_write();
		Kore::Graphics4::Image* videoImage;
		FileReader* infile;
	};
}
