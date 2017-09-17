#pragma once
//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
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

	void SetDeviceAndContext(Microsoft::WRL::ComPtr<ID3D11Device4> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext3>  context);
	Microsoft::WRL::ComPtr<IDXGIAdapter3> HolographicFrameController::GetCompatibleDxgiAdapter();

	// Starts the holographic frame and updates the content.
	Windows::Graphics::Holographic::HolographicFrame^ Update();

	// IDeviceNotify
	virtual void OnDeviceLost();
	virtual void OnDeviceRestored();

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
	void CreateHolographicSpace(Windows::UI::Core::CoreWindow^ window);
	// Asynchronously creates resources for new holographic cameras.
	void OnCameraAdded(
		Windows::Graphics::Holographic::HolographicSpace^ sender,
		Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs^ args);

	// Synchronously releases resources for holographic cameras that are no longer
	// attached to the system.
	void OnCameraRemoved(
		Windows::Graphics::Holographic::HolographicSpace^ sender,
		Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs^ args);

	// Used to prevent the device from deactivating positional tracking, which is 
	// necessary to continue to receive spatial mapping data.
	void OnPositionalTrackingDeactivating(
		Windows::Perception::Spatial::SpatialLocator^ sender,
		Windows::Perception::Spatial::SpatialLocatorPositionalTrackingDeactivatingEventArgs^ args);
	
	void OnPositionalTrackingLocatabilityChanged(
		Windows::Perception::Spatial::SpatialLocator^ sender,
		Platform::Object^ args);

	// Clears event registration state. Used when changing to a new HolographicSpace
	// and when tearing down AppMain.
	void UnregisterHolographicEventHandlers();

	Windows::Graphics::Holographic::HolographicFrame^ m_currentHolographicFrame;
	Windows::Graphics::Holographic::HolographicFramePrediction^ m_currentPrediction;
	Windows::Perception::Spatial::SpatialCoordinateSystem^ m_currentCoordinateSystem;
	Windows::Graphics::Holographic::HolographicCameraPose^ m_currentCamPose;
	DX::CameraResources* m_currentCameraResources;

	// SpatialLocator that is attached to the primary camera.
	Windows::Perception::Spatial::SpatialLocator^                       m_locator;

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