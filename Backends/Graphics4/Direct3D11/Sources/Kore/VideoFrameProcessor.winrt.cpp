//*********************************************************
// Original author Microsoft (license MIT)
// https://github.com/Microsoft/Windows-universal-samples/tree/master/Samples/HolographicFaceTracking
// Code is adapted for Kore integration.
//*********************************************************


#include "pch.h"

#ifdef KORE_HOLOLENS 
#include "VideoFrameProcessor.winrt.h"

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation::Numerics;

using namespace Windows::Media::Capture;
using namespace Windows::Media::Capture::Frames;
using namespace Windows::Media::MediaProperties;
using namespace concurrency;
using namespace Platform;
using namespace std::placeholders;

VideoFrameProcessor::VideoFrameProcessor(Platform::Agile<MediaCapture> mediaCapture, MediaFrameReader^ reader, MediaFrameSource^ source)
    : m_mediaCapture(std::move(mediaCapture))
    , m_mediaFrameReader(std::move(reader))
    , m_mediaFrameSource(std::move(source))
{
    // Listen for new frames, so we know when to update our m_latestFrame
    m_mediaFrameReader->FrameArrived +=
        ref new TypedEventHandler<MediaFrameReader^, MediaFrameArrivedEventArgs^>(
            std::bind(&VideoFrameProcessor::onFrameArrived, this, _1, _2));
}

task<std::shared_ptr<VideoFrameProcessor>> VideoFrameProcessor::createAsync(void)
{
    return create_task(MediaFrameSourceGroup::FindAllAsync())
        .then([](IVectorView<MediaFrameSourceGroup^>^ groups)
    {
        MediaFrameSourceGroup^ selectedGroup = nullptr;
        MediaFrameSourceInfo^ selectedSourceInfo = nullptr;

        // Pick first color source.
        for (MediaFrameSourceGroup^ sourceGroup : groups)
        {
            for (MediaFrameSourceInfo^ sourceInfo : sourceGroup->SourceInfos)
            {
                if (sourceInfo->SourceKind == MediaFrameSourceKind::Color)
                {
                    selectedSourceInfo = sourceInfo;
                    break;
                }
            }

            if (selectedSourceInfo != nullptr)
            {
                selectedGroup = sourceGroup;
                break;
            }
        }

        // No valid camera was found. This will happen on the emulator.
        if (selectedGroup == nullptr || selectedSourceInfo == nullptr)
        {
            return task_from_result(std::shared_ptr<VideoFrameProcessor>(nullptr));
        }

        MediaCaptureInitializationSettings^ settings = ref new MediaCaptureInitializationSettings();
        settings->MemoryPreference = MediaCaptureMemoryPreference::Cpu; // Need SoftwareBitmaps for FaceAnalysis
        settings->StreamingCaptureMode = StreamingCaptureMode::Video;   // Only need to stream video
        settings->SourceGroup = selectedGroup;

        Platform::Agile<MediaCapture> mediaCapture(ref new MediaCapture());
        
		return create_task(mediaCapture->InitializeAsync(settings))
            .then([=]
        {
            MediaFrameSource^ selectedSource = mediaCapture->FrameSources->Lookup(selectedSourceInfo->Id);
			
            return create_task(mediaCapture->CreateFrameReaderAsync(selectedSource, MediaEncodingSubtypes::Bgra8)) //TODO mawe: format works on hololens?
                .then([=](MediaFrameReader^ reader)
            {
                return create_task(reader->StartAsync())
                    .then([=](MediaFrameReaderStartStatus status)
                {
                    // Only create a VideoFrameProcessor if the reader successfully started
                    if (status == MediaFrameReaderStartStatus::Success)
                    {
                        return std::make_shared<VideoFrameProcessor>(mediaCapture, reader, selectedSource);
                    }
                    else
                    {
                        return std::shared_ptr<VideoFrameProcessor>(nullptr);
                    }
                });
            });
        });
    });
}

Windows::Media::Capture::Frames::MediaFrameReference^ VideoFrameProcessor::getAndResetLatestFrame(void)
{
    auto lock = std::shared_lock<std::shared_mutex>(m_propertiesLock);
    auto theFrame= m_latestFrame;
	m_latestFrame = nullptr;
	return theFrame;

}

Windows::Media::Capture::Frames::VideoMediaFrameFormat^ VideoFrameProcessor::getCurrentFormat(void) const
{
    return m_mediaFrameSource->CurrentFormat->VideoFormat;
}

void VideoFrameProcessor::onFrameArrived(MediaFrameReader^ sender, MediaFrameArrivedEventArgs^ args)
{
    if (MediaFrameReference^ frame = sender->TryAcquireLatestFrame())
    {
        std::lock_guard<std::shared_mutex> lock(m_propertiesLock);
        m_latestFrame = frame;
    }
}



// Opens up the underlying buffer of a SoftwareBitmap and copies to our D3D11 Texture
bool VideoFrameProcessor::copyFromVideoMediaFrame(Windows::Media::Capture::Frames::VideoMediaFrame^ source, CameraImage* image)
{
	SoftwareBitmap^ softwareBitmap = source->SoftwareBitmap;

	if (softwareBitmap == nullptr)
	{
		Kore::log(Kore::LogLevel::Info,"SoftwareBitmap was null\r\n");
		return false;
	}

	if (softwareBitmap->BitmapPixelFormat != BitmapPixelFormat::Bgra8)
	{
		Kore::log(Kore::LogLevel::Info, "BitmapPixelFormat was not Bgra8\r\n");
		return false;
	}

	BitmapBuffer^ bitmapBuffer = softwareBitmap->LockBuffer(BitmapBufferAccessMode::Read);
	IMemoryBufferReference^ bufferRef = bitmapBuffer->CreateReference();

	ComPtr<IMemoryBufferByteAccess> memoryBufferByteAccess;
	if (SUCCEEDED(reinterpret_cast<IInspectable*>(bufferRef)->QueryInterface(IID_PPV_ARGS(&memoryBufferByteAccess))))
	{
		BYTE* pSourceBuffer = nullptr;
		UINT32 sourceCapacity = 0;
		if (SUCCEEDED(memoryBufferByteAccess->GetBuffer(&pSourceBuffer, &sourceCapacity)) && pSourceBuffer)
		{
			/*if (sourceCapacity != currentFrameDataSize) {
				currentFrameDataSize = sourceCapacity;
				delete[] currentFrameData;
				currentFrameData = new int[currentFrameDataSize/4];
			}*/

			//std::memcpy(currentFrameData, pSourceBuffer, sourceCapacity);
			image->imageBGRA8Data = new int[sourceCapacity / 4];
			std::memcpy(image->imageBGRA8Data, pSourceBuffer, sourceCapacity);
			return true;
		}
	}
	return false;
}

CameraImage* VideoFrameProcessor::getCurrentCameraImage(SpatialCoordinateSystem^ worldCoordSystem) {
	MediaFrameReference^ frame = getAndResetLatestFrame();

	if (frame  == nullptr) {
		return NULL;
	}

	VideoMediaFrame^ videoMediaFrame = frame->VideoMediaFrame;
	if (videoMediaFrame == nullptr) {
		return NULL;
	}

	//videoMediaFrame->CameraIntrinsics->
	auto format = videoMediaFrame->VideoFormat;


	if (!frame->Properties->HasKey(MFSampleExtension_Spatial_CameraProjectionTransform)) {
		return NULL;
	}
	auto projectionTransformProperty= (Windows::Foundation::IPropertyValue^) frame->Properties->Lookup(MFSampleExtension_Spatial_CameraProjectionTransform);
	Platform::Array<unsigned char>^ projectionMatrixByteArray = ref new Platform::Array<unsigned char>(4*4*4);
	projectionTransformProperty->GetUInt8Array(&projectionMatrixByteArray);
	float* vals = reinterpret_cast<float*>(projectionMatrixByteArray->Data);
	Kore::mat4 projectionMat;
	for(int i=0;i<16;i++){
		projectionMat.data[i] = vals[i];
	}

	SpatialCoordinateSystem^ cameraCoordinateSystem = frame->CoordinateSystem;
	if (cameraCoordinateSystem == nullptr) {
		return NULL;
	}
	CameraIntrinsics^ cameraIntrinsics = videoMediaFrame->CameraIntrinsics;
	IBox<float4x4>^ cameraToWorld = cameraCoordinateSystem->TryGetTransformTo(worldCoordSystem);

	// If we can't locate the world, this transform will be null.
	if (cameraToWorld == nullptr)
	{
		return NULL;
	}

	Kore::vec2 focalLength = Kore::vec2(cameraIntrinsics->FocalLength.x, cameraIntrinsics->FocalLength.x);
	
	//todo create cameraImage from frame..
	CameraImage* cameraImage = new CameraImage(format->Width, format->Height, nullptr, WindowsNumericsToKoreMat(cameraToWorld->Value), projectionMat, focalLength);

	if (!copyFromVideoMediaFrame(videoMediaFrame,cameraImage)) {
		delete cameraImage;
		return NULL;
	}



	return cameraImage;
}

#endif