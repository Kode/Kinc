#include "pch.h"
#include <Kore/Audio/Audio.h>
#include <Windows.h>
#include <wrl/implements.h>
#include <mfapi.h>
#include <AudioClient.h>
#include <mmdeviceapi.h>

using namespace Kore;

using namespace Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;

namespace {
	void affirm(HRESULT hr) {
		if (FAILED(hr)) {
			int a = 3;
			++a;
		}
	}

	// release and zero out a possible NULL pointer. note this will
	// do the release on a temp copy to avoid reentrancy issues that can result from
	// callbacks durring the release
	template <class T> void SafeRelease( __deref_inout_opt T** ppT) {
		T *pTTemp = *ppT;    // temp copy
		*ppT = nullptr;      // zero the input
		if (pTTemp) {
			pTTemp->Release();
		}
	}

		// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define EXIT_ON_ERROR(hres)  \
			  if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
			  if ((punk) != NULL)  \
				{ (punk)->Release(); (punk) = NULL; }

	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	IAudioClient2* audioClient = nullptr;
	IAudioRenderClient *pRenderClient = NULL;
	WAVEFORMATEX *pwfx = NULL;
	DWORD flags = 0;
	BYTE *pData;
	REFERENCE_TIME hnsActualDuration;
	UINT32 numFramesAvailable;
	UINT32 numFramesPadding;
	UINT32 bufferFrameCount;

	void copySample(void* buffer) {
		float value = *(float*)&Audio::buffer.data[Audio::buffer.readLocation];
		Audio::buffer.readLocation += 4;
		if (Audio::buffer.readLocation >= Audio::buffer.dataSize) Audio::buffer.readLocation = 0;
		*(float*)buffer = value;
	}

	class AudioRenderer : public RuntimeClass< RuntimeClassFlags< ClassicCom >, FtmBase, IActivateAudioInterfaceCompletionHandler > {
	public:
		STDMETHOD(ActivateCompleted)( IActivateAudioInterfaceAsyncOperation *operation ) {
			REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
		
			IUnknown* punkAudioInterface = nullptr;
			HRESULT hrActivateResult = S_OK;
			HRESULT hr = operation->GetActivateResult( &hrActivateResult, &punkAudioInterface );
			if (SUCCEEDED( hr ) && SUCCEEDED( hrActivateResult )) {
				// Get the pointer for the Audio Client
				punkAudioInterface->QueryInterface(IID_PPV_ARGS(&audioClient));

				affirm(audioClient->GetMixFormat(&pwfx));
				affirm(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL));
	
				// Get the actual size of the allocated buffer.
				affirm(audioClient->GetBufferSize(&bufferFrameCount));
				affirm(audioClient->GetService(IID_IAudioRenderClient, (void**)&pRenderClient));
				// Grab the entire buffer for the initial fill operation.
				affirm(pRenderClient->GetBuffer(bufferFrameCount, &pData));

				// Load the initial data into the shared buffer.
				//hr = pMySource->LoadData(bufferFrameCount, pData, &flags);

				affirm(pRenderClient->ReleaseBuffer(bufferFrameCount, flags));

				// Calculate the actual duration of the allocated buffer.
				hnsActualDuration = static_cast<REFERENCE_TIME>((double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec);

				affirm(audioClient->Start());  // Start playing.

				// Each loop fills about half of the shared buffer.
				while (flags != AUDCLNT_BUFFERFLAGS_SILENT) {
					// Sleep for half the buffer duration.
					//Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

					// See how much buffer space is available.
					affirm(audioClient->GetCurrentPadding(&numFramesPadding));
					numFramesAvailable = bufferFrameCount - numFramesPadding;
					
					if (Kore::Audio::audioCallback != nullptr && numFramesAvailable > 0) {
						Kore::Audio::audioCallback(numFramesAvailable * 2);
						// Grab all the available space in the shared buffer.
						affirm(pRenderClient->GetBuffer(numFramesAvailable, &pData));
						// Get next 1/2-second of data from the audio source.
						//pMySource->LoadData(numFramesAvailable, pData, &flags);
					
						for (UINT32 i = 0; i < numFramesAvailable; ++i) {
							copySample(&pData[i * pwfx->nBlockAlign]);
							copySample(&pData[i * pwfx->nBlockAlign + 4]);
						}

						affirm(pRenderClient->ReleaseBuffer(numFramesAvailable, flags));
					}
				}
			}
			return S_OK;
		}
	};

	ComPtr<AudioRenderer> renderer;
}

void Audio::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];
	renderer = Make<AudioRenderer>();

	IActivateAudioInterfaceAsyncOperation* asyncOp;
	Platform::String^ deviceId = MediaDevice::GetDefaultAudioRenderId(Windows::Media::Devices::AudioDeviceRole::Default);
	affirm(ActivateAudioInterfaceAsync(deviceId->Data(), __uuidof(IAudioClient2), nullptr, renderer.Get(), &asyncOp));
	SafeRelease(&asyncOp);
}

void Audio::update() {
	
}

void Audio::shutdown() {
	// Wait for last data in buffer to play before stopping.
	//Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

//	affirm(pAudioClient->Stop());  // Stop playing.

//	CoTaskMemFree(pwfx);
//	SAFE_RELEASE(pEnumerator)
//	SAFE_RELEASE(pDevice)
//	SAFE_RELEASE(pAudioClient)
//	SAFE_RELEASE(pRenderClient)
}
