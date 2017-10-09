#pragma once
//*********************************************************
// Original author Microsoft (license MIT)
// https://github.com/Microsoft/Windows-universal-samples/tree/master/Samples/HolographicSpatialMapping
// Code is adapted for Kore integration.
//*********************************************************

#define NOMINMAX
#include "DeviceResources.winrt.h"
#include "VideoFrameProcessor.winrt.h"

#include <Kore/Vr/SensorState.h>

using namespace Windows::UI::Core;
using namespace Windows::System;

class HolographicFrameController : public DX::IDeviceNotify
{
public:
	HolographicFrameController(Windows::UI::Core::CoreWindow^ window);
	~HolographicFrameController();

	void setDeviceAndContext(Microsoft::WRL::ComPtr<ID3D11Device4> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext3>  context);
	Microsoft::WRL::ComPtr<IDXGIAdapter3> getCompatibleDxgiAdapter();

	// Starts the holographic frame and updates the content.
	Windows::Graphics::Holographic::HolographicFrame^ createNextHolographicFrame();

	// IDeviceNotify
	virtual void onDeviceLost();
	virtual void onDeviceRestored();

	//
	Windows::Perception::Spatial::SpatialCoordinateSystem^ getCurrentWorldCoordinateSystem();

	//vr interface
	void begin();
	void beginRender(int eye);
	SensorState getSensorState(int eye);
	void endRender(int eye);
	void warpSwap();
private:
	// Sets the holographic space. This is our closest analogue to setting a new window
	// for the app.
	void createHolographicSpace(Windows::UI::Core::CoreWindow^ window);
	void registerCameraEvents();
	void setupSpatialLocator();
	// Asynchronously creates resources for new holographic cameras.
	void onCameraAdded(
		Windows::Graphics::Holographic::HolographicSpace^ sender,
		Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs^ args);

	// Synchronously releases resources for holographic cameras that are no longer
	// attached to the system.
	void onCameraRemoved(
		Windows::Graphics::Holographic::HolographicSpace^ sender,
		Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs^ args);

	// Used to prevent the device from deactivating positional tracking, which is 
	// necessary to continue to receive spatial mapping data.
	void onPositionalTrackingDeactivating(
		Windows::Perception::Spatial::SpatialLocator^ sender,
		Windows::Perception::Spatial::SpatialLocatorPositionalTrackingDeactivatingEventArgs^ args);
	
	void onPositionalTrackingLocatabilityChanged(
		Windows::Perception::Spatial::SpatialLocator^ sender,
		Platform::Object^ args);

	// Clears event registration state. Used when changing to a new HolographicSpace
	// and when tearing down AppMain.
	void unregisterHolographicEventHandlers();

	Windows::Graphics::Holographic::HolographicFrame^ m_currentHolographicFrame;
	Windows::Graphics::Holographic::HolographicFramePrediction^ m_currentPrediction;
	Windows::Perception::Spatial::SpatialCoordinateSystem^ m_currentCoordinateSystem;
	Windows::Graphics::Holographic::HolographicCameraPose^ m_currentCamPose;
	DX::CameraResources* m_currentCameraResources;

	// SpatialLocator that is attached to the primary camera.
	Windows::Perception::Spatial::SpatialLocator^                       m_spatialLocator;

	// A reference frame attached to the holographic camera.
	Windows::Perception::Spatial::SpatialStationaryFrameOfReference^ m_referenceFrame;


	// Cached pointer to device resources.
	std::shared_ptr<DX::DeviceResources>                                m_deviceResources;

	// Represents the holographic space around the user.
	Windows::Graphics::Holographic::HolographicSpace^                   m_holographicSpace;

	// Event registration tokens.
	Windows::Foundation::EventRegistrationToken                         m_cameraAddedToken;
	Windows::Foundation::EventRegistrationToken                         m_cameraRemovedToken;
	Windows::Foundation::EventRegistrationToken                         m_positionalTrackingDeactivatingToken;
	Windows::Foundation::EventRegistrationToken                         m_positionalTrackingLocatabilityChangedToken;
};