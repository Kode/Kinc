#pragma once

#include <X11/X.h>

namespace Kore { namespace System { namespace Monitor {
	struct KoreScreen {
		XID id; // TODO (DK) is XID correct?
		bool isAvailable;
		char name[32];
		int x;
		int y;
        int width;
        int height;
		bool isPrimary;

		KoreScreen() {
			id = nullptr;
			name[0] = 0;
			isAvailable = false;
			isPrimary = false;
		}
	};

	void enumerate();

	int count();

	const KoreScreen* primaryScreen();
	const KoreScreen* screenById( int id );
}}}
