#include <Kore/Graphics/Texture.h>
#include <Kore/IO/FileReader.h>

namespace Kore {
	class VideoSoundStream {
	public:
		VideoSoundStream(int nChannels, int freq);
		void insertData(float* data, int nSamples);
		float nextSample();
		bool ended();
		//void* audioTrackOutput;
	private:
		float* buffer;
		const int bufferSize;
		int bufferWritePosition;
		int bufferReadPosition;
		u64 read;
		u64 written;
	};
}
