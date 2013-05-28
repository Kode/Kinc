#pragma once

namespace Kore {
	class Sound {
	public:
		Sound(const char* filename);
		void play();
	private:
		u8* data;
		int size;
	};
}
