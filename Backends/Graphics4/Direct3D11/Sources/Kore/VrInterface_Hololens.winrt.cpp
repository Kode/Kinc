#include "pch.h"

#ifdef KORE_HOLOLENS

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
	holographicFrameController.reset();
	videoFrameProcessor.reset();
}

void VrInterface::resetHmdPose()
{
	//not supported for hololens
}

void VrInterface::updateTrackingOrigin(TrackingOrigin origin)
{
	//could be implemented by setting up the tracking coordinate system with an offset
}

CameraImage* VrInterface::getCurrentCameraImage()
{
	if (videoFrameProcessor == nullptr) {
		return nullptr;
	}
	return videoFrameProcessor->getCurrentCameraImage(holographicFrameController->getCurrentWorldCoordinateSystem());
}

#endif