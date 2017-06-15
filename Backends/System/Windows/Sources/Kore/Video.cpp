#include "pch.h"

#include "Video.h"

#include <streams.h>

using namespace Kore;

namespace {
	IGraphBuilder* graphBuilder;
	IMediaControl* mediaControl;
	IMediaPosition* mediaPosition;
	IMediaEvent* mediaEvent;

	struct __declspec(uuid("{71771540-2017-11cf-ae24-0020afd79767}")) CLSID_TextureRenderer;	
}

class CTextureRenderer : public CBaseVideoRenderer
{
public:
	CTextureRenderer(LPUNKNOWN pUnk, HRESULT *phr);
	~CTextureRenderer();

public:
	HRESULT CheckMediaType(const CMediaType *pmt);     // Format acceptable? 
	HRESULT SetMediaType(const CMediaType *pmt);       // Video format notification 
	HRESULT DoRenderSample(IMediaSample *pMediaSample); // New video sample 

	//BOOL m_bUseDynamicTextures;
	//LONG m_lVidWidth;   // Video width 
	//LONG m_lVidHeight;  // Video Height 
	//LONG m_lVidPitch;   // Video Pitch

	Graphics4::Texture* image;
	int width;
	int height;
	u8* pixels;
};

CTextureRenderer::CTextureRenderer(LPUNKNOWN pUnk, HRESULT *phr)
	: CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer),
		TEXT("Texture Renderer"), pUnk, phr)
{
	// Store and AddRef the texture for our use.
	ASSERT(phr);
	if (phr)
		*phr = S_OK;
}

CTextureRenderer::~CTextureRenderer()
{
	// Do nothing
}

#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}

HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
	HRESULT   hr = E_FAIL;
	VIDEOINFO *pvi = 0;

	CheckPointer(pmt, E_POINTER);

	// Reject the connection if this is not a video type
	if (*pmt->FormatType() != FORMAT_VideoInfo) {
		return E_INVALIDARG;
	}

	// Only accept RGB24 video
	pvi = (VIDEOINFO *)pmt->Format();

	if (IsEqualGUID(*pmt->Type(), MEDIATYPE_Video) &&
		IsEqualGUID(*pmt->Subtype(), MEDIASUBTYPE_RGB24))
	{
		hr = S_OK;
	}

	return hr;
}

HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt) {
	VIDEOINFO* info = (VIDEOINFO*)pmt->Format();
	width = info->bmiHeader.biWidth;
	height = abs(info->bmiHeader.biHeight);
	image = new Graphics4::Texture(width, height, Graphics4::Image::RGBA32, false);
	pixels = (u8*)malloc(width * height * 3);

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			pixels[y * width * 3 + x * 3 + 0] = 0;
			pixels[y * width * 3 + x * 3 + 1] = 0;
			pixels[y * width * 3 + x * 3 + 2] = 0;
		}
	}

	return S_OK;
}

HRESULT CTextureRenderer::DoRenderSample(IMediaSample* sample) {
	BYTE* videoPixels;
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

Video::Video(const char* filename) {
	duration = 1000 * 10;
	position = 0;
	finished = false;
	paused = false;
	//image = new Graphics4::Texture(100, 100, Graphics4::Image::RGBA32, false);

	HRESULT hr = S_OK;
	IBaseFilter* pFSrc;          // Source Filter 
	IPin* pFSrcPinOut;    // Source Filter Output Pin    

	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, __uuidof(IGraphBuilder), (void**)&graphBuilder);
	renderer = new CTextureRenderer(NULL, &hr);
	hr = graphBuilder->AddFilter(renderer, L"TEXTURERENDERER");
	wchar_t wideFilename[2048];
	mbstowcs(wideFilename, filename, 2048 - 1);
	hr = graphBuilder->AddSourceFilter(wideFilename, L"SOURCE", &pFSrc);
	hr = pFSrc->FindPin(L"Output", &pFSrcPinOut);
	hr = graphBuilder->Render(pFSrcPinOut);

	graphBuilder->QueryInterface(&mediaControl);
	graphBuilder->QueryInterface(&mediaPosition);
	graphBuilder->QueryInterface(&mediaEvent);

	mediaPosition->get_Duration(&duration);
	this->position = 0;
}

Graphics4::Texture* Video::currentImage() {
	u8* pixels = renderer->image->lock();
	int stride = renderer->image->stride();
	for (int y = 0; y < renderer->height; ++y) {
		for (int x = 0; x < renderer->width; ++x) {
			pixels[y * stride + x * 4 + 0] = renderer->pixels[y * renderer->width * 3 + x * 3 + 0];
			pixels[y * stride + x * 4 + 1] = renderer->pixels[y * renderer->width * 3 + x * 3 + 1];
			pixels[y * stride + x * 4 + 2] = renderer->pixels[y * renderer->width * 3 + x * 3 + 2];
			pixels[y * stride + x * 4 + 3] = 255;
		}
	}
	renderer->image->unlock();

	mediaPosition->get_CurrentPosition(&position);

	return renderer->image;
}

int Video::width() {
	return renderer->width;
}

int Video::height() {
	return renderer->height;
}

void Video::play() {
	mediaControl->Run();
}

void Video::pause() {
	mediaControl->Pause();
}

void Video::stop() {
	mediaControl->Stop();
}

void Video::update(double time) {
	mediaPosition->put_CurrentPosition(time);
}
