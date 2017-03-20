#pragma once

namespace Kore {
	namespace Display {
		struct DeviceInfo {
			DeviceInfo() {

			}
		};

		void enumerate();
		const DeviceInfo* primary();
		const DeviceInfo* byId(int id);
		int height(int index);
		int width(int index);
	}
}
