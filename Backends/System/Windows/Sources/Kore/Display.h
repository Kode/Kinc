#pragma once

struct HMONITOR__;
typedef HMONITOR__* HMONITOR;

namespace Kore {
	namespace Display {
		struct DeviceInfo {
			HMONITOR id;
			bool isAvailable;
			char name[32];
			int x;
			int y;
			int width;
			int height;
			bool isPrimary;

			DeviceInfo() {
				id = nullptr;
				name[0] = 0;
				isAvailable = false;
				isPrimary = false;
			}
		};

		void enumerate();
		const DeviceInfo* primary();
		const DeviceInfo* byId(int id);
		int height(int index);
		int width(int index);
		int x(int index);
		int y(int index);
		bool isPrimary(int index);
		int count();
	}
}
