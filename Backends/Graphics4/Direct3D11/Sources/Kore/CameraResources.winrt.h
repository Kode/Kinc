//*********************************************************
// Original author Microsoft (license MIT)
// https://github.com/Microsoft/Windows-universal-samples/tree/master/Samples/HolographicSpatialMapping
// Code is adapted for Kore integration.
//*********************************************************

#pragma once

#include <memory>
#include <d3d11_4.h>
#include <wrl.h>

namespace DX
{
    class DeviceResources;

    // Manages DirectX device resources that are specific to a holographic camera, such as the
    // back buffer, ViewProjection constant buffer, and viewport.
    class CameraResources
    {
    public:
        CameraResources(Windows::Graphics::Holographic::HolographicCamera^ holographicCamera);

        void createResourcesForBackBuffer(
            DX::DeviceResources* pDeviceResources,
            Windows::Graphics::Holographic::HolographicCameraRenderingParameters^ cameraParameters
            );
        void releaseResourcesForBackBuffer(
            DX::DeviceResources* pDeviceResources
            );

        // Direct3D device resources.
        //ID3D11RenderTargetView* GetBackBufferRenderTargetView()     const { return m_d3dRenderTargetView.Get();     }
        ID3D11RenderTargetView* getBackBufferRenderTargetViewLeft()     const { return m_d3dRenderTargetViewLeft.Get();     }
        ID3D11RenderTargetView* getBackBufferRenderTargetViewRight()     const { return m_d3dRenderTargetViewRight.Get();     }
        //ID3D11DepthStencilView* GetDepthStencilView()               const { return m_d3dDepthStencilView.Get();     }
        ID3D11DepthStencilView* getDepthStencilViewLeft()               const { return m_d3dDepthStencilViewLeft.Get();     }
        ID3D11DepthStencilView* getDepthStencilViewRight()               const { return m_d3dDepthStencilViewRight.Get();     }

        // Render target properties.
        bool                    isRenderingStereoscopic()           const { return m_isStereo;                      }

        // The holographic camera these resources are for.
        Windows::Graphics::Holographic::HolographicCamera^ getHolographicCamera() const { return m_holographicCamera; }

    private:
        // Direct3D rendering objects. Required for 3D.
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_d3dRenderTargetView;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_d3dRenderTargetViewLeft;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_d3dRenderTargetViewRight;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_d3dDepthStencilView;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_d3dDepthStencilViewLeft;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      m_d3dDepthStencilViewRight;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_d3dBackBuffer;

        // Direct3D rendering properties.
        DXGI_FORMAT                                         m_dxgiFormat;
        Windows::Foundation::Size                           m_d3dRenderTargetSize;
        D3D11_VIEWPORT                                      m_d3dViewport;

        // Indicates whether the camera supports stereoscopic rendering.
        bool                                                m_isStereo = false;

        // Pointer to the holographic camera these resources are for.
        Windows::Graphics::Holographic::HolographicCamera^  m_holographicCamera = nullptr;
    };
}
