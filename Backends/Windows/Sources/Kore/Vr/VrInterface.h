#pragma once

//#include <kha/Image.h>
//#include <kha/math/Quaternion.h>
//#include <kha/vr/TimeWarpParms.h>

#include <Kore/Vr/SensorState.h>

namespace Kore {
	namespace VrInterface {
		//void WarpSwap(kha::vr::TimeWarpParms_obj* parms);
		void* Init(void* hinst);
		SensorState* getSensorState();

		void ovrShutdown();
		void changeTrackingOrigin(bool standUp);
		void recenterTracking();
		void getHMDRessolution(int eye, int& w, int&h);
	}
}
