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
#include "pch.h"

#include "HolographicMain.winrt.h"
#include "DirectXHelper.winrt.h"
#include <DirectXColors.h>

#include <windows.graphics.directx.direct3d11.interop.h>
#include <Collection.h>
#include <ppltasks.h>    // For create_task

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::Holographic;
using namespace Windows::Perception::Spatial;
using namespace Windows::UI::Input::Spatial;
using namespace std::placeholders;


std::unique_ptr<HolographicMain> m_main;

void HolographicMain::begin()
{
	m_currentHolographicFrame = Update();

	//if (Render(m_currentHolographicFrame))
	//{
	//	m_deviceResources->Present(m_currentHolographicFrame);
	//}
	m_deviceResources->LockCameraResources();

	// Up-to-date frame predictions enhance the effectiveness of image stablization and
	// allow more accurate positioning of holograms.
	m_currentHolographicFrame->UpdateCurrentPrediction();

	m_currentPrediction = m_currentHolographicFrame->CurrentPrediction;
	m_currentCoordinateSystem = m_referenceFrame->GetStationaryCoordinateSystemAtTimestamp(m_currentPrediction->Timestamp);
	
	m_currentCamPose = m_currentPrediction->CameraPoses->GetAt(0);
	// This represents the device-based resources for a HolographicCamera.
	m_currentCameraResources = m_deviceResources->GetResourcesForCamera(m_currentCamPose->HolographicCamera);

	const auto context = m_deviceResources->GetD3DDeviceContext();
	// Set render targets to the current holographic camera.
	const auto depthStencilView = m_currentCameraResources->GetDepthStencilView();
	ID3D11RenderTargetView *const targets[1] = { m_currentCameraResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, depthStencilView);

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(targets[0], DirectX::Colors::Transparent);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

}

void HolographicMain::beginRender(int eye)
{
	// The system changes the viewport on a per-frame basis for system optimizations.
	auto m_d3dViewport = CD3D11_VIEWPORT(
		m_currentCamPose->Viewport.Left,
		m_currentCamPose->Viewport.Top,
		m_currentCamPose->Viewport.Width,
		m_currentCamPose->Viewport.Height
	);
	const auto context = m_deviceResources->GetD3DDeviceContext();
	context->RSSetViewports(1, &m_d3dViewport);

	
}

SensorState HolographicMain::getSensorState(int eye)
{
	SensorState state;

	HolographicFramePrediction^ prediction = m_currentHolographicFrame->CurrentPrediction;
	SpatialCoordinateSystem^ currentCoordinateSystem = m_referenceFrame->GetStationaryCoordinateSystemAtTimestamp(prediction->Timestamp);
	auto firstCamPose = prediction->CameraPoses->GetAt(0);
	// This represents the device-based resources for a HolographicCamera.
	DX::CameraResources* pCameraResources = m_deviceResources->GetResourcesForCamera(firstCamPose->HolographicCamera);

	return state;
}

void HolographicMain::endRender(int eye)
{
	m_deviceResources->UnlockCameraResources();
}

void HolographicMain::warpSwap()
{
	m_deviceResources->Present(m_currentHolographicFrame);
}



// Loads and initializes application assets when the application is loaded.
HolographicMain::HolographicMain(
	const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the device is lost or recreated.
	m_deviceResources->RegisterDeviceNotify(this);
}


void HolographicMain::SetHolographicSpace(
	HolographicSpace^ holographicSpace)
{
	UnregisterHolographicEventHandlers();

	m_holographicSpace = holographicSpace;

	//register camera events
	m_cameraAddedToken =
		m_holographicSpace->CameraAdded +=
		ref new Windows::Foundation::TypedEventHandler<HolographicSpace^, HolographicSpaceCameraAddedEventArgs^>(
			std::bind(&HolographicMain::OnCameraAdded, this, _1, _2)
			);
	m_cameraRemovedToken =
		m_holographicSpace->CameraRemoved +=
		ref new Windows::Foundation::TypedEventHandler<HolographicSpace^, HolographicSpaceCameraRemovedEventArgs^>(
			std::bind(&HolographicMain::OnCameraRemoved, this, _1, _2)
			);


	// Use the default SpatialLocator to track the motion of the device.
	m_locator = SpatialLocator::GetDefault();

	m_positionalTrackingDeactivatingToken =
		m_locator->PositionalTrackingDeactivating +=
		ref new Windows::Foundation::TypedEventHandler<SpatialLocator^, SpatialLocatorPositionalTrackingDeactivatingEventArgs^>(
			std::bind(&HolographicMain::OnPositionalTrackingDeactivating, this, _1, _2)
			);
	// follow along with the device's location.
	m_referenceFrame = m_locator->CreateAttachedFrameOfReferenceAtCurrentHeading();

}

void HolographicMain::OnPositionalTrackingDeactivating(
	SpatialLocator^ sender,
	SpatialLocatorPositionalTrackingDeactivatingEventArgs^ args)
{
	// Without positional tracking, spatial meshes will not be locatable.
	args->Canceled = true;
}

void HolographicMain::UnregisterHolographicEventHandlers()
{
	if (m_holographicSpace != nullptr)
	{
		// Clear previous event registrations.

		if (m_cameraAddedToken.Value != 0)
		{
			m_holographicSpace->CameraAdded -= m_cameraAddedToken;
			m_cameraAddedToken.Value = 0;
		}

		if (m_cameraRemovedToken.Value != 0)
		{
			m_holographicSpace->CameraRemoved -= m_cameraRemovedToken;
			m_cameraRemovedToken.Value = 0;
		}
	}

	if (m_locator != nullptr)
	{
		m_locator->PositionalTrackingDeactivating -= m_positionalTrackingDeactivatingToken;
	}
}

HolographicMain::~HolographicMain()
{
	// Deregister device notification.
	m_deviceResources->RegisterDeviceNotify(nullptr);
	UnregisterHolographicEventHandlers();
}


// Updates the application state once per frame.
HolographicFrame^ HolographicMain::Update()
{
	// Before doing the timer update, there is some work to do per-frame
	// to maintain holographic rendering. First, we will get information
	// about the current frame.

	// The HolographicFrame has information that the app needs in order
	// to update and render the current frame. The app begins each new
	// frame by calling CreateNextFrame.
	HolographicFrame^ holographicFrame = m_holographicSpace->CreateNextFrame();

	// Get a prediction of where holographic cameras will be when this frame
	// is presented.
	HolographicFramePrediction^ prediction = holographicFrame->CurrentPrediction;

	// Back buffers can change from frame to frame. Validate each buffer, and recreate
	// resource views and depth buffers as needed.
	m_deviceResources->EnsureCameraResources(holographicFrame, prediction);

	// The holographic frame will be used to get up-to-date view and projection matrices and
	// to present the swap chain.
	return holographicFrame;
}

// Renders the current frame to each holographic camera, according to the
// current application and spatial positioning state. Returns true if the
// frame was rendered to at least one camera.
bool HolographicMain::Render(
	HolographicFrame^ holographicFrame)
{
	// Lock the set of holographic camera resources, then draw to each camera
	// in this frame.
	return m_deviceResources->UseHolographicCameraResources<bool>(
		[this, holographicFrame](std::map<UINT32, std::unique_ptr<DX::CameraResources>>& cameraResourceMap)
	{
		// Up-to-date frame predictions enhance the effectiveness of image stablization and
		// allow more accurate positioning of holograms.
		holographicFrame->UpdateCurrentPrediction();
		HolographicFramePrediction^ prediction = holographicFrame->CurrentPrediction;

		bool atLeastOneCameraRendered = false;
		for (auto cameraPose : prediction->CameraPoses)
		{
			// This represents the device-based resources for a HolographicCamera.
			DX::CameraResources* pCameraResources = cameraResourceMap[cameraPose->HolographicCamera->Id].get();
//
//			// Get the device context.
			const auto context = m_deviceResources->GetD3DDeviceContext();
			const auto depthStencilView = pCameraResources->GetDepthStencilView();

			// Set render targets to the current holographic camera.
			ID3D11RenderTargetView *const targets[1] = { pCameraResources->GetBackBufferRenderTargetView() };
			context->OMSetRenderTargets(1, targets, depthStencilView);

			// Clear the back buffer and depth stencil view.
			context->ClearRenderTargetView(targets[0], DirectX::Colors::Red);
			context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

//			// The view and projection matrices for each holographic camera will change
//			// every frame. This function refreshes the data in the constant buffer for
//			// the holographic camera indicated by cameraPose.
//			pCameraResources->UpdateViewProjectionBuffer(m_deviceResources, cameraPose, currentCoordinateSystem);
//
//			// Attach the view/projection constant buffer for this camera to the graphics pipeline.
//			bool cameraActive = pCameraResources->AttachViewProjectionBuffer(m_deviceResources);
//
//#ifdef DRAW_SAMPLE_CONTENT
//			// Only render world-locked content when positional tracking is active.
//			if (cameraActive)
//			{
//				// Draw the sample hologram.
//				m_meshRenderer->Render(pCameraResources->IsRenderingStereoscopic(), m_drawWireframe);
//			}
//#endif
			atLeastOneCameraRendered = true;
		}

		return atLeastOneCameraRendered;
	});
}


// Notifies classes that use Direct3D device resources that the device resources
// need to be released before this method returns.
void HolographicMain::OnDeviceLost()
{
	//release device dependent resources
}

// Notifies classes that use Direct3D device resources that the device resources
// may now be recreated.
void HolographicMain::OnDeviceRestored()
{
	//recreate device dependent resources
}

void HolographicMain::OnCameraAdded(
	HolographicSpace^ sender,
	HolographicSpaceCameraAddedEventArgs^ args)
{
	Deferral^ deferral = args->GetDeferral();
	HolographicCamera^ holographicCamera = args->Camera;
	create_task([this, deferral, holographicCamera]()
	{
		// Create device-based resources for the holographic camera and add it to the list of
		// cameras used for updates and rendering. Notes:
		//   * Since this function may be called at any time, the AddHolographicCamera function
		//     waits until it can get a lock on the set of holographic camera resources before
		//     adding the new camera. At 60 frames per second this wait should not take long.
		//   * A subsequent Update will take the back buffer from the RenderingParameters of this
		//     camera's CameraPose and use it to create the ID3D11RenderTargetView for this camera.
		//     Content can then be rendered for the HolographicCamera.
		m_deviceResources->AddHolographicCamera(holographicCamera);

		// Holographic frame predictions will not include any information about this camera until
		// the deferral is completed.
		deferral->Complete();
	});
}

void HolographicMain::OnCameraRemoved(
	HolographicSpace^ sender,
	HolographicSpaceCameraRemovedEventArgs^ args)
{
	// Before letting this callback return, ensure that all references to the back buffer
	// are released.
	// Since this function may be called at any time, the RemoveHolographicCamera function
	// waits until it can get a lock on the set of holographic camera resources before
	// deallocating resources for this camera. At 60 frames per second this wait should
	// not take long.
	m_deviceResources->RemoveHolographicCamera(args->Camera);
}
