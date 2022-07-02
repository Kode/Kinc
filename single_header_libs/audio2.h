#pragma once

/*! \file global.h
    \brief Provides basic functionality that's used all over the place. There's usually no need to manually include this.
*/

#ifdef __cplusplus

namespace Kore {
	typedef unsigned char u8;   // 1 Byte
	typedef unsigned short u16; // 2 Byte
	typedef unsigned int u32;   // 4 Byte

#if defined(__LP64__) || defined(_LP64) || defined(_WIN64)
#define KORE_64
#endif

#ifdef KORE_WINDOWS
	typedef unsigned __int64 u64; // 8 Byte
#else
	typedef unsigned long long u64;
#endif
	typedef char s8;   // 1 Byte
	typedef short s16; // 2 Byte
	typedef int s32;   // 4 Byte
#ifdef KORE_WINDOWS
	typedef __int64 s64; // 8 Byte
#else
	typedef long long s64;
#endif

	typedef u32 uint; // 4 Byte
	typedef s32 sint; // 4 Byte

#ifdef KORE_64
	typedef s64 spint;
	typedef u64 upint;
#else
	typedef s32 spint;
	typedef u32 upint;
#endif
}

// pseudo C++11
#if !defined(_MSC_VER) && __cplusplus <= 199711L
#define nullptr 0
#define override
#endif

#define Noexcept throw()

#endif

#include <stdbool.h>
#include <stdint.h>

#if defined(KORE_PPC)
#define KORE_BIG_ENDIAN
#else
#define KORE_LITTLE_ENDIAN
#endif

#ifdef KORE_WINDOWS
#if defined(KINC_DYNAMIC)
#define KINC_FUNC __declspec(dllimport)
#elif defined(KINC_DYNAMIC_COMPILE)
#define KINC_FUNC __declspec(dllexport)
#else
#define KINC_FUNC
#endif
#else
#define KINC_FUNC
#endif

#if defined(KORE_PPC)
#define KINC_BIG_ENDIAN
#else
#define KINC_LITTLE_ENDIAN
#endif

#ifdef _MSC_VER
#define KINC_INLINE static __forceinline
#else
#define KINC_INLINE static __attribute__((always_inline))
#endif

#ifdef _MSC_VER
#define KINC_MICROSOFT
#define KORE_MICROSOFT
#endif

#if defined(_WIN32)

#if defined(KORE_WINDOWSAPP)

#define KINC_WINDOWSAPP
#define KINC_WINRT
#define KORE_WINRT

#else

#define KINC_WINDOWS
#define KORE_WINDOWS

#endif

#elif defined(__APPLE__)

#include <TargetConditionals.h>

#if TARGET_OS_IPHONE

#if defined(KORE_TVOS)
#define KINC_TVOS
#else
#define KINC_IOS
#define KORE_IOS
#endif

#define KINC_APPLE_SOC

#else

#define KINC_MACOS
#define KORE_MACOS

#if defined(__arm64__)
#define KINC_APPLE_SOC
#endif

#endif

#define KINC_POSIX
#define KORE_POSIX

#elif defined(__linux__)

#if !defined(KORE_ANDROID)
#define KINC_LINUX
#define KORE_LINUX
#endif

#define KINC_POSIX
#define KORE_POSIX

#endif

#ifdef __cplusplus
extern "C" {
#endif
int kickstart(int argc, char **argv);
#ifdef __cplusplus
}
#endif

#include <stdint.h>

/*! \file audio.h
    \brief Audio2 is a low-level audio-API that allows you to directly provide a stream of audio-samples.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kinc_a2_buffer_format {
	int channels;
	int samples_per_second;
	int bits_per_sample;
} kinc_a2_buffer_format_t;

typedef struct kinc_a2_buffer {
	kinc_a2_buffer_format_t format;
	uint8_t *data;
	int data_size;
	int read_location;
	int write_location;
} kinc_a2_buffer_t;

/// <summary>
/// Initializes the Audio2-API.
/// </summary>
KINC_FUNC void kinc_a2_init(void);

/// <summary>
/// Sets the callback that's used to provide audio-samples. This is the primary method of operation for Audio2. The callback is expected to write the requested
/// number of samples into the ring-buffer. The callback is typically called from the system's audio-thread to minimize audio-latency.
/// </summary>
/// <param name="kinc_a2_audio_callback">The callback to set</param>
KINC_FUNC void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples));

/// <summary>
/// Sets a callback that's called when the system's sample-rate changes.
/// </summary>
/// <param name="kinc_a2_sample_rate_callback">The callback to set</param>
/// <returns></returns>
KINC_FUNC void kinc_a2_set_sample_rate_callback(void (*kinc_a2_sample_rate_callback)(void));

/// <summary>
/// The current sample-rate of the system.
/// </summary>
KINC_FUNC extern int kinc_a2_samples_per_second;

/// <summary>
/// kinc_a2_update should be called every frame. It is required by some systems to pump their audio-loops but on most systems it is a no-op.
/// </summary>
KINC_FUNC void kinc_a2_update(void);

/// <summary>
/// Shuts down the Audio2-API.
/// </summary>
KINC_FUNC void kinc_a2_shutdown(void);

#ifdef KINC_IMPLEMENTATION_AUDIO2
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

int kinc_a2_samples_per_second = 44100;

#ifdef KORE_WINDOWS
#include <kinc/backend/SystemMicrosoft.h>

#include <stdarg.h>

/*! \file error.h
    \brief Contains functionality to stop the program in case of an error and create a user-visible error message.

    The affirm and error functions print an error message and then exit the program. Error messages can be made
    visible to the user (unless a console window is active this is only implemented for Windows).
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Exits the program when a condition is untrue and shows
/// a generic error message.
/// </summary>
/// <remarks>
/// This is an alternative to assert which also persists in release
/// builds. Use this instead of assert in situations where you want
/// your users to see what's going wrong.
/// This uses Kore's log and error functionality to make errors
/// visible.
/// </remarks>
/// <param name="condition">
/// Exits the program if condition is false,
/// otherwise does nothing.
/// </param>
KINC_FUNC void kinc_affirm(bool condition);

/// <summary>
/// Exits the program when a condition is untrue and shows
/// a provided error message.
/// </summary>
/// <remarks>
/// This is equivalent to kinc_affirm() but uses the provided message
/// instead of a generic one.
/// </remarks>
/// <param name="condition">
/// Exits the program if condition is false,
/// otherwise does nothing.
/// </param>
/// <param name="format">
/// The parameter is equivalent to the first printf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second printf parameter.
/// </param>
KINC_FUNC void kinc_affirm_message(bool condition, const char *format, ...);

/// <summary>
/// Equivalent to kinc_affirm_message but uses a va_list parameter.
/// </summary>
/// <remarks>
/// You will need this if you want to provide the parameters using va_start/va_end.
/// </remarks>
/// <param name="condition">
/// Exits the program if condition is false,
/// otherwise does nothing.
/// </param>
/// <param name="format">
/// The parameter is equivalent to the first vprintf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second vprintf parameter.
/// </param>
KINC_FUNC void kinc_affirm_args(bool condition, const char *format, va_list args);

/// <summary>
/// Exits the program and shows a generic error message
/// </summary>
/// <remarks>
/// Mainly this just calls exit(EXIT_FAILURE) but will also use
/// Kore's log function and on Windows show an error message box.
/// </remarks>
KINC_FUNC void kinc_error(void);

/// <summary>
/// Exits the program and shows a provided error message.
/// </summary>
/// <remarks>
/// This is equivalent to kinc_error() but uses the provided message
/// instead of a generic one.
/// </remarks>
/// <param name="format">
/// The parameter is equivalent to the first printf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second printf parameter.
/// </param>
KINC_FUNC void kinc_error_message(const char *format, ...);

/// <summary>
/// Equivalent to kinc_error_message but uses a va_list parameter.
/// </summary>
/// <remarks>
/// You will need this if you want to provide the parameters using va_start/va_end.
/// </remarks>
/// <param name="format">
/// The parameter is equivalent to the first vprintf parameter.
/// </param>
/// <param name="...">
/// The parameter is equivalent to the second vprintf parameter.
/// </param>
KINC_FUNC void kinc_error_args(const char *format, va_list args);

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#ifndef KINC_IMPLEMENTATION_ROOT
#undef KINC_IMPLEMENTATION
#endif
#include <stdarg.h>

/*! \file log.h
    \brief Contains basic logging functionality.

    Logging functionality is similar to plain printf but provides some system-specific bonuses.
*/

#ifdef __cplusplus
extern "C" {
#endif

/// <summary>
/// Pass this to kinc_log or kinc_log_args
/// </summary>
/// <remarks>
/// When used on Android the log level is converted to the equivalent
/// Android logging level. It is currently ignored on all other targets.
/// </remarks>
typedef enum { KINC_LOG_LEVEL_INFO, KINC_LOG_LEVEL_WARNING, KINC_LOG_LEVEL_ERROR } kinc_log_level_t;

/// <summary>
/// Logging function similar to printf including some system-specific bonuses
/// </summary>
/// <remarks>
/// On most systems this is equivalent to printf.
/// On Windows it works with utf-8 strings (like printf does on any other target)
/// and also prints to the debug console in IDEs.
/// On Android this uses the android logging functions and also passes the logging level.
/// </remarks>
/// <param name="log_level">
/// The logLevel is ignored on all targets but Android where it is converted
/// to the equivalent Android log level
/// </param>
/// <param name="format">The parameter is equivalent to the first printf parameter.</param>
/// <param name="...">The parameter is equivalent to the second printf parameter.</param>
KINC_FUNC void kinc_log(kinc_log_level_t log_level, const char *format, ...);

/// <summary>
/// Equivalent to kinc_log but uses a va_list parameter
/// </summary>
/// <remarks>
/// You will need this if you want to log parameters using va_start/va_end.
/// </remarks>
/// <param name="log_level">
/// The logLevel is ignored on all targets but Android where it is converted
/// to the equivalent Android log level
/// </param>
/// <param name="format">The parameter is equivalent to the first vprintf parameter.</param>
/// <param name="args">The parameter is equivalent to the second vprintf parameter.</param>
KINC_FUNC void kinc_log_args(kinc_log_level_t log_level, const char *format, va_list args);

#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KINC_IMPLEMENTATION

#include <stdio.h>
#include <string.h>

#ifdef KINC_IMPLEMENTATION_ROOT
#undef KINC_IMPLEMENTATION
#endif
#include <kinc/string.h>
#include <kinc/system.h>
#ifdef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#ifdef KORE_MICROSOFT
#include <Windows.h>
#include <kinc/backend/SystemMicrosoft.h>
#endif

#ifdef KORE_ANDROID
#include <android/log.h>
#endif

void kinc_log(kinc_log_level_t level, const char *format, ...) {
	va_list args;
	va_start(args, format);
	kinc_log_args(level, format, args);
	va_end(args);
}

#define UTF8

void kinc_log_args(kinc_log_level_t level, const char *format, va_list args) {
#ifdef KORE_MICROSOFT
#ifdef UTF8
	wchar_t buffer[4096];
	kinc_microsoft_format(format, args, buffer);
	kinc_wstring_append(buffer, L"\r\n");
	OutputDebugString(buffer);
#ifdef KORE_WINDOWS
	DWORD written;
	WriteConsole(GetStdHandle(level == KINC_LOG_LEVEL_INFO ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), buffer, (DWORD)kinc_wstring_length(buffer), &written, NULL);
#endif
#else
	char buffer[4096];
	vsnprintf(buffer, 4090, format, args);
	kinc_string_append(buffer, "\r\n");
	OutputDebugStringA(buffer);
#ifdef KORE_WINDOWS
	DWORD written;
	WriteConsoleA(GetStdHandle(level == KINC_LOG_LEVEL_INFO ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE), buffer, (DWORD)kinc_string_length(buffer), &written, NULL);
#endif
#endif
#else
	char buffer[4096];
	vsnprintf(buffer, 4090, format, args);
	kinc_string_append(buffer, "\n");
	fprintf(level == KINC_LOG_LEVEL_INFO ? stdout : stderr, "%s", buffer);
#endif

#ifdef KORE_ANDROID
	switch (level) {
	case KINC_LOG_LEVEL_INFO:
		__android_log_vprint(ANDROID_LOG_INFO, "Kinc", format, args);
		break;
	case KINC_LOG_LEVEL_WARNING:
		__android_log_vprint(ANDROID_LOG_WARN, "Kinc", format, args);
		break;
	case KINC_LOG_LEVEL_ERROR:
		__android_log_vprint(ANDROID_LOG_ERROR, "Kinc", format, args);
		break;
	}
#endif
}

#endif

#ifdef __cplusplus
}
#endif
#ifndef KINC_IMPLEMENTATION_ROOT
#define KINC_IMPLEMENTATION
#endif

#include <stdlib.h>

#ifdef KORE_WINDOWS
#include <Windows.h>

#include <kinc/backend/SystemMicrosoft.h>
#endif

void kinc_affirm(bool condition) {
	if (!condition) {
		kinc_error();
	}
}

void kinc_affirm_message(bool condition, const char *format, ...) {
	if (!condition) {
		va_list args;
		va_start(args, format);
		kinc_error_args(format, args);
		va_end(args);
	}
}

void kinc_affirm_args(bool condition, const char *format, va_list args) {
	if (!condition) {
		kinc_error_args(format, args);
	}
}

void kinc_error(void) {
	kinc_error_message("Unknown error");
}

void kinc_error_message(const char *format, ...) {
	{
		va_list args;
		va_start(args, format);
		kinc_log_args(KINC_LOG_LEVEL_ERROR, format, args);
		va_end(args);
	}

#ifdef KORE_WINDOWS
	{
		va_list args;
		va_start(args, format);
		wchar_t buffer[4096];
		kinc_microsoft_format(format, args, buffer);
		MessageBox(NULL, buffer, L"Error", 0);
		va_end(args);
	}
#endif

#ifndef KINC_NO_CLIB
	exit(EXIT_FAILURE);
#endif
}

void kinc_error_args(const char *format, va_list args) {
	kinc_log_args(KINC_LOG_LEVEL_ERROR, format, args);

#ifdef KORE_WINDOWS
	wchar_t buffer[4096];
	kinc_microsoft_format(format, args, buffer);
	MessageBox(NULL, buffer, L"Error", 0);
#endif

#ifndef KINC_NO_CLIB
	exit(EXIT_FAILURE);
#endif
}

#endif

#ifdef __cplusplus
}
#endif
#include <kinc/log.h>
#include <kinc/memory.h>

#include <initguid.h>

#include <AudioClient.h>
#include <Windows.h>
#include <mmdeviceapi.h>

// MIDL_INTERFACE("1CB9AD4C-DBFA-4c32-B178-C2F568A703B2")
DEFINE_GUID(IID_IAudioClient, 0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2);
// MIDL_INTERFACE("F294ACFC-3146-4483-A7BF-ADDCA7C260E2")
DEFINE_GUID(IID_IAudioRenderClient, 0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2);
// MIDL_INTERFACE("A95664D2-9614-4F35-A746-DE8DB63617E6")
DEFINE_GUID(IID_IMMDeviceEnumerator, 0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6);
// DECLSPEC_UUID("BCDE0395-E52F-467C-8E3D-C4579291692E")
DEFINE_GUID(CLSID_MMDeviceEnumerator, 0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E);

#define SAFE_RELEASE(punk)                                                                                                                                     \
	if ((punk) != NULL) {                                                                                                                                      \
		(punk)->Release();                                                                                                                                     \
		(punk) = NULL;                                                                                                                                         \
	}

// based on the implementation in soloud and Microsoft sample code
static void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples) = NULL;
static kinc_a2_buffer_t a2_buffer;

static IMMDeviceEnumerator *deviceEnumerator;
static IMMDevice *device;
static IAudioClient *audioClient = NULL;
static IAudioRenderClient *renderClient = NULL;
static HANDLE bufferEndEvent = 0;
static HANDLE audioProcessingDoneEvent;
static UINT32 bufferFrames;
static WAVEFORMATEX requestedFormat;
static WAVEFORMATEX *closestFormat;
static WAVEFORMATEX *format;

static bool initDefaultDevice() {
	if (renderClient != NULL) {
		renderClient->lpVtbl->Release(renderClient);
		renderClient = NULL;
	}

	if (audioClient != NULL) {
		audioClient->lpVtbl->Release(audioClient);
		audioClient = NULL;
	}

	if (bufferEndEvent != 0) {
		CloseHandle(bufferEndEvent);
		bufferEndEvent = 0;
	}

	kinc_log(KINC_LOG_LEVEL_INFO, "Initializing a new default audio device.");

	HRESULT hr = deviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(deviceEnumerator, eRender, eConsole, &device);
	if (hr == S_OK) {
		hr = device->lpVtbl->Activate(device, &IID_IAudioClient, CLSCTX_ALL, 0, (void **)&audioClient);
	}

	if (hr == S_OK) {
		const int sampleRate = 48000;

		format = &requestedFormat;
		kinc_memset(&requestedFormat, 0, sizeof(WAVEFORMATEX));
		requestedFormat.nChannels = 2;
		requestedFormat.nSamplesPerSec = sampleRate;
		requestedFormat.wFormatTag = WAVE_FORMAT_PCM;
		requestedFormat.wBitsPerSample = sizeof(short) * 8;
		requestedFormat.nBlockAlign = (requestedFormat.nChannels * requestedFormat.wBitsPerSample) / 8;
		requestedFormat.nAvgBytesPerSec = requestedFormat.nSamplesPerSec * requestedFormat.nBlockAlign;
		requestedFormat.cbSize = 0;

		HRESULT supported = audioClient->lpVtbl->IsFormatSupported(audioClient, AUDCLNT_SHAREMODE_SHARED, format, &closestFormat);
		if (supported == S_FALSE) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Falling back to the system's preferred WASAPI mix format.", supported);
			if (closestFormat != NULL) {
				format = closestFormat;
			}
			else {
				audioClient->lpVtbl->GetMixFormat(audioClient, &format);
			}
		}
		HRESULT result =
		    audioClient->lpVtbl->Initialize(audioClient, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 40 * 1000 * 10, 0, format, 0);
		if (result != S_OK) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize WASAPI audio, going silent (error code 0x%x).", result);
			return false;
		}

		kinc_a2_samples_per_second = format->nSamplesPerSec;
		a2_buffer.format.samples_per_second = kinc_a2_samples_per_second;

		bufferFrames = 0;
		kinc_microsoft_affirm(audioClient->lpVtbl->GetBufferSize(audioClient, &bufferFrames));
		kinc_microsoft_affirm(audioClient->lpVtbl->GetService(audioClient, &IID_IAudioRenderClient, (void **)&renderClient));

		bufferEndEvent = CreateEvent(0, FALSE, FALSE, 0);
		kinc_affirm(bufferEndEvent != 0);

		kinc_microsoft_affirm(audioClient->lpVtbl->SetEventHandle(audioClient, bufferEndEvent));

		return true;
	}
	else {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not initialize WASAPI audio.");
		return false;
	}
}

static void copyS16Sample(int16_t *buffer) {
	float value = *(float *)&a2_buffer.data[a2_buffer.read_location];
	a2_buffer.read_location += 4;
	if (a2_buffer.read_location >= a2_buffer.data_size) a2_buffer.read_location = 0;
	*buffer = (int16_t)(value * 32767);
}

static void copyFloatSample(float *buffer) {
	float value = *(float *)&a2_buffer.data[a2_buffer.read_location];
	a2_buffer.read_location += 4;
	if (a2_buffer.read_location >= a2_buffer.data_size) a2_buffer.read_location = 0;
	*buffer = value;
}

static void submitEmptyBuffer(unsigned frames) {
	BYTE *buffer = NULL;
	HRESULT result = renderClient->lpVtbl->GetBuffer(renderClient, frames, &buffer);
	if (FAILED(result)) {
		return;
	}

	kinc_memset(buffer, 0, frames * format->nBlockAlign);

	result = renderClient->lpVtbl->ReleaseBuffer(renderClient, frames, 0);
}

static void submitBuffer(unsigned frames) {
	BYTE *buffer = NULL;
	HRESULT result = renderClient->lpVtbl->GetBuffer(renderClient, frames, &buffer);
	if (FAILED(result)) {
		if (result == AUDCLNT_E_DEVICE_INVALIDATED) {
			initDefaultDevice();
			submitEmptyBuffer(bufferFrames);
			audioClient->lpVtbl->Start(audioClient);
		}
		return;
	}

	if (a2_callback != NULL) {
		a2_callback(&a2_buffer, frames * 2);
		kinc_memset(buffer, 0, frames * format->nBlockAlign);
		if (format->wFormatTag == WAVE_FORMAT_PCM) {
			for (UINT32 i = 0; i < frames; ++i) {
				copyS16Sample((int16_t *)&buffer[i * format->nBlockAlign]);
				copyS16Sample((int16_t *)&buffer[i * format->nBlockAlign + 2]);
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
		kinc_memset(buffer, 0, frames * format->nBlockAlign);
	}

	result = renderClient->lpVtbl->ReleaseBuffer(renderClient, frames, 0);
	if (FAILED(result)) {
		if (result == AUDCLNT_E_DEVICE_INVALIDATED) {
			initDefaultDevice();
			submitEmptyBuffer(bufferFrames);
			audioClient->lpVtbl->Start(audioClient);
		}
	}
}

static DWORD WINAPI audioThread(LPVOID ignored) {
	submitBuffer(bufferFrames);
	audioClient->lpVtbl->Start(audioClient);
	while (WAIT_OBJECT_0 != WaitForSingleObject(audioProcessingDoneEvent, 0)) {
		WaitForSingleObject(bufferEndEvent, INFINITE);
		UINT32 padding = 0;
		HRESULT result = audioClient->lpVtbl->GetCurrentPadding(audioClient, &padding);
		if (FAILED(result)) {
			if (result == AUDCLNT_E_DEVICE_INVALIDATED) {
				initDefaultDevice();
				submitEmptyBuffer(bufferFrames);
				audioClient->lpVtbl->Start(audioClient);
			}
			continue;
		}
		UINT32 frames = bufferFrames - padding;
		submitBuffer(frames);
	}
	return 0;
}

void kinc_windows_co_initialize(void);

void kinc_a2_init() {
	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = (uint8_t *)kinc_allocate(a2_buffer.data_size);

	audioProcessingDoneEvent = CreateEvent(0, FALSE, FALSE, 0);
	kinc_affirm(audioProcessingDoneEvent != 0);

	kinc_windows_co_initialize();
	kinc_microsoft_affirm(CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void **)&deviceEnumerator));

	if (initDefaultDevice()) {
		CreateThread(0, 65536, audioThread, NULL, 0, 0);
	}
}

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples)) {
	a2_callback = kinc_a2_audio_callback;
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
#endif

#endif

#ifdef __cplusplus
}
#endif
