#pragma once

#include <Kore/Vr/SensorState.h>

enum TrackingOrigin { Stand, Sit };

namespace Kore {
	namespace VrInterface {
		void* init(void* hinst);
		void begin();
		void beginRender(int eye);
		void endRender(int eye);
		SensorState* getSensorState(int eye);
		void warpSwap(/*kha::vr::TimeWarpParms_obj* parms*/);

		void updateTrackingOrigin(TrackingOrigin origin);
		void resetHmdPose();
		void ovrShutdown();

		void createOculusTexture();
	}
}
