#include "pch.h"

#include <Kore/Vr/VrInterface.h>
using namespace Kore;

void* VrInterface::init(void* hinst, const char* title, const char* windowClassName)
{
	return nullptr;
}

void VrInterface::begin() {

}
	
void VrInterface::beginRender(int eye)
{
	
}

SensorState VrInterface::getSensorState(int eye)
{
	SensorState state;
	return state;
}

void VrInterface::endRender(int eye)
{
	
}

void VrInterface::warpSwap()
{

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

