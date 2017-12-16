#include "pch.h"

#include <Kore/Audio2/Audio.h>

#include <Kore/WinError.h>

#include <AudioClient.h>
#include <Windows.h>
#include <mfapi.h>
#include <mmdeviceapi.h>
#include <wrl/implements.h>

using namespace Kore;

using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;

// based on the implementation in soloud and Microsoft sample code
namespace {
	IMMDeviceEnumerator* deviceEnumerator;
	IMMDevice* device;
	IAudioClient* audioClient;
	IAudioRenderClient* renderClient;
	HANDLE bufferEndEvent;
	HANDLE audioProcessingDoneEvent;
	UINT32 bufferFrames;
	int channels;

	void copySample(s16* buffer) {
		float value = *(float*)&Audio2::buffer.data[Audio2::buffer.readLocation];
		Audio2::buffer.readLocation += 4;
		if (Audio2::buffer.readLocation >= Audio2::buffer.dataSize) Audio2::buffer.readLocation = 0;
		*buffer = (s16)(value * 32767);
	}
	
	void submitBuffer(unsigned frames) {
		BYTE* buffer = nullptr;
		if (FAILED(renderClient->GetBuffer(frames, &buffer))) {
			return;
		}
		
		s16* s16buffer = (s16*)buffer;
		Kore::Audio2::audioCallback(frames * 2);
		for (UINT32 i = 0; i < frames * 2; ++i) {
			copySample(&s16buffer[i]);
		}

		renderClient->ReleaseBuffer(frames, 0);
	}

	void audioThread(LPVOID aParam) {
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

	class AudioRenderer : public RuntimeClass<RuntimeClassFlags<ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler> {
	public:
		STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* operation) {
			IUnknown* audioInterface = nullptr;
			HRESULT hrActivateResult = S_OK;
			HRESULT hr = operation->GetActivateResult(&hrActivateResult, &audioInterface);
			if (SUCCEEDED(hr) && SUCCEEDED(hrActivateResult)) {
				audioInterface->QueryInterface(IID_PPV_ARGS(&audioClient));

				const int sampleRate = 48000;

				bufferEndEvent = CreateEvent(0, FALSE, FALSE, 0);
				affirm(bufferEndEvent != 0);

				audioProcessingDoneEvent = CreateEvent(0, FALSE, FALSE, 0);
				affirm(audioProcessingDoneEvent != 0);

				WAVEFORMATEX format;
				ZeroMemory(&format, sizeof(WAVEFORMATEX));
				format.nChannels = 2;
				format.nSamplesPerSec = sampleRate;
				format.wFormatTag = WAVE_FORMAT_PCM;
				format.wBitsPerSample = sizeof(short) * 8;
				format.nBlockAlign = (format.nChannels*format.wBitsPerSample) / 8;
				format.nAvgBytesPerSec = format.nSamplesPerSec*format.nBlockAlign;
				affirm(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 40 * 1000 * 10, 0, &format, 0));

				bufferFrames = 0;
				affirm(audioClient->GetBufferSize(&bufferFrames));
				affirm(audioClient->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void**>(&renderClient)));
				affirm(audioClient->SetEventHandle(bufferEndEvent));

				channels = format.nChannels;

				audioThread(nullptr);
				//** createThread
			}
			return S_OK;
		}
	};

	ComPtr<AudioRenderer> renderer;
}

template <class T> void SafeRelease(__deref_inout_opt T** ppT) {
	T* pTTemp = *ppT;
	*ppT = nullptr;
	if (pTTemp) {
		pTTemp->Release();
	}
}

#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL) {   \
		(punk)->Release();  \
		(punk) = NULL;      \
	}

void Audio2::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];
	
	renderer = Make<AudioRenderer>();

	IActivateAudioInterfaceAsyncOperation* asyncOp;
	Platform::String^ deviceId = MediaDevice::GetDefaultAudioRenderId(Windows::Media::Devices::AudioDeviceRole::Default);
	affirm(ActivateAudioInterfaceAsync(deviceId->Data(), __uuidof(IAudioClient2), nullptr, renderer.Get(), &asyncOp));
	SafeRelease(&asyncOp);
	
	/*
	affirm(CoInitializeEx(0, COINIT_MULTITHREADED));
	
	auto m_DeviceIdString = MediaDevice::GetDefaultAudioRenderId(Windows::Media::Devices::AudioDeviceRole::Default);
	ActivateAudioInterfaceAsync(m_DeviceIdString, __uuidof(IAudioClient))

	affirm(CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&data->deviceEnumerator)));

	affirm(data->deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &data->device));

	affirm(data->device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, reinterpret_cast<void**>(&data->audioClient)));
	*/
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
