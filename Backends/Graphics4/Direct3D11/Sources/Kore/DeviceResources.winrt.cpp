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
#include "DeviceResources.winrt.h"
#include "DirectXHelper.winrt.h"
#include "CameraResources.winrt.h"
#include <Kore/WinError.h>

#include <Collection.h>
#include <windows.graphics.directx.direct3d11.interop.h>

using namespace Kore;
using namespace Microsoft::WRL;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Graphics::Display;
using namespace Windows::Graphics::Holographic;


// Constructor for DeviceResources.
DX::DeviceResources::DeviceResources()
{
	m_cameraResLock= std::unique_lock<std::mutex>(m_cameraResourcesMutex, std::defer_lock);
    //CreateDeviceIndependentResources();
}

DX::CameraResources* DX::DeviceResources::GetResourcesForCamera(Windows::Graphics::Holographic::HolographicCamera^ camera)
{
	return m_cameraResources[camera->Id].get();
}

void DX::DeviceResources::LockCameraResources()
{
	m_cameraResLock.lock();
}

void DX::DeviceResources::UnlockCameraResources()
{
	m_cameraResLock.unlock();
}


void DX::DeviceResources::InitWithDevice(Microsoft::WRL::ComPtr<ID3D11Device4> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext3>  context)
{
	m_d3dDevice = device;
	m_d3dContext = context;

	// Acquire the DXGI interface for the Direct3D device.
	ComPtr<IDXGIDevice3> dxgiDevice;
	affirm(
		m_d3dDevice.As(&dxgiDevice)
	);

	// Wrap the native device using a WinRT interop object.
	m_d3dInteropDevice = CreateDirect3DDevice(dxgiDevice.Get());

	//// Cache the DXGI adapter.
	//// This is for the case of no preferred DXGI adapter, or fallback to WARP.
	//ComPtr<IDXGIAdapter> dxgiAdapter;
	//affirm(
	//	dxgiDevice->GetAdapter(&dxgiAdapter)
	//);
	//affirm(
	//	dxgiAdapter.As(&m_dxgiAdapter)
	//);
}

// Validates the back buffer for each HolographicCamera and recreates
// resources for back buffers that have changed.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::EnsureCameraResources(
    HolographicFrame^ frame,
    HolographicFramePrediction^ prediction)
{
    UseHolographicCameraResources<void>([this, frame, prediction](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        for (const auto& pose : prediction->CameraPoses)
        {
            HolographicCameraRenderingParameters^ renderingParameters = frame->GetRenderingParameters(pose);
            CameraResources* pCameraResources = cameraResourceMap[pose->HolographicCamera->Id].get();

            pCameraResources->CreateResourcesForBackBuffer(this, renderingParameters);
        }
    });
}

// Prepares to allocate resources and adds resource views for a camera.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::AddHolographicCamera(HolographicCamera^ camera)
{
    UseHolographicCameraResources<void>([this, camera](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        cameraResourceMap[camera->Id] = std::make_unique<CameraResources>(camera);
    });
}

// Deallocates resources for a camera and removes the camera from the set.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::RemoveHolographicCamera(HolographicCamera^ camera)
{
    UseHolographicCameraResources<void>([this, camera](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        CameraResources* pCameraResources = cameraResourceMap[camera->Id].get();

        if (pCameraResources != nullptr)
        {
            pCameraResources->ReleaseResourcesForBackBuffer(this);
            cameraResourceMap.erase(camera->Id);
        }
    });
}

// Recreate all device resources and set them back to the current state.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::HandleDeviceLost()
{
    //if (m_deviceNotify != nullptr)
    //{
    //    m_deviceNotify->OnDeviceLost();
    //}

    //UseHolographicCameraResources<void>([this](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    //{
    //    for (auto& pair : cameraResourceMap)
    //    {
    //        CameraResources* pCameraResources = pair.second.get();
    //        pCameraResources->ReleaseResourcesForBackBuffer(this);
    //    }
    //});

    //InitializeUsingHolographicSpace();

    //if (m_deviceNotify != nullptr)
    //{
    //    m_deviceNotify->OnDeviceRestored();
    //}
}

// Register our DeviceNotify to be informed on device lost and creation.
void DX::DeviceResources::RegisterDeviceNotify(DX::IDeviceNotify* deviceNotify)
{
    m_deviceNotify = deviceNotify;
}

// Call this method when the app suspends. It provides a hint to the driver that the app
// is entering an idle state and that temporary buffers can be reclaimed for use by other apps.
void DX::DeviceResources::Trim()
{
    m_d3dContext->ClearState();

    ComPtr<IDXGIDevice3> dxgiDevice;
    DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));
    dxgiDevice->Trim();
}

// Present the contents of the swap chain to the screen.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::Present(HolographicFrame^ frame)
{
    // By default, this API waits for the frame to finish before it returns.
    // Holographic apps should wait for the previous frame to finish before
    // starting work on a new frame. This allows for better results from
    // holographic frame predictions.
    HolographicFramePresentResult presentResult = frame->PresentUsingCurrentPrediction();

    HolographicFramePrediction^ prediction = frame->CurrentPrediction;
    UseHolographicCameraResources<void>([this, prediction](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        for (auto cameraPose : prediction->CameraPoses)
        {
            // This represents the device-based resources for a HolographicCamera.
            DX::CameraResources* pCameraResources = cameraResourceMap[cameraPose->HolographicCamera->Id].get();

            // Discard the contents of the render target.
            // This is a valid operation only when the existing contents will be
            // entirely overwritten. If dirty or scroll rects are used, this call
            // should be removed.
            m_d3dContext->DiscardView(pCameraResources->GetBackBufferRenderTargetViewLeft());
			m_d3dContext->DiscardView(pCameraResources->GetBackBufferRenderTargetViewRight());

            // Discard the contents of the depth stencil.
            m_d3dContext->DiscardView(pCameraResources->GetDepthStencilViewLeft());
			m_d3dContext->DiscardView(pCameraResources->GetDepthStencilViewRight());
			
        }
    });

    // The PresentUsingCurrentPrediction API will detect when the graphics device
    // changes or becomes invalid. When this happens, it is considered a Direct3D
    // device lost scenario.
    if (presentResult == HolographicFramePresentResult::DeviceRemoved)
    {
        // The Direct3D device, context, and resources should be recreated.
        HandleDeviceLost();
    }
}
