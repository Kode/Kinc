#include <Kore/Graphics/Texture.h>
#include <Kore/IO/FileReader.h>

namespace Kore {
	class VideoSoundStream {
	public:
		VideoSoundStream(int nChannels, int freq) { }
		void insertData(float* data, int nSamples) { }
		float nextSample() { return 0; }
		bool ended() { return true; }
	};
}
