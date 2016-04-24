#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#undef RegisterClass

namespace Kore { namespace Display {
	struct DeviceInfo {
		HMONITOR id;
		bool isAvailable;
		char name[32];
		int x;
		int y;
        int width;
        int height;
		bool isPrimary;

		DeviceInfo () {
			id = NULL;
			name[0] = 0;
			isAvailable = false;
			isPrimary = false;
		}
	};

	void enumerate();
	const DeviceInfo * primary();
	const DeviceInfo * byId( int id );   
	int height( int index );
	int width( int index ); 
}}
