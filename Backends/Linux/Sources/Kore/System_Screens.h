#pragma once

#include <X11/X.h>

namespace Kore { namespace System { namespace Monitor {
	struct Screen {
		XID id; // TODO (DK) is XID correct?
		bool isAvailable;
		char name[32];
		int x;
		int y;
        int width;
        int height;
		bool isPrimary;

		Screen() {
			id = nullptr;
			name[0] = 0;
			isAvailable = false;
			isPrimary = false;
		}
	};

	void enumerate();

	int count();

	const Screen* primaryScreen();
	const Screen* screenById( int id );
}}}
