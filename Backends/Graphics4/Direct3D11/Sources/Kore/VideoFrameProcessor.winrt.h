//*********************************************************
// Original author Microsoft (license MIT)
// https://github.com/Microsoft/Windows-universal-samples/tree/master/Samples/HolographicFaceTracking
// Code is adapted for Kore integration.
//*********************************************************

#define NOMINMAX
#pragma once
#include <concrt.h>
#include <ppltasks.h>
#include <mutex>
#include <wrl.h>
#include <shared_mutex>
#include <collection.h>
#include <MemoryBuffer.h> // IMemoryBufferByteAccess
#include <Kore/Vr/CameraImage.h>
#include "Conversion.winrt.h"
#include <Kore/Log.h>

using namespace Windows::Perception::Spatial;
using namespace Windows::UI::Input::Spatial;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Imaging;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace Windows::Media::Devices::Core;

EXTERN_GUID(MFSampleExtension_Spatial_CameraViewTransform, 0x4e251fa4, 0x830f, 0x4770, 0x85, 0x9a, 0x4b, 0x8d, 0x99, 0xaa, 0x80, 0x9b);
EXTERN_GUID(MFSampleExtension_Spatial_CameraCoordinateSystem, 0x9d13c82f, 0x2199, 0x4e67, 0x91, 0xcd, 0xd1, 0xa4, 0x18, 0x1f, 0x25, 0x34);
EXTERN_GUID(MFSampleExtension_Spatial_CameraProjectionTransform, 0x47f9fcb5, 0x2a02, 0x4f26, 0xa4, 0x77, 0x79, 0x2f, 0xdf, 0x95, 0x88, 0x6a);

// Class to manage receiving video frames from Windows::Media::Capture
class VideoFrameProcessor
{
public:
	static Concurrency::task<std::shared_ptr<VideoFrameProcessor>> createAsync(void);

	VideoFrameProcessor(
		Platform::Agile<Windows::Media::Capture::MediaCapture> mediaCapture,
		Windows::Media::Capture::Frames::MediaFrameReader^ reader,
		Windows::Media::Capture::Frames::MediaFrameSource^ source);

	CameraImage* VideoFrameProcessor::getCurrentCameraImage(SpatialCoordinateSystem^ worldCoordSystem);

protected:
	void onFrameArrived(
		Windows::Media::Capture::Frames::MediaFrameReader^ sender,
		Windows::Media::Capture::Frames::MediaFrameArrivedEventArgs^ args);

	Platform::Agile<Windows::Media::Capture::MediaCapture> m_mediaCapture;
	Windows::Media::Capture::Frames::MediaFrameReader^     m_mediaFrameReader;

	mutable std::shared_mutex                              m_propertiesLock;
	Windows::Media::Capture::Frames::MediaFrameSource^     m_mediaFrameSource;
	Windows::Media::Capture::Frames::MediaFrameReference^  m_latestFrame;
private:
	Windows::Media::Capture::Frames::MediaFrameReference^ getAndResetLatestFrame(void);
	Windows::Media::Capture::Frames::VideoMediaFrameFormat^ getCurrentFormat(void) const;
	bool copyFromVideoMediaFrame(Windows::Media::Capture::Frames::VideoMediaFrame^ source, CameraImage* image);
};