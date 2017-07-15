#include "pch.h"

#define NOMINMAX
//#include "DeviceResources.winrt.h"

#include "Hololens.winrt.h"
#include <Kore/Vr/VrInterface.h>

//#include "DirectXHelper.winrt.h"
#include <ppltasks.h>    // For create_task


using namespace Kore;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;

Windows::Foundation::EventRegistrationToken                         m_cameraAddedToken;
Windows::Foundation::EventRegistrationToken                         m_cameraRemovedToken;

std::unique_ptr<HolographicMain> m_main;
HolographicSpace^ m_holographicSpace;
std::shared_ptr<DX::DeviceResources>  m_deviceResources;
SpatialLocatorAttachedFrameOfReference^ m_referenceFrame;
HolographicFrame^ m_currentHolographicFrame;


void* VrInterface::init(void* hinst, const char* title, const char* windowClassName)
{
	return nullptr;
}

void VrInterface::begin() {
	m_currentHolographicFrame = m_main->Update();
	////SpatialCoordinateSystem^ currentCoordinateSystem = m_referenceFrame->GetStationaryCoordinateSystemAtTimestamp(prediction->Timestamp);

	if (m_main->Render(m_currentHolographicFrame))
	{
		
	}

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
	m_deviceResources->Present(m_currentHolographicFrame);
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

