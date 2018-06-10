#include "pch.h"

#include <Kore/Audio2/Audio.h>
#include <Kore/Log.h>
#include <Kore/SystemMicrosoft.h>
#include <Kore/Threads/Thread.h>

#include <AudioClient.h>
#include <Windows.h>
#include <initguid.h>
#include <mfapi.h>
#include <mmdeviceapi.h>
#include <wrl/implements.h>

using namespace Kore;

#ifndef KORE_WINDOWS
using namespace ::Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;
#endif

// based on the implementation in soloud and Microsoft sample code
namespace {
	IMMDeviceEnumerator* deviceEnumerator;
	IMMDevice* device;
	IAudioClient* audioClient;
	IAudioRenderClient* renderClient;
	HANDLE bufferEndEvent;
	HANDLE audioProcessingDoneEvent;
	UINT32 bufferFrames;
	WAVEFORMATEX requestedFormat;
	WAVEFORMATEX* closestFormat;
	WAVEFORMATEX* format;

	void copyS16Sample(s16* buffer) {
		float value = *(float*)&Audio2::buffer.data[Audio2::buffer.readLocation];
		Audio2::buffer.readLocation += 4;
		if (Audio2::buffer.readLocation >= Audio2::buffer.dataSize) Audio2::buffer.readLocation = 0;
		*buffer = (s16)(value * 32767);
	}

	void copyFloatSample(float* buffer) {
		float value = *(float*)&Audio2::buffer.data[Audio2::buffer.readLocation];
		Audio2::buffer.readLocation += 4;
		if (Audio2::buffer.readLocation >= Audio2::buffer.dataSize) Audio2::buffer.readLocation = 0;
		*buffer = value;
	}

	void submitBuffer(unsigned frames) {
		BYTE* buffer = nullptr;
		if (FAILED(renderClient->GetBuffer(frames, &buffer))) {
			return;
		}

		Kore::Audio2::audioCallback(frames * 2);
		memset(buffer, 0, frames * format->nBlockAlign);
		if (format->wFormatTag == WAVE_FORMAT_PCM) {
			for (UINT32 i = 0; i < frames; ++i) {
				copyS16Sample((s16*)&buffer[i * format->nBlockAlign]);
				copyS16Sample((s16*)&buffer[i * format->nBlockAlign + 2]);
			}
		}
		else {
			for (UINT32 i = 0; i < frames; ++i) {
				copyFloatSample((float*)&buffer[i * format->nBlockAlign]);
				copyFloatSample((float*)&buffer[i * format->nBlockAlign + 4]);
			}
		}

		renderClient->ReleaseBuffer(frames, 0);
	}

	void audioThread(LPVOID) {
		submitBuffer(bufferFrames);
		audioClient->Start();
		while (WAIT_OBJECT_0 != WaitForSingleObject(audioProcessingDoneEvent, 0)) {
			WaitForSingleObject(bufferEndEvent, INFINITE);
			UINT32 padding = 0;
			if (FAILED(audioClient->GetCurrentPadding(&padding))) {
				continue;
			}
			UINT32 frames = bufferFrames - padding;
			submitBuffer(frames);
		}
	}

	void initAudio() {
		const int sampleRate = 48000;

		bufferEndEvent = CreateEvent(0, FALSE, FALSE, 0);
		affirm(bufferEndEvent != 0);

		audioProcessingDoneEvent = CreateEvent(0, FALSE, FALSE, 0);
		affirm(audioProcessingDoneEvent != 0);

		format = &requestedFormat;
		ZeroMemory(&requestedFormat, sizeof(WAVEFORMATEX));
		requestedFormat.nChannels = 2;
		requestedFormat.nSamplesPerSec = sampleRate;
		requestedFormat.wFormatTag = WAVE_FORMAT_PCM;
		requestedFormat.wBitsPerSample = sizeof(short) * 8;
		requestedFormat.nBlockAlign = (requestedFormat.nChannels * requestedFormat.wBitsPerSample) / 8;
		requestedFormat.nAvgBytesPerSec = requestedFormat.nSamplesPerSec * requestedFormat.nBlockAlign;
		requestedFormat.cbSize = 0;

		HRESULT supported = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, format, &closestFormat);
		if (supported == S_FALSE) {
			log(Warning, "Falling back to the system's preferred WASAPI mix format.", supported);
			if (closestFormat != nullptr) {
				format = closestFormat;
			}
			else {
				audioClient->GetMixFormat(&format);
			}
		}
		HRESULT result = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 40 * 1000 * 10, 0, format, 0);
		if (result != S_OK) {
			log(Warning, "Could not initialize WASAPI audio, going silent (error code 0x%x).", result);
			return;
		}

		bufferFrames = 0;
		Kore::Microsoft::affirm(audioClient->GetBufferSize(&bufferFrames));
		Kore::Microsoft::affirm(audioClient->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void**>(&renderClient)));
		Kore::Microsoft::affirm(audioClient->SetEventHandle(bufferEndEvent));

#ifdef KORE_WINDOWS
		createAndRunThread(audioThread, nullptr);
#else
		audioThread(nullptr);
#endif
	}

#ifndef KORE_WINDOWS
	class AudioRenderer : public RuntimeClass<RuntimeClassFlags<ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler> {
	public:
		STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* operation) {
			IUnknown* audioInterface = nullptr;
			HRESULT hrActivateResult = S_OK;
			HRESULT hr = operation->GetActivateResult(&hrActivateResult, &audioInterface);
			if (SUCCEEDED(hr) && SUCCEEDED(hrActivateResult)) {
				audioInterface->QueryInterface(IID_PPV_ARGS(&audioClient));
				initAudio();
			}
			return S_OK;
		}
	};

	ComPtr<AudioRenderer> renderer;
#endif
} // namespace

template <class T> void SafeRelease(__deref_inout_opt T** ppT) {
	T* pTTemp = *ppT;
	*ppT = nullptr;
	if (pTTemp) {
		pTTemp->Release();
	}
}

#define SAFE_RELEASE(punk)                                                                                                                                     \
	if ((punk) != NULL) {                                                                                                                                      \
		(punk)->Release();                                                                                                                                     \
		(punk) = NULL;                                                                                                                                         \
	}

void Audio2::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];

#ifdef KORE_WINDOWS
	Microsoft::affirm(CoInitializeEx(0, COINIT_MULTITHREADED));
	Microsoft::affirm(
	    CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&deviceEnumerator)));
	Microsoft::affirm(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device));
	Microsoft::affirm(device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, reinterpret_cast<void**>(&audioClient)));
	initAudio();
#else
	renderer = Make<AudioRenderer>();

	IActivateAudioInterfaceAsyncOperation* asyncOp;
	Platform::String ^ deviceId = MediaDevice::GetDefaultAudioRenderId(Windows::Media::Devices::AudioDeviceRole::Default);
	Kore::Microsoft::affirm(ActivateAudioInterfaceAsync(deviceId->Data(), __uuidof(IAudioClient2), nullptr, renderer.Get(), &asyncOp));
	SafeRelease(&asyncOp);
#endif
}

void Audio2::update() {}

void Audio2::shutdown() {
	// Wait for last data in buffer to play before stopping.
	// Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

	//	affirm(pAudioClient->Stop());  // Stop playing.

	//	CoTaskMemFree(pwfx);
	//	SAFE_RELEASE(pEnumerator)
	//	SAFE_RELEASE(pDevice)
	//	SAFE_RELEASE(pAudioClient)
	//	SAFE_RELEASE(pRenderClient)
}
