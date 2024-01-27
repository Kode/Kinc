#include <kinc/audio2/audio.h>

#include <kinc/backend/SystemMicrosoft.h>

#include <kinc/error.h>
#include <kinc/log.h>
#include <kinc/threads/thread.h>

#include <AudioClient.h>
#include <Windows.h>
#include <initguid.h>
#ifdef KORE_WINRT
#include <mfapi.h>
#endif
#include <mmdeviceapi.h>
#include <wrl/implements.h>

using namespace Kore;

#ifdef KORE_WINRT
using namespace ::Microsoft::WRL;
using namespace Windows::Media::Devices;
using namespace Windows::Storage::Streams;
#endif

template <class T> void SafeRelease(__deref_inout_opt T **ppT) {
	T *pTTemp = *ppT;
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

// based on the implementation in soloud and Microsoft sample code
namespace {
	kinc_thread_t thread;
	void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples, void *userdata) = nullptr;
	void *a2_userdata = nullptr;
	kinc_a2_buffer_t a2_buffer;

	IMMDeviceEnumerator *deviceEnumerator;
	IMMDevice *device;
	IAudioClient *audioClient = NULL;
	IAudioRenderClient *renderClient = NULL;
	HANDLE bufferEndEvent = 0;
	HANDLE audioProcessingDoneEvent;
	UINT32 bufferFrames;
	WAVEFORMATEX requestedFormat;
	WAVEFORMATEX *closestFormat;
	WAVEFORMATEX *format;

	bool initDefaultDevice();
	void audioThread(LPVOID);

#ifdef KORE_WINRT
	class AudioRenderer : public RuntimeClass<RuntimeClassFlags<ClassicCom>, FtmBase, IActivateAudioInterfaceCompletionHandler> {
	public:
		STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation *operation) {
			IUnknown *audioInterface = nullptr;
			HRESULT hrActivateResult = S_OK;
			HRESULT hr = operation->GetActivateResult(&hrActivateResult, &audioInterface);
			if (SUCCEEDED(hr) && SUCCEEDED(hrActivateResult)) {
				audioInterface->QueryInterface(IID_PPV_ARGS(&audioClient));
				initDefaultDevice();
				audioThread(nullptr);
			}
			return S_OK;
		}
	};

	ComPtr<AudioRenderer> renderer;
#endif

	bool initDefaultDevice() {
#ifdef KORE_WINRT
		HRESULT hr = S_OK;
#else
		if (renderClient != NULL) {
			renderClient->Release();
			renderClient = NULL;
		}

		if (audioClient != NULL) {
			audioClient->Release();
			audioClient = NULL;
		}

		if (bufferEndEvent != 0) {
			CloseHandle(bufferEndEvent);
			bufferEndEvent = 0;
		}

		kinc_log(KINC_LOG_LEVEL_INFO, "Initializing a new default audio device.");

		HRESULT hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
		if (hr == S_OK) {
			hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, 0, reinterpret_cast<void **>(&audioClient));
		}
#endif

		if (hr == S_OK) {
			const int sampleRate = 48000;

			format = &requestedFormat;
			memset(&requestedFormat, 0, sizeof(WAVEFORMATEX));
			requestedFormat.nChannels = 2;
			requestedFormat.nSamplesPerSec = sampleRate;
			requestedFormat.wFormatTag = WAVE_FORMAT_PCM;
			requestedFormat.wBitsPerSample = sizeof(short) * 8;
			requestedFormat.nBlockAlign = (requestedFormat.nChannels * requestedFormat.wBitsPerSample) / 8;
			requestedFormat.nAvgBytesPerSec = requestedFormat.nSamplesPerSec * requestedFormat.nBlockAlign;
			requestedFormat.cbSize = 0;

			HRESULT supported = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, format, &closestFormat);
			if (supported == S_FALSE) {
				kinc_log(KINC_LOG_LEVEL_WARNING, "Falling back to the system's preferred WASAPI mix format.", supported);
				if (closestFormat != nullptr) {
					format = closestFormat;
				}
				else {
					audioClient->GetMixFormat(&format);
				}
			}
			HRESULT result = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 40 * 1000 * 10, 0, format, 0);
			if (result != S_OK) {
				kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize WASAPI audio, going silent (error code 0x%x).", result);
				return false;
			}

			kinc_a2_samples_per_second = format->nSamplesPerSec;
			a2_buffer.format.samples_per_second = kinc_a2_samples_per_second;

			bufferFrames = 0;
			kinc_microsoft_affirm(audioClient->GetBufferSize(&bufferFrames));
			kinc_microsoft_affirm(audioClient->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void **>(&renderClient)));

			bufferEndEvent = CreateEvent(0, FALSE, FALSE, 0);
			kinc_affirm(bufferEndEvent != 0);

			kinc_microsoft_affirm(audioClient->SetEventHandle(bufferEndEvent));

			return true;
		}
		else {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize WASAPI audio.");
			return false;
		}
	}

	void copyS16Sample(s16 *buffer) {
		float value = *(float *)&a2_buffer.data[a2_buffer.read_location];
		a2_buffer.read_location += 4;
		if (a2_buffer.read_location >= a2_buffer.data_size) a2_buffer.read_location = 0;
		*buffer = (s16)(value * 32767);
	}

	void copyFloatSample(float *buffer) {
		float value = *(float *)&a2_buffer.data[a2_buffer.read_location];
		a2_buffer.read_location += 4;
		if (a2_buffer.read_location >= a2_buffer.data_size) a2_buffer.read_location = 0;
		*buffer = value;
	}

	void submitEmptyBuffer(unsigned frames) {
		BYTE *buffer = nullptr;
		HRESULT result = renderClient->GetBuffer(frames, &buffer);
		if (FAILED(result)) {
			return;
		}

		memset(buffer, 0, frames * format->nBlockAlign);

		result = renderClient->ReleaseBuffer(frames, 0);
	}

	void submitBuffer(unsigned frames) {
		BYTE *buffer = nullptr;
		HRESULT result = renderClient->GetBuffer(frames, &buffer);
		if (FAILED(result)) {
			if (result == AUDCLNT_E_DEVICE_INVALIDATED) {
				initDefaultDevice();
				submitEmptyBuffer(bufferFrames);
				audioClient->Start();
			}
			return;
		}

		if (a2_callback != nullptr) {
			a2_callback(&a2_buffer, frames * 2, a2_userdata);
			memset(buffer, 0, frames * format->nBlockAlign);
			if (format->wFormatTag == WAVE_FORMAT_PCM) {
				for (UINT32 i = 0; i < frames; ++i) {
					copyS16Sample((s16 *)&buffer[i * format->nBlockAlign]);
					copyS16Sample((s16 *)&buffer[i * format->nBlockAlign + 2]);
				}
			}
			else {
				for (UINT32 i = 0; i < frames; ++i) {
					copyFloatSample((float *)&buffer[i * format->nBlockAlign]);
					copyFloatSample((float *)&buffer[i * format->nBlockAlign + 4]);
				}
			}
		}
		else {
			memset(buffer, 0, frames * format->nBlockAlign);
		}

		result = renderClient->ReleaseBuffer(frames, 0);
		if (FAILED(result)) {
			if (result == AUDCLNT_E_DEVICE_INVALIDATED) {
				initDefaultDevice();
				submitEmptyBuffer(bufferFrames);
				audioClient->Start();
			}
		}
	}

	void audioThread(LPVOID) {
		submitBuffer(bufferFrames);
		audioClient->Start();
		while (WAIT_OBJECT_0 != WaitForSingleObject(audioProcessingDoneEvent, 0)) {
			WaitForSingleObject(bufferEndEvent, INFINITE);
			UINT32 padding = 0;
			HRESULT result = audioClient->GetCurrentPadding(&padding);
			if (FAILED(result)) {
				if (result == AUDCLNT_E_DEVICE_INVALIDATED) {
					initDefaultDevice();
					submitEmptyBuffer(bufferFrames);
					audioClient->Start();
				}
				continue;
			}
			UINT32 frames = bufferFrames - padding;
			submitBuffer(frames);
		}
	}

} // namespace

#ifndef KORE_WINRT
extern "C" void kinc_windows_co_initialize(void);
#endif

static bool initialized = false;

void kinc_a2_init() {
	if (initialized) {
		return;
	}

	initialized = true;

	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = (uint8_t *)malloc(a2_buffer.data_size);

	audioProcessingDoneEvent = CreateEvent(0, FALSE, FALSE, 0);
	kinc_affirm(audioProcessingDoneEvent != 0);

#ifdef KORE_WINRT
	renderer = Make<AudioRenderer>();

	IActivateAudioInterfaceAsyncOperation *asyncOp;
	Platform::String ^ deviceId = MediaDevice::GetDefaultAudioRenderId(Windows::Media::Devices::AudioDeviceRole::Default);
	kinc_microsoft_affirm(ActivateAudioInterfaceAsync(deviceId->Data(), __uuidof(IAudioClient2), nullptr, renderer.Get(), &asyncOp));
	SafeRelease(&asyncOp);
#else
	kinc_windows_co_initialize();
	kinc_microsoft_affirm(
	    CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void **>(&deviceEnumerator)));

	if (initDefaultDevice()) {
		kinc_thread_init(&thread, audioThread, NULL);
	}
#endif
}

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples, void *userdata), void *userdata) {
	a2_callback = kinc_a2_audio_callback;
	a2_userdata = userdata;
}

void kinc_a2_update() {}

void kinc_a2_shutdown() {
	// Wait for last data in buffer to play before stopping.
	// Sleep((DWORD)(hnsActualDuration/REFTIMES_PER_MILLISEC/2));

	//	affirm(pAudioClient->Stop());  // Stop playing.

	//	CoTaskMemFree(pwfx);
	//	SAFE_RELEASE(pEnumerator)
	//	SAFE_RELEASE(pDevice)
	//	SAFE_RELEASE(pAudioClient)
	//	SAFE_RELEASE(pRenderClient)
}
