#include <kinc/video.h>

#ifdef KORE_DIRECT3D12

/*#include <mfapi.h>
#include <mfd3d12.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>*/

#include <d3d12video.h>
#include <malloc.h>

#include <assert.h>

extern "C" ID3D12Device *device;

#if 0
void kinc_video_init(kinc_video_t *video, const char *filename) {}

void kinc_video_destroy(kinc_video_t *video) {}

kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video) {
	return NULL;
}

int kinc_video_width(kinc_video_t *video) {
	return 64;
}

int kinc_video_height(kinc_video_t *video) {
	return 64;
}

void kinc_video_play(kinc_video_t *video, bool loop) {
	MFStartup(MF_VERSION, MFSTARTUP_FULL);

	UINT resetToken;
	IMFDXGIDeviceManager *dxgiDeviceManager;
	MFCreateDXGIDeviceManager(&resetToken, &dxgiDeviceManager);

	dxgiDeviceManager->ResetDevice(device, resetToken);

	IMFSourceReader *sourceReader;
	{
		IMFAttributes *creationAttributes;
		MFCreateAttributes(&creationAttributes, 1);
		creationAttributes->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, dxgiDeviceManager);
		// creationAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);

		IMFByteStream *stream;
		MFCreateFile(MF_FILE_ACCESSMODE::MF_ACCESSMODE_READ, MF_FILE_OPENMODE::MF_OPENMODE_FAIL_IF_NOT_EXIST, MF_FILE_FLAGS::MF_FILEFLAGS_NONE, L"test.mp4",
		             &stream);
		MFCreateSourceReaderFromByteStream(stream, creationAttributes, &sourceReader);
	}

	{
		IMFMediaType *pVideoMediaType;
		HRESULT hr = MFCreateMediaType(&pVideoMediaType);

		hr = pVideoMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		hr = pVideoMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
		// hr = pVideoMediaType->SetUINT32(MF_MT_D3D_RESOURCE_VERSION, MF_D3D12_RESOURCE);
		// hr = pVideoMediaType->SetUINT32(MF_MT_D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS, TRUE);

		hr = sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pVideoMediaType);
		assert(SUCCEEDED(hr));
	}

	IMFTransform *transform;
	{
		IMFSourceReaderEx *sourceReaderEx = (IMFSourceReaderEx *)sourceReader;
		GUID cat = MFT_CATEGORY_VIDEO_DECODER;

		HRESULT hr;
		hr = sourceReaderEx->GetTransformForStream(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &cat, &transform);

		hr = transform->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, (ULONG_PTR)dxgiDeviceManager);
	}

	HRESULT hr = sourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
	hr = sourceReader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);

	size_t decodedFrames = 0;
	IMFD3D12SynchronizationObjectCommands *pMFSyncCmd;
	IMFD3D12SynchronizationObject *pMFSyncObjs;
	HANDLE sampleResourceReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE sampleResourceFinalRelease = CreateEvent(NULL, TRUE, FALSE, NULL);
	while (true) {
		DWORD readFlags = decodedFrames == 0 ? 0 : MF_SOURCE_READER_CONTROLF_DRAIN;
		DWORD dwActualStreamIndex;
		DWORD dwStreamFlags;
		LONGLONG llTimestamp;
		IMFSample *sample;
		hr = sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, readFlags, &dwActualStreamIndex, &dwStreamFlags, &llTimestamp, &sample);
		assert(SUCCEEDED(hr));

		if (decodedFrames++ == 0) {
		}

		if (!sample)
			break; // finished draining

		DWORD bufferCount;
		sample->GetBufferCount(&bufferCount);

		IMFMediaBuffer *output_media_buffer;
		hr = sample->GetBufferByIndex(0, &output_media_buffer);

		IMFDXGIBuffer *spDXGIBuffer = (IMFDXGIBuffer *)output_media_buffer;
		if (SUCCEEDED(hr)) {
			ID3D12Resource *texture;
			hr = spDXGIBuffer->GetResource(IID_PPV_ARGS(&texture));
			assert(SUCCEEDED(hr));
			texture->SetName(L"MFT texture");

			hr = spDXGIBuffer->GetUnknown(MF_D3D12_SYNCHRONIZATION_OBJECT, IID_PPV_ARGS(&pMFSyncCmd));
			assert(SUCCEEDED(hr));
			hr = spDXGIBuffer->GetUnknown(MF_D3D12_SYNCHRONIZATION_OBJECT, IID_PPV_ARGS(&pMFSyncObjs));
			assert(SUCCEEDED(hr));

			pMFSyncCmd->SignalEventOnResourceReady(sampleResourceReady);
		}
	}
}

void kinc_video_pause(kinc_video_t *video) {}

void kinc_video_stop(kinc_video_t *video) {}

void kinc_video_update(kinc_video_t *video, double time) {}

double kinc_video_duration(kinc_video_t *video) {
	return 0.0;
}

double kinc_video_position(kinc_video_t *video) {
	return 0.0;
}

bool kinc_video_finished(kinc_video_t *video) {
	return true;
}

bool kinc_video_paused(kinc_video_t *video) {
	return true;
}
#endif

void kinc_video_init(kinc_video_t *video, const char *filename) {}

void kinc_video_destroy(kinc_video_t *video) {}

kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video) {
	return NULL;
}

int kinc_video_width(kinc_video_t *video) {
	return 64;
}

int kinc_video_height(kinc_video_t *video) {
	return 64;
}

void kinc_video_play(kinc_video_t *video, bool loop) {
	ID3D12VideoDevice *video_device = NULL;
	HRESULT result = device->QueryInterface(IID_PPV_ARGS(&video_device));

	D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILE_COUNT decode_profile_count = {};
	decode_profile_count.NodeIndex = 0;
	result = video_device->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_PROFILE_COUNT, &decode_profile_count, sizeof(decode_profile_count));

	GUID *guids = (GUID *)_alloca(sizeof(GUID) * decode_profile_count.ProfileCount);
	D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILES profiles = {};
	profiles.NodeIndex = 0;
	profiles.ProfileCount = decode_profile_count.ProfileCount;
	profiles.pProfiles = guids;
	result = video_device->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_PROFILES, &profiles, sizeof(profiles));

	// for (UINT i = 0; i < decode_profile_count.ProfileCount; ++i) {
	UINT i = 0;
	D3D12_VIDEO_DECODE_CONFIGURATION decode_config = {guids[i], D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE, D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE};

	D3D12_FEATURE_DATA_VIDEO_DECODE_FORMAT_COUNT formats_count;
	formats_count.NodeIndex = 0;
	formats_count.Configuration = decode_config;
	result = video_device->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_FORMAT_COUNT, &formats_count, sizeof(formats_count));

	D3D12_FEATURE_DATA_VIDEO_DECODE_FORMATS formats;
	formats.NodeIndex = 0;
	formats.Configuration = decode_config;
	formats.FormatCount = formats_count.FormatCount;
	formats.pOutputFormats = (DXGI_FORMAT *)_alloca(sizeof(DXGI_FORMAT) * formats_count.FormatCount);
	result = video_device->CheckFeatureSupport(D3D12_FEATURE_VIDEO_DECODE_FORMATS, &formats, sizeof(formats));
	//}

	ID3D12VideoDecoder *decoder;
	D3D12_VIDEO_DECODER_DESC video_decoder_desc;
	video_decoder_desc.NodeMask = 0;
	video_decoder_desc.Configuration = decode_config;
	result = video_device->CreateVideoDecoder(&video_decoder_desc, IID_PPV_ARGS(&decoder));

	ID3D12VideoDecoderHeap *decoder_heap;
	D3D12_VIDEO_DECODER_HEAP_DESC video_decoder_heap_desc;
	video_decoder_heap_desc.NodeMask = 0;
	video_decoder_heap_desc.BitRate = 0;
	video_decoder_heap_desc.Configuration = decode_config;
	video_decoder_heap_desc.DecodeWidth = 1280;
	video_decoder_heap_desc.DecodeHeight = 720;
	video_decoder_heap_desc.Format = formats.pOutputFormats[0];
	video_decoder_heap_desc.FrameRate.Numerator = 0;
	video_decoder_heap_desc.FrameRate.Denominator = 1;
	video_decoder_heap_desc.MaxDecodePictureBufferCount = 2;
	result = video_device->CreateVideoDecoderHeap(&video_decoder_heap_desc, IID_PPV_ARGS(&decoder_heap));

	ID3D12CommandAllocator *command_allocator;
	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE, IID_PPV_ARGS(&command_allocator));

	ID3D12VideoDecodeCommandList *command_list;
	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE, command_allocator, NULL, IID_PPV_ARGS(&command_list));

	D3D12_VIDEO_DECODE_INPUT_STREAM_ARGUMENTS input_arguments;
	input_arguments.CompressedBitstream.pBuffer;
	input_arguments.CompressedBitstream.Size;
	input_arguments.CompressedBitstream.Offset;
	input_arguments.FrameArguments[0].pData;
	input_arguments.FrameArguments[0].Size;
	input_arguments.FrameArguments[0].Type;
	input_arguments.NumFrameArguments = 1;
	input_arguments.pHeap = decoder_heap;
	input_arguments.ReferenceFrames;

	D3D12_VIDEO_DECODE_OUTPUT_STREAM_ARGUMENTS output_arguments;
	output_arguments.ConversionArguments.Enable = FALSE;
	output_arguments.OutputSubresource = 0;
	output_arguments.pOutputTexture2D;

	command_list->DecodeFrame(decoder, &output_arguments, &input_arguments);

	int a = 3;
	++a;
}

void kinc_video_pause(kinc_video_t *video) {}

void kinc_video_stop(kinc_video_t *video) {}

void kinc_video_update(kinc_video_t *video, double time) {}

double kinc_video_duration(kinc_video_t *video) {
	return 0.0;
}

double kinc_video_position(kinc_video_t *video) {
	return 0.0;
}

bool kinc_video_finished(kinc_video_t *video) {
	return true;
}

bool kinc_video_paused(kinc_video_t *video) {
	return true;
}

#else

#ifndef KINC_NO_DIRECTSHOW

#include <streams.h>

namespace {
	IGraphBuilder *graphBuilder;
	IMediaControl *mediaControl;
	IMediaPosition *mediaPosition;
	IMediaEvent *mediaEvent;

	struct __declspec(uuid("{71771540-2017-11cf-ae24-0020afd79767}")) CLSID_TextureRenderer;
}

class CTextureRenderer : public CBaseVideoRenderer {
public:
	CTextureRenderer(LPUNKNOWN pUnk, HRESULT *phr);
	~CTextureRenderer();

public:
	HRESULT CheckMediaType(const CMediaType *pmt);      // Format acceptable?
	HRESULT SetMediaType(const CMediaType *pmt);        // Video format notification
	HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample

	// BOOL m_bUseDynamicTextures;
	// LONG m_lVidWidth;   // Video width
	// LONG m_lVidHeight;  // Video Height
	// LONG m_lVidPitch;   // Video Pitch

	kinc_g4_texture_t image;
	int width;
	int height;
	uint8_t *pixels;
};

CTextureRenderer::CTextureRenderer(LPUNKNOWN pUnk, HRESULT *phr) : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), TEXT("Texture Renderer"), pUnk, phr) {
	// Store and AddRef the texture for our use.
	ASSERT(phr);
	if (phr)
		*phr = S_OK;
}

CTextureRenderer::~CTextureRenderer() {
	// Do nothing
}

HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt) {
	HRESULT hr = E_FAIL;
	VIDEOINFO *pvi = 0;

	CheckPointer(pmt, E_POINTER);

	// Reject the connection if this is not a video type
	if (*pmt->FormatType() != FORMAT_VideoInfo) {
		return E_INVALIDARG;
	}

	// Only accept RGB24 video
	pvi = (VIDEOINFO *)pmt->Format();

	if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Video) && IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB24)) {
		hr = S_OK;
	}

	return hr;
}

HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt) {
	VIDEOINFO *info = (VIDEOINFO *)pmt->Format();
	width = info->bmiHeader.biWidth;
	height = abs(info->bmiHeader.biHeight);
	kinc_g4_texture_init(&image, width, height, KINC_IMAGE_FORMAT_RGBA32);
	pixels = (uint8_t *)malloc(width * height * 3);

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			pixels[y * width * 3 + x * 3 + 0] = 0;
			pixels[y * width * 3 + x * 3 + 1] = 0;
			pixels[y * width * 3 + x * 3 + 2] = 0;
		}
	}

	return S_OK;
}

HRESULT CTextureRenderer::DoRenderSample(IMediaSample *sample) {
	BYTE *videoPixels;
	sample->GetPointer(&videoPixels);
	int videoPitch = (width * 3 + 3) & ~(3);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			pixels[y * width * 3 + x * 3 + 0] = videoPixels[(height - y - 1) * videoPitch + x * 3 + 2];
			pixels[y * width * 3 + x * 3 + 1] = videoPixels[(height - y - 1) * videoPitch + x * 3 + 1];
			pixels[y * width * 3 + x * 3 + 2] = videoPixels[(height - y - 1) * videoPitch + x * 3 + 0];
		}
	}
	return S_OK;
}

void kinc_video_init(kinc_video_t *video, const char *filename) {
	video->impl.duration = 1000 * 10;
	video->impl.position = 0;
	video->impl.finished = false;
	video->impl.paused = false;
	// image = new Graphics4::Texture(100, 100, Graphics4::Image::RGBA32, false);

	HRESULT hr = S_OK;
	IBaseFilter *pFSrc; // Source Filter
	IPin *pFSrcPinOut;  // Source Filter Output Pin

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, __uuidof(IGraphBuilder), (void **)&graphBuilder);
	video->impl.renderer = new CTextureRenderer(NULL, &hr);
	hr = graphBuilder->AddFilter((CTextureRenderer *)video->impl.renderer, L"TEXTURERENDERER");
	wchar_t wideFilename[2048];
	mbstowcs(wideFilename, filename, 2048 - 1);
	hr = graphBuilder->AddSourceFilter(wideFilename, L"SOURCE", &pFSrc);
	hr = pFSrc->FindPin(L"Output", &pFSrcPinOut);
	hr = graphBuilder->Render(pFSrcPinOut);

	graphBuilder->QueryInterface(&mediaControl);
	graphBuilder->QueryInterface(&mediaPosition);
	graphBuilder->QueryInterface(&mediaEvent);

	mediaPosition->get_Duration(&video->impl.duration);
	video->impl.position = 0;
}

void kinc_video_destroy(kinc_video_t *video) {}

kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video) {
	CTextureRenderer *renderer = (CTextureRenderer *)video->impl.renderer;
	uint8_t *pixels = kinc_g4_texture_lock(&renderer->image);
	int stride = kinc_g4_texture_stride(&renderer->image);
	for (int y = 0; y < renderer->height; ++y) {
		for (int x = 0; x < renderer->width; ++x) {
			pixels[y * stride + x * 4 + 0] = renderer->pixels[y * renderer->width * 3 + x * 3 + 0];
			pixels[y * stride + x * 4 + 1] = renderer->pixels[y * renderer->width * 3 + x * 3 + 1];
			pixels[y * stride + x * 4 + 2] = renderer->pixels[y * renderer->width * 3 + x * 3 + 2];
			pixels[y * stride + x * 4 + 3] = 255;
		}
	}
	kinc_g4_texture_unlock(&renderer->image);

	mediaPosition->get_CurrentPosition(&video->impl.position);

	return &renderer->image;
}

int kinc_video_width(kinc_video_t *video) {
	CTextureRenderer *renderer = (CTextureRenderer *)video->impl.renderer;
	return renderer->width;
}

int kinc_video_height(kinc_video_t *video) {
	CTextureRenderer *renderer = (CTextureRenderer *)video->impl.renderer;
	return renderer->height;
}

void kinc_video_play(kinc_video_t *video, bool loop) {
	mediaControl->Run();
}

void kinc_video_pause(kinc_video_t *video) {
	mediaControl->Pause();
}

void kinc_video_stop(kinc_video_t *video) {
	mediaControl->Stop();
}

void kinc_video_update(kinc_video_t *video, double time) {
	mediaPosition->put_CurrentPosition(time);
}

double kinc_video_duration(kinc_video_t *video) {
	return video->impl.duration;
}

double kinc_video_position(kinc_video_t *video) {
	return video->impl.position;
}

bool kinc_video_finished(kinc_video_t *video) {
	return video->impl.finished;
}

bool kinc_video_paused(kinc_video_t *video) {
	return video->impl.paused;
}

#else

void kinc_video_init(kinc_video_t *video, const char *filename) {}

void kinc_video_destroy(kinc_video_t *video) {}

kinc_g4_texture_t *kinc_video_current_image(kinc_video_t *video) {
	return NULL;
}

int kinc_video_width(kinc_video_t *video) {
	return 64;
}

int kinc_video_height(kinc_video_t *video) {
	return 64;
}

void kinc_video_play(kinc_video_t *video, bool loop) {}

void kinc_video_pause(kinc_video_t *video) {}

void kinc_video_stop(kinc_video_t *video) {}

void kinc_video_update(kinc_video_t *video, double time) {}

double kinc_video_duration(kinc_video_t *video) {
	return 0.0;
}

double kinc_video_position(kinc_video_t *video) {
	return 0.0;
}

bool kinc_video_finished(kinc_video_t *video) {
	return true;
}

bool kinc_video_paused(kinc_video_t *video) {
	return true;
}

#endif

#endif

void kinc_internal_video_sound_stream_init(kinc_internal_video_sound_stream_t *stream, int channel_count, int frequency) {}

void kinc_internal_video_sound_stream_destroy(kinc_internal_video_sound_stream_t *stream) {}

void kinc_internal_video_sound_stream_insert_data(kinc_internal_video_sound_stream_t *stream, float *data, int sample_count) {}

float kinc_internal_video_sound_stream_next_sample(kinc_internal_video_sound_stream_t *stream) {
	return 0.0f;
}

bool kinc_internal_video_sound_stream_ended(kinc_internal_video_sound_stream_t *stream) {
	return true;
}
