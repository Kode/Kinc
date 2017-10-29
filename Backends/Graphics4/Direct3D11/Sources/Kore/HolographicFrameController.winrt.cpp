//*********************************************************
// Original author Microsoft (license MIT)
// https://github.com/Microsoft/Windows-universal-samples/tree/master/Samples/HolographicSpatialMapping
// Code is adapted for Kore integration.
//*********************************************************

#include "pch.h"

#ifdef KORE_HOLOLENS 

#include "HolographicFrameController.winrt.h"
#include <DirectXColors.h>
#include <Kore/WinError.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <Collection.h>
#include <ppltasks.h>    // For create_task
#include <Kore/Graphics4/Graphics.h>
#include <wrl.h>
#include "Conversion.winrt.h"
#include "HolographicFrameController.winrt.h"
using namespace Microsoft::WRL;
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
using namespace Kore;

std::unique_ptr<HolographicFrameController> holographicFrameController;
std::shared_ptr<VideoFrameProcessor> videoFrameProcessor;

HolographicFrameController::HolographicFrameController(Windows::UI::Core::CoreWindow^ window)
{
	createHolographicSpace(window);
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	// Register to be notified if the device is lost or recreated.
	m_deviceResources->registerDeviceNotify(this);
}

void HolographicFrameController::createHolographicSpace(Windows::UI::Core::CoreWindow^ window)
{
	unregisterHolographicEventHandlers();

	m_holographicSpace = HolographicSpace::CreateForCoreWindow(window);
	registerCameraEvents();


	setupSpatialLocator();
}

void HolographicFrameController::registerCameraEvents()
{
	m_cameraAddedToken =
		m_holographicSpace->CameraAdded +=
		ref new Windows::Foundation::TypedEventHandler<HolographicSpace^, HolographicSpaceCameraAddedEventArgs^>(
			std::bind(&HolographicFrameController::onCameraAdded, this, _1, _2)
			);
	m_cameraRemovedToken =
		m_holographicSpace->CameraRemoved +=
		ref new Windows::Foundation::TypedEventHandler<HolographicSpace^, HolographicSpaceCameraRemovedEventArgs^>(
			std::bind(&HolographicFrameController::onCameraRemoved, this, _1, _2)
			);
}

void HolographicFrameController::setupSpatialLocator()
{
	// Use the default SpatialLocator to track the motion of the device.
	m_spatialLocator = SpatialLocator::GetDefault();
	m_positionalTrackingDeactivatingToken =
		m_spatialLocator->PositionalTrackingDeactivating +=
		ref new Windows::Foundation::TypedEventHandler<SpatialLocator^, SpatialLocatorPositionalTrackingDeactivatingEventArgs^>(
			std::bind(&HolographicFrameController::onPositionalTrackingDeactivating, this, _1, _2)
			);

	m_positionalTrackingLocatabilityChangedToken =
		m_spatialLocator->LocatabilityChanged +=
		ref new Windows::Foundation::TypedEventHandler<SpatialLocator^, Object^>(
			std::bind(&HolographicFrameController::onPositionalTrackingLocatabilityChanged, this, _1, _2)
			);

	m_referenceFrame = m_spatialLocator->CreateStationaryFrameOfReferenceAtCurrentLocation();
}

Windows::Perception::Spatial::SpatialCoordinateSystem^ HolographicFrameController::getCurrentWorldCoordinateSystem() {
	return m_currentCoordinateSystem;
}

void HolographicFrameController::begin()
{
	m_currentHolographicFrame = createNextHolographicFrame();
	m_deviceResources->lockCameraResources();

	m_currentHolographicFrame->UpdateCurrentPrediction();

	m_currentPrediction = m_currentHolographicFrame->CurrentPrediction;
	m_currentCoordinateSystem = m_referenceFrame->CoordinateSystem;

	m_currentCamPose = m_currentPrediction->CameraPoses->GetAt(0);
	m_currentCameraResources = m_deviceResources->getResourcesForCamera(m_currentCamPose->HolographicCamera);
}

void HolographicFrameController::beginRender(int eye)
{
	// The system changes the viewport on a per-frame basis for system optimizations. [can be done in begin() ?]
	auto m_d3dViewport = CD3D11_VIEWPORT(
		m_currentCamPose->Viewport.Left,
		m_currentCamPose->Viewport.Top,
		m_currentCamPose->Viewport.Width,
		m_currentCamPose->Viewport.Height
	);
	const auto context = m_deviceResources->getD3DDeviceContext();
	context->RSSetViewports(1, &m_d3dViewport);

	ID3D11DepthStencilView* depthStencilView;
	ID3D11RenderTargetView* targets[1];
	if (eye == 0) {
		depthStencilView = m_currentCameraResources->getDepthStencilViewLeft();
		targets[0] = m_currentCameraResources->getBackBufferRenderTargetViewLeft();
	}
	else {
		depthStencilView = m_currentCameraResources->getDepthStencilViewRight();
		targets[0] = m_currentCameraResources->getBackBufferRenderTargetViewRight();
	}
	//Kore::Graphics4::setRenderTargets(RenderTarget** targets, int count)
	context->OMSetRenderTargets(1, targets, depthStencilView);
	context->ClearRenderTargetView(targets[0], DirectX::Colors::Transparent);
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

SensorState HolographicFrameController::getSensorState(int eye)
{
	SensorState state;
	// The projection transform for each frame is provided by the HolographicCameraPose.
	HolographicStereoTransform cameraProjectionTransform = m_currentCamPose->ProjectionTransform;

	Platform::IBox<HolographicStereoTransform>^ viewTransformContainer = m_currentCamPose->TryGetViewTransform(m_currentCoordinateSystem);
	HolographicStereoTransform viewCoordinateSystemTransform;
	
	if (viewTransformContainer != nullptr)
	{
		viewCoordinateSystemTransform = viewTransformContainer->Value;
	}

	if (eye == 0) {
		state.pose.vrPose.projection = WindowsNumericsToKoreMat(cameraProjectionTransform.Left);
		state.pose.vrPose.eye = WindowsNumericsToKoreMat(viewCoordinateSystemTransform.Left);
	}
	else
	{
		state.pose.vrPose.projection = WindowsNumericsToKoreMat(cameraProjectionTransform.Right);
		state.pose.vrPose.eye = WindowsNumericsToKoreMat(viewCoordinateSystemTransform.Right);
	}
	return state;
}

void HolographicFrameController::endRender(int eye)
{
}

void HolographicFrameController::warpSwap()
{
	m_deviceResources->unlockCameraResources();
	m_deviceResources->present(m_currentHolographicFrame);

}

void HolographicFrameController::setDeviceAndContext(Microsoft::WRL::ComPtr<ID3D11Device4> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext3>  context)
{
	m_deviceResources->initWithDevice(device, context);
	m_holographicSpace->SetDirect3D11Device(m_deviceResources->getD3DInteropDevice());
}



Microsoft::WRL::ComPtr<IDXGIAdapter3> HolographicFrameController::getCompatibleDxgiAdapter()
{
	Microsoft::WRL::ComPtr<IDXGIAdapter3> theDxgiAdapter;
	// The holographic space might need to determine which adapter supports
	// holograms, in which case it will specify a non-zero PrimaryAdapterId.
	LUID id =
	{
		m_holographicSpace->PrimaryAdapterId.LowPart,
		m_holographicSpace->PrimaryAdapterId.HighPart
	};

	// When a primary adapter ID is given to the app, the app should find
	// the corresponding DXGI adapter and use it to create Direct3D devices
	// and device contexts. Otherwise, there is no restriction on the DXGI
	// adapter the app can use.
	if ((id.HighPart != 0) && (id.LowPart != 0))
	{
		UINT createFlags = 0;
#ifdef DEBUG
		if (DX::SdkLayersAvailable())
		{
			createFlags |= DXGI_CREATE_FACTORY_DEBUG;
}
#endif
		// Create the DXGI factory.
		ComPtr<IDXGIFactory1> dxgiFactory;
		affirm(
			CreateDXGIFactory2(
				createFlags,
				IID_PPV_ARGS(&dxgiFactory)
			)
		);
		ComPtr<IDXGIFactory4> dxgiFactory4;
		affirm(dxgiFactory.As(&dxgiFactory4));

		// Retrieve the adapter specified by the holographic space.
		affirm(
			dxgiFactory4->EnumAdapterByLuid(
				id,
				IID_PPV_ARGS(&theDxgiAdapter)
			)
		);
	}
	return theDxgiAdapter;

}


void HolographicFrameController::onPositionalTrackingDeactivating(
	SpatialLocator^ sender,
	SpatialLocatorPositionalTrackingDeactivatingEventArgs^ args)
{
	// Without positional tracking, spatial meshes will not be locatable.
	args->Canceled = true;
}

void HolographicFrameController::onPositionalTrackingLocatabilityChanged(
	SpatialLocator^ sender,
	Object^ args)
{
	//auto locatability = sender->Locatability;
	//String^ message = L"Warning! Positional tracking is " +
	//	sender->Locatability.ToString() + L".\n";
	//OutputDebugStringW(message->Data());
}


void HolographicFrameController::unregisterHolographicEventHandlers()
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

	if (m_spatialLocator != nullptr)
	{
		m_spatialLocator->PositionalTrackingDeactivating -= m_positionalTrackingDeactivatingToken;
		m_spatialLocator->LocatabilityChanged -= m_positionalTrackingLocatabilityChangedToken;
	}
}

HolographicFrameController::~HolographicFrameController()
{
	// Deregister device notification.
	m_deviceResources->registerDeviceNotify(nullptr);
	unregisterHolographicEventHandlers();
}


// Updates the application state once per frame.
HolographicFrame^ HolographicFrameController::createNextHolographicFrame()
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
	m_deviceResources->ensureCameraResources(holographicFrame, prediction);

	// The holographic frame will be used to get up-to-date view and projection matrices and
	// to present the swap chain.
	return holographicFrame;
}


// Notifies classes that use Direct3D device resources that the device resources
// need to be released before this method returns.
void HolographicFrameController::onDeviceLost()
{
	Kore::error("device lost");
	//release device dependent resources
}

// Notifies classes that use Direct3D device resources that the device resources
// may now be recreated.
void HolographicFrameController::onDeviceRestored()
{
	//recreate device dependent resources
}

void HolographicFrameController::onCameraAdded(
	HolographicSpace^ sender,
	HolographicSpaceCameraAddedEventArgs^ args)
{
	Deferral^ deferral = args->GetDeferral();
	HolographicCamera^ holographicCamera = args->Camera;
	create_task([this, deferral, holographicCamera]()
	{
		// Create device-based resources for the holographic camera and add it to the list of
		// cameras used for updates and rendering. Notes:
		//   * Since this function may be called at any time, the addHolographicCamera function
		//     waits until it can get a lock on the set of holographic camera resources before
		//     adding the new camera. At 60 frames per second this wait should not take long.
		//   * A subsequent Update will take the back buffer from the RenderingParameters of this
		//     camera's CameraPose and use it to create the ID3D11RenderTargetView for this camera.
		//     Content can then be rendered for the HolographicCamera.
		m_deviceResources->addHolographicCamera(holographicCamera);

		// Holographic frame predictions will not include any information about this camera until
		// the deferral is completed.
		deferral->Complete();
	});
}

void HolographicFrameController::onCameraRemoved(
	HolographicSpace^ sender,
	HolographicSpaceCameraRemovedEventArgs^ args)
{
	// Before letting this callback return, ensure that all references to the back buffer
	// are released.
	// Since this function may be called at any time, the removeHolographicCamera function
	// waits until it can get a lock on the set of holographic camera resources before
	// deallocating resources for this camera. At 60 frames per second this wait should
	// not take long.
	m_deviceResources->removeHolographicCamera(args->Camera);
}

#endif