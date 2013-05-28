#pragma once

namespace Kore {
	namespace Audio {
		void init();
		void update();
		void shutdown();

		void play(s16* data, int size);
	}
}
