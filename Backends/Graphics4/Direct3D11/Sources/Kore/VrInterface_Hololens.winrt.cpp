#include "pch.h"

#include "Hololens.winrt.h"
#include <Kore/Vr/VrInterface.h>

using namespace Kore;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

using namespace Windows::Media::Capture;
using namespace Windows::Media::Capture::Frames;
using namespace Windows::Media::MediaProperties;

void* VrInterface::init(void* hinst, const char* title, const char* windowClassName)
{
	return nullptr;
}

void VrInterface::begin() {
	holographicFrameController->begin();
}
	
void VrInterface::beginRender(int eye)
{
	holographicFrameController->beginRender(eye);
}

SensorState VrInterface::getSensorState(int eye)
{
	return holographicFrameController->getSensorState(eye);
}

void VrInterface::endRender(int eye)
{
	holographicFrameController->endRender(eye);
}

void VrInterface::warpSwap()
{
	holographicFrameController->warpSwap();
}



void VrInterface::ovrShutdown()
{
	
}

void VrInterface::resetHmdPose()
{
	
}

void VrInterface::updateTrackingOrigin(TrackingOrigin origin)
{
	
}

CameraImage* VrInterface::getCurrentCameraImage()
{
	if (videoFrameProcessor == nullptr) {
		return nullptr;
	}
	return videoFrameProcessor->getCurrentCameraImage(holographicFrameController->getCurrentWorldCoordinateSystem());
}
