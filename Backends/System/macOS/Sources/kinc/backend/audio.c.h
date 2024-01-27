#include <CoreAudio/AudioHardware.h>
#include <CoreServices/CoreServices.h>

#include <kinc/audio2/audio.h>
#include <kinc/backend/video.h>
#include <kinc/log.h>

#include <stdio.h>

static kinc_internal_video_sound_stream_t *video = NULL;

void macPlayVideoSoundStream(kinc_internal_video_sound_stream_t *v) {
	video = v;
}

void macStopVideoSoundStream(void) {
	video = NULL;
}

// const int samplesPerSecond = 44100;

static void affirm(OSStatus err) {
	if (err != kAudioHardwareNoError) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Error: %i\n", err);
	}
}

static bool initialized;
static bool soundPlaying;
static AudioDeviceID device;
static UInt32 deviceBufferSize;
static UInt32 size;
static AudioStreamBasicDescription deviceFormat;
static AudioObjectPropertyAddress address;

static AudioDeviceIOProcID theIOProcID = NULL;

static void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples, void *userdata) = NULL;
static void *a2_userdata = NULL;
static void (*a2_sample_rate_callback)(void *userdata) = NULL;
static void *a2_sample_rate_userdata = NULL;
static kinc_a2_buffer_t a2_buffer;

static void copySample(void *buffer) {
	float value = *(float *)&a2_buffer.data[a2_buffer.read_location];
	a2_buffer.read_location += 4;
	if (a2_buffer.read_location >= a2_buffer.data_size)
		a2_buffer.read_location = 0;
	*(float *)buffer = value;
}

static OSStatus appIOProc(AudioDeviceID inDevice, const AudioTimeStamp *inNow, const AudioBufferList *inInputData, const AudioTimeStamp *inInputTime,
                          AudioBufferList *outOutputData, const AudioTimeStamp *inOutputTime, void *userdata) {
	affirm(AudioObjectGetPropertyData(device, &address, 0, NULL, &size, &deviceFormat));
	if (kinc_a2_samples_per_second != (int)deviceFormat.mSampleRate) {
		kinc_a2_samples_per_second = (int)deviceFormat.mSampleRate;
		if (a2_sample_rate_callback != NULL) {
			a2_sample_rate_callback(a2_sample_rate_userdata);
		}
	}
	int numSamples = deviceBufferSize / deviceFormat.mBytesPerFrame;
	a2_callback(&a2_buffer, numSamples * 2, a2_userdata);
	float *out = (float *)outOutputData->mBuffers[0].mData;
	for (int i = 0; i < numSamples; ++i) {
		copySample(out++); // left
		copySample(out++); // right
	}
	return kAudioHardwareNoError;
}

static bool initialized = false;

void kinc_a2_init(void) {
	if (initialized) {
		return;
	}

	initialized = true;

	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = (uint8_t *)malloc(a2_buffer.data_size);

	device = kAudioDeviceUnknown;

	initialized = false;

	size = sizeof(AudioDeviceID);
	address.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
	address.mScope = kAudioObjectPropertyScopeGlobal;
	address.mElement = kAudioObjectPropertyElementMaster;
	affirm(AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, &size, &device));

	size = sizeof(UInt32);
	address.mSelector = kAudioDevicePropertyBufferSize;
	address.mScope = kAudioDevicePropertyScopeOutput;
	affirm(AudioObjectGetPropertyData(device, &address, 0, NULL, &size, &deviceBufferSize));

	kinc_log(KINC_LOG_LEVEL_INFO, "deviceBufferSize = %i\n", deviceBufferSize);

	size = sizeof(AudioStreamBasicDescription);
	address.mSelector = kAudioDevicePropertyStreamFormat;
	address.mScope = kAudioDevicePropertyScopeOutput;

	affirm(AudioObjectGetPropertyData(device, &address, 0, NULL, &size, &deviceFormat));

	if (deviceFormat.mFormatID != kAudioFormatLinearPCM) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "mFormatID !=  kAudioFormatLinearPCM\n");
		return;
	}

	if (!(deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsFloat)) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Only works with float format.\n");
		return;
	}

	kinc_a2_samples_per_second = (int)deviceFormat.mSampleRate;
	a2_buffer.format.samples_per_second = kinc_a2_samples_per_second;
	a2_buffer.format.bits_per_sample = 32;
	a2_buffer.format.channels = 2;

	initialized = true;

	kinc_log(KINC_LOG_LEVEL_INFO, "mSampleRate = %g\n", deviceFormat.mSampleRate);
	kinc_log(KINC_LOG_LEVEL_INFO, "mFormatFlags = %08X\n", (unsigned int)deviceFormat.mFormatFlags);
	kinc_log(KINC_LOG_LEVEL_INFO, "mBytesPerPacket = %d\n", (unsigned int)deviceFormat.mBytesPerPacket);
	kinc_log(KINC_LOG_LEVEL_INFO, "mFramesPerPacket = %d\n", (unsigned int)deviceFormat.mFramesPerPacket);
	kinc_log(KINC_LOG_LEVEL_INFO, "mChannelsPerFrame = %d\n", (unsigned int)deviceFormat.mChannelsPerFrame);
	kinc_log(KINC_LOG_LEVEL_INFO, "mBytesPerFrame = %d\n", (unsigned int)deviceFormat.mBytesPerFrame);
	kinc_log(KINC_LOG_LEVEL_INFO, "mBitsPerChannel = %d\n", (unsigned int)deviceFormat.mBitsPerChannel);

	if (soundPlaying)
		return;

	affirm(AudioDeviceCreateIOProcID(device, appIOProc, NULL, &theIOProcID));
	affirm(AudioDeviceStart(device, theIOProcID));

	soundPlaying = true;
}

void kinc_a2_update(void) {}

void kinc_a2_shutdown(void) {
	if (!initialized)
		return;
	if (!soundPlaying)
		return;

	affirm(AudioDeviceStop(device, theIOProcID));
	affirm(AudioDeviceDestroyIOProcID(device, theIOProcID));

	soundPlaying = false;
}

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples, void *userdata), void *userdata) {
	a2_callback = kinc_a2_audio_callback;
	a2_userdata = userdata;
}

void kinc_a2_set_sample_rate_callback(void (*kinc_a2_sample_rate_callback)(void *userdata), void *userdata) {
	a2_sample_rate_callback = kinc_a2_sample_rate_callback;
	a2_sample_rate_userdata = userdata;
}
