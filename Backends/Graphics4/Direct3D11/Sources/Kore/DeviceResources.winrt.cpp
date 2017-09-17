//*********************************************************
// Original author Microsoft (license MIT)
// https://github.com/Microsoft/Windows-universal-samples/tree/master/Samples/HolographicSpatialMapping
// Code is adapted for Kore integration.
//*********************************************************

#include "pch.h"

#ifdef KORE_HOLOLENS 
#include "DeviceResources.winrt.h"
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

DX::CameraResources* DX::DeviceResources::getResourcesForCamera(Windows::Graphics::Holographic::HolographicCamera^ camera)
{
	return m_cameraResources[camera->Id].get();
}

void DX::DeviceResources::lockCameraResources()
{
	m_cameraResLock.lock();
}

void DX::DeviceResources::unlockCameraResources()
{
	m_cameraResLock.unlock();
}


void DX::DeviceResources::initWithDevice(Microsoft::WRL::ComPtr<ID3D11Device4> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext3>  context)
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
void DX::DeviceResources::ensureCameraResources(
    HolographicFrame^ frame,
    HolographicFramePrediction^ prediction)
{
    useHolographicCameraResources<void>([this, frame, prediction](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        for (const auto& pose : prediction->CameraPoses)
        {
            HolographicCameraRenderingParameters^ renderingParameters = frame->GetRenderingParameters(pose);
            CameraResources* pCameraResources = cameraResourceMap[pose->HolographicCamera->Id].get();

            pCameraResources->createResourcesForBackBuffer(this, renderingParameters);
        }
    });
}

// Prepares to allocate resources and adds resource views for a camera.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::addHolographicCamera(HolographicCamera^ camera)
{
    useHolographicCameraResources<void>([this, camera](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        cameraResourceMap[camera->Id] = std::make_unique<CameraResources>(camera);
    });
}

// Deallocates resources for a camera and removes the camera from the set.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::removeHolographicCamera(HolographicCamera^ camera)
{
    useHolographicCameraResources<void>([this, camera](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        CameraResources* pCameraResources = cameraResourceMap[camera->Id].get();

        if (pCameraResources != nullptr)
        {
            pCameraResources->releaseResourcesForBackBuffer(this);
            cameraResourceMap.erase(camera->Id);
        }
    });
}

// Recreate all device resources and set them back to the current state.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::handleDeviceLost()
{
    //if (m_deviceNotify != nullptr)
    //{
    //    m_deviceNotify->onDeviceLost();
    //}

    //useHolographicCameraResources<void>([this](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
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
    //    m_deviceNotify->onDeviceRestored();
    //}
}

// Register our DeviceNotify to be informed on device lost and creation.
void DX::DeviceResources::registerDeviceNotify(DX::IDeviceNotify* deviceNotify)
{
    m_deviceNotify = deviceNotify;
}

// Present the contents of the swap chain to the screen.
// Locks the set of holographic camera resources until the function exits.
void DX::DeviceResources::present(HolographicFrame^ frame)
{
    // By default, this API waits for the frame to finish before it returns.
    // Holographic apps should wait for the previous frame to finish before
    // starting work on a new frame. This allows for better results from
    // holographic frame predictions.
    HolographicFramePresentResult presentResult = frame->PresentUsingCurrentPrediction();

    HolographicFramePrediction^ prediction = frame->CurrentPrediction;
    useHolographicCameraResources<void>([this, prediction](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
    {
        for (auto cameraPose : prediction->CameraPoses)
        {
            // This represents the device-based resources for a HolographicCamera.
            DX::CameraResources* pCameraResources = cameraResourceMap[cameraPose->HolographicCamera->Id].get();

            // Discard the contents of the render target.
            // This is a valid operation only when the existing contents will be
            // entirely overwritten. If dirty or scroll rects are used, this call
            // should be removed.
            m_d3dContext->DiscardView(pCameraResources->getBackBufferRenderTargetViewLeft());
			m_d3dContext->DiscardView(pCameraResources->getBackBufferRenderTargetViewRight());

            // Discard the contents of the depth stencil.
            m_d3dContext->DiscardView(pCameraResources->getDepthStencilViewLeft());
			m_d3dContext->DiscardView(pCameraResources->getDepthStencilViewRight());
			
        }
    });

    // The PresentUsingCurrentPrediction API will detect when the graphics device
    // changes or becomes invalid. When this happens, it is considered a Direct3D
    // device lost scenario.
    if (presentResult == HolographicFramePresentResult::DeviceRemoved)
    {
        // The Direct3D device, context, and resources should be recreated.
        handleDeviceLost();
    }
}

#endif