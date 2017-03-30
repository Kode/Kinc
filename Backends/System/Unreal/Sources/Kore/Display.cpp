#include "pch.h"

#include "Display.h"

#include <Kore/Log.h>

#include <cstdio>

namespace Kore {
	namespace Display {
		enum { MAXIMUM_DISPLAY_COUNT = 10 };

		DeviceInfo displays[10];
		
		void enumerate() {

		}

		int count() {
			return 1;
		}

		const DeviceInfo* primary() {
			return nullptr;
		}

		const DeviceInfo* byId(int id) {
			return nullptr;
		}

		int width(int index) {
			return 100;
		}

		int height(int index) {
			return 100;
		}

		int x(int index) {
			return 0;
		}

		int y(int index) {
			return 0;
		}

		bool isPrimary(int index) {
			return true;
		}
	}
}
