//*********************************************************
// Original author Microsoft (license MIT)
// https://github.com/Microsoft/Windows-universal-samples/tree/master/Samples/HolographicSpatialMapping
// Code is adapted for Kore integration.
//*********************************************************


#include "pch.h"

#ifdef KORE_HOLOLENS 

#include <Kore/WinError.h>
#include "CameraResources.winrt.h"
#include "DeviceResources.winrt.h"
#include <windows.graphics.directx.direct3d11.interop.h>
#include <WindowsNumerics.h>
#include <iostream>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation::Numerics;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Graphics::Holographic;
using namespace Windows::Perception::Spatial;

DX::CameraResources::CameraResources(HolographicCamera^ camera) :
    m_holographicCamera(camera),
    m_isStereo(camera->IsStereo),
    m_d3dRenderTargetSize(camera->RenderTargetSize)
{
    m_d3dViewport = CD3D11_VIEWPORT(
        0.f, 0.f,
        m_d3dRenderTargetSize.Width,
        m_d3dRenderTargetSize.Height
        );
};

// Updates resources associated with a holographic camera's swap chain.
// The app does not access the swap chain directly, but it does create
// resource views for the back buffer.
void DX::CameraResources::createResourcesForBackBuffer(
    DX::DeviceResources* pDeviceResources,
    HolographicCameraRenderingParameters^ cameraParameters
    )
{
    const auto device = pDeviceResources->getD3DDevice();

    // Get the WinRT object representing the holographic camera's back buffer.
    IDirect3DSurface^ surface = cameraParameters->Direct3D11BackBuffer;

    // Get a DXGI interface for the holographic camera's back buffer.
    // Holographic cameras do not provide the DXGI swap chain, which is owned
    // by the system. The Direct3D back buffer resource is provided using WinRT
    // interop APIs.
    ComPtr<ID3D11Resource> resource;
    Kore::affirm(GetDXGIInterfaceFromObject(surface, IID_PPV_ARGS(&resource)));

    // Get a Direct3D interface for the holographic camera's back buffer.
    ComPtr<ID3D11Texture2D> cameraBackBuffer;
	Kore::affirm(resource.As(&cameraBackBuffer));

	D3D11_TEXTURE2D_DESC desc;
	cameraBackBuffer.Get()->GetDesc(&desc);
	if(desc.ArraySize!=2)
	{
		std::cout << "should be a 2 dimensional texture2d array";
		throw;
	}

    // Determine if the back buffer has changed. If so, ensure that the render target view
    // is for the current back buffer.
    if (m_d3dBackBuffer.Get() != cameraBackBuffer.Get())
    {
        // This can change every frame as the system moves to the next buffer in the
        // swap chain. This mode of operation will occur when certain rendering modes
        // are activated.
        m_d3dBackBuffer = cameraBackBuffer;

        // Create a render target view of the back buffer.
        // Creating this resource is inexpensive, and is better than keeping track of
        // the back buffers in order to pre-allocate render target views for each one.
		Kore::affirm(
            device->CreateRenderTargetView(
                m_d3dBackBuffer.Get(),
                nullptr,
                &m_d3dRenderTargetView
                )
            );

		D3D11_RENDER_TARGET_VIEW_DESC rtvdesc;
		m_d3dRenderTargetView->GetDesc(&rtvdesc);
		std::cout << rtvdesc.Format;


		//create single texture slice views for non stereo instancing rendering only
		D3D11_RENDER_TARGET_VIEW_DESC leftRtvDesc;
		leftRtvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		leftRtvDesc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		leftRtvDesc.Texture2DArray.ArraySize = 1;
		leftRtvDesc.Texture2DArray.MipSlice = 0;
		leftRtvDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0,0,1);

		Kore::affirm(
			device->CreateRenderTargetView(
				m_d3dBackBuffer.Get(),
				&leftRtvDesc,
				&m_d3dRenderTargetViewLeft
			)
		);

		D3D11_RENDER_TARGET_VIEW_DESC rightRtvDesc;
		rightRtvDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		rightRtvDesc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rightRtvDesc.Texture2DArray.ArraySize = 1;
		rightRtvDesc.Texture2DArray.MipSlice = 0;
		rightRtvDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, 1, 1);


		Kore::affirm(
			device->CreateRenderTargetView(
				m_d3dBackBuffer.Get(),
				&rightRtvDesc,
				&m_d3dRenderTargetViewRight
			)
		);


        // Get the DXGI format for the back buffer.
        // This information can be accessed by the app using CameraResources::getBackBufferDXGIFormat().
        D3D11_TEXTURE2D_DESC backBufferDesc;
        m_d3dBackBuffer->GetDesc(&backBufferDesc);
        m_dxgiFormat = backBufferDesc.Format;

        // Check for render target size changes.
        Windows::Foundation::Size currentSize = m_holographicCamera->RenderTargetSize;
        if (m_d3dRenderTargetSize != currentSize)
        {
            // Set render target size.
            m_d3dRenderTargetSize = currentSize;

            // A new depth stencil view is also needed.
            m_d3dDepthStencilView.Reset();
            m_d3dDepthStencilViewLeft.Reset();
            m_d3dDepthStencilViewRight.Reset();
        }
    }

    // Refresh depth stencil resources, if needed.
    if (m_d3dDepthStencilView == nullptr)
    {
        // Create a depth stencil view for use with 3D rendering if needed.
        CD3D11_TEXTURE2D_DESC depthStencilDesc(
            DXGI_FORMAT_D16_UNORM,
            static_cast<UINT>(m_d3dRenderTargetSize.Width),
            static_cast<UINT>(m_d3dRenderTargetSize.Height),
            m_isStereo ? 2 : 1, // Create two textures when rendering in stereo.
            1, // Use a single mipmap level.
            D3D11_BIND_DEPTH_STENCIL
            );

        ComPtr<ID3D11Texture2D> depthStencil;
		Kore::affirm(
            device->CreateTexture2D(
                &depthStencilDesc,
                nullptr,
                &depthStencil
                )
            );

        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(
            m_isStereo ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D
            );
		Kore::affirm(
            device->CreateDepthStencilView(
                depthStencil.Get(),
                &depthStencilViewDesc,
                &m_d3dDepthStencilView
                )
            );

		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
       m_d3dDepthStencilView->GetDesc(&desc);
	   std::cout << desc.Flags;

		//left and right
		CD3D11_DEPTH_STENCIL_VIEW_DESC leftDSVDesc;
		leftDSVDesc.Flags = 0;
		leftDSVDesc.Format= DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		leftDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		leftDSVDesc.Texture2DArray.ArraySize = 1;
		leftDSVDesc.Texture2DArray.MipSlice = 0;
		leftDSVDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, 0, 1);

		Kore::affirm(
			device->CreateDepthStencilView(
				depthStencil.Get(),
				&leftDSVDesc,
				&m_d3dDepthStencilViewLeft
			)
		);

		CD3D11_DEPTH_STENCIL_VIEW_DESC rightDSVDesc;
		rightDSVDesc.Flags = 0;
		rightDSVDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		rightDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		rightDSVDesc.Texture2DArray.ArraySize = 1;
		rightDSVDesc.Texture2DArray.MipSlice = 0;
		rightDSVDesc.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, 1, 1);

		Kore::affirm(
			device->CreateDepthStencilView(
				depthStencil.Get(),
				&rightDSVDesc,
				&m_d3dDepthStencilViewRight
			)
		);
    }

}

// Releases resources associated with a back buffer.
void DX::CameraResources::releaseResourcesForBackBuffer(DX::DeviceResources* pDeviceResources)
{
    const auto context = pDeviceResources->getD3DDeviceContext();

    // Release camera-specific resources.
    m_d3dBackBuffer.Reset();
    m_d3dRenderTargetView.Reset();
    m_d3dRenderTargetViewLeft.Reset();
    m_d3dRenderTargetViewRight.Reset();
    m_d3dDepthStencilView.Reset();
    m_d3dDepthStencilViewLeft.Reset();
    m_d3dDepthStencilViewRight.Reset();

    // Ensure system references to the back buffer are released by clearing the render
    // target from the graphics pipeline state, and then flushing the Direct3D context.
    ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
    context->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
    context->Flush();
}

#endif