#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace Kore { namespace System { namespace Monitor {
	struct Screen {
		HMONITOR id;
		bool isAvailable;
		char name[32];
		int x;
		int y;
        int width;
        int height;
		bool isPrimary;

		Screen() {
			id = NULL;
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
