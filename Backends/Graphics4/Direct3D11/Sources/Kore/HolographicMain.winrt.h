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

#include "DeviceResources.winrt.h"
//#include "Common\StepTimer.h"


class HolographicMain : public DX::IDeviceNotify
{
public:
	HolographicMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
	~HolographicMain();

	// Sets the holographic space. This is our closest analogue to setting a new window
	// for the app.
	void SetHolographicSpace(Windows::Graphics::Holographic::HolographicSpace^ holographicSpace);

	// Starts the holographic frame and updates the content.
	Windows::Graphics::Holographic::HolographicFrame^ Update();

	// Renders holograms, including world-locked content.
	bool Render(Windows::Graphics::Holographic::HolographicFrame^ holographicFrame);

	// Handle saving and loading of app state owned by AppMain.
	void SaveAppState();
	void LoadAppState();

	// IDeviceNotify
	virtual void OnDeviceLost();
	virtual void OnDeviceRestored();

private:
	// Asynchronously creates resources for new holographic cameras.
	void OnCameraAdded(
		Windows::Graphics::Holographic::HolographicSpace^ sender,
		Windows::Graphics::Holographic::HolographicSpaceCameraAddedEventArgs^ args);

	// Synchronously releases resources for holographic cameras that are no longer
	// attached to the system.
	void OnCameraRemoved(
		Windows::Graphics::Holographic::HolographicSpace^ sender,
		Windows::Graphics::Holographic::HolographicSpaceCameraRemovedEventArgs^ args);

	// Clears event registration state. Used when changing to a new HolographicSpace
	// and when tearing down AppMain.
	void UnregisterHolographicEventHandlers();

	// Cached pointer to device resources.
	std::shared_ptr<DX::DeviceResources>                                m_deviceResources;

	// Represents the holographic space around the user.
	Windows::Graphics::Holographic::HolographicSpace^                   m_holographicSpace;

	// Event registration tokens.
	Windows::Foundation::EventRegistrationToken                         m_cameraAddedToken;
	Windows::Foundation::EventRegistrationToken                         m_cameraRemovedToken;
};
