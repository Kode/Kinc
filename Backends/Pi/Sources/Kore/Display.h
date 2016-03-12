#pragma once

namespace Kore { namespace Display {
	struct DeviceInfo {
		int number;
		bool isAvailable;
		char name[32];
		int x;
		int y;
        int width;
        int height;
		bool isPrimary;

		 DeviceInfo() {
			number = -1;
			name[0] = 0;
			isAvailable = false;
			isPrimary = false;
		}
	};

	void enumerate();
	const DeviceInfo * primaryScreen();
	const DeviceInfo * screenById( int id );
}}
