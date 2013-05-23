#pragma once

struct IDirectSoundBuffer8;

namespace Kore {
	namespace Sound {
		class SoundHandle {
		public:
			SoundHandle(const char* filename, bool loops);
			~SoundHandle();
			void play();
			void setVolume(float volume);
			int length();
			int position();
		private:
			void* player;
			IDirectSoundBuffer8* secondaryBuffer;
			uint size;
			uint sampleRate;
			uint bitsPerSample;
			bool loops;
			bool playing;
		};

		void init();
		void shutdown();
	}
}
