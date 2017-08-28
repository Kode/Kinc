#pragma once

#include <Kore/Vr/SensorState.h>
#include <Kore/Vr/CameraImage.h>

enum TrackingOrigin { Stand, Sit };

namespace Kore {
	namespace VrInterface {
		void* init(void* hinst, const char* title, const char* windowClassName);
		void begin();
		void beginRender(int eye);
		void endRender(int eye);
		SensorState getSensorState(int eye);
		VrPoseState getController(int index);
		void warpSwap();

		void updateTrackingOrigin(TrackingOrigin origin);
		void resetHmdPose();
		void ovrShutdown();

		//AR device functions
		CameraImage* getCurrentCameraImage();

	}
}
