//*********************************************************
// Original author Microsoft (license MIT)
// https://github.com/Microsoft/Windows-universal-samples/tree/master/Samples/HolographicSpatialMapping
// Code is adapted for Kore integration.
//*********************************************************


#pragma once

#include "CameraResources.winrt.h"
#include <wrl.h>
#include <dxgi1_5.h>
#include <d3d11_4.h>
#include <map>
#include <mutex>

namespace DX
{
    // Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
    interface IDeviceNotify
    {
        virtual void onDeviceLost()     = 0;
        virtual void onDeviceRestored() = 0;
    };

    // Creates and manages a Direct3D device and immediate context, Direct2D device and context (for debug), and the holographic swap chain.
    class DeviceResources
    {
    public:
        DeviceResources();
		void initWithDevice(Microsoft::WRL::ComPtr<ID3D11Device4> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext3>  context);
        // Public methods related to Direct3D devices.
        void handleDeviceLost();
        void registerDeviceNotify(IDeviceNotify* deviceNotify);
        void present(Windows::Graphics::Holographic::HolographicFrame^ frame);

        // Public methods related to holographic devices.
        void ensureCameraResources(
            Windows::Graphics::Holographic::HolographicFrame^ frame,
            Windows::Graphics::Holographic::HolographicFramePrediction^ prediction);

		void lockCameraResources();
		void unlockCameraResources();

		DX::CameraResources* getResourcesForCamera(Windows::Graphics::Holographic::HolographicCamera^ camera);
        void addHolographicCamera(Windows::Graphics::Holographic::HolographicCamera^ camera);
        void removeHolographicCamera(Windows::Graphics::Holographic::HolographicCamera^ camera);

        // Holographic accessors.
        template<typename RetType, typename LCallback>
        RetType                 useHolographicCameraResources(const LCallback& callback);

        Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice^
                                getD3DInteropDevice() const             { return m_d3dInteropDevice;    }

        // D3D accessors.
        ID3D11Device4*          getD3DDevice() const                    { return m_d3dDevice.Get();     }
        ID3D11DeviceContext3*   getD3DDeviceContext() const             { return m_d3dContext.Get();    }
        D3D_FEATURE_LEVEL       getDeviceFeatureLevel() const           { return m_d3dFeatureLevel;     }

    private:
        // Direct3D objects.
        Microsoft::WRL::ComPtr<ID3D11Device4>                   m_d3dDevice;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext3>            m_d3dContext;

        // Direct3D interop objects.
        Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice^ m_d3dInteropDevice;

        // Properties of the Direct3D device currently in use.
        D3D_FEATURE_LEVEL                                       m_d3dFeatureLevel = D3D_FEATURE_LEVEL_10_0;

        // The IDeviceNotify can be held directly as it owns the DeviceResources.
        IDeviceNotify*                                          m_deviceNotify = nullptr;

        // Back buffer resources, etc. for attached holographic cameras.
        std::map<UINT32, std::unique_ptr<CameraResources>>      m_cameraResources;
        std::mutex                                              m_cameraResourcesMutex;
		std::unique_lock<std::mutex>							m_cameraResLock;
    };
}

// Device-based resources for holographic cameras are stored in a std::map. Access this list by providing a
// callback to this function, and the std::map will be guarded from add and remove
// events until the callback returns. The callback is processed immediately and must
// not contain any nested calls to useHolographicCameraResources.
// The callback takes a parameter of type std::map<UINT32, std::unique_ptr<DX::CameraResources>>&
// through which the list of cameras will be accessed.
template<typename RetType, typename LCallback>
RetType DX::DeviceResources::useHolographicCameraResources(const LCallback& callback)
{
    std::lock_guard<std::mutex> guard(m_cameraResourcesMutex);
    return callback(m_cameraResources);
}

