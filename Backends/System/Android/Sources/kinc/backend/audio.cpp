#include "pch.h"

#include <kinc/audio2/audio.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <string.h>

namespace {
	void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples) = nullptr;
	kinc_a2_buffer_t a2_buffer;

	SLObjectItf engineObject;
	SLEngineItf engineEngine;
	SLObjectItf outputMixObject;
	SLObjectItf bqPlayerObject;
	SLPlayItf bqPlayerPlay = NULL;
	SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
	const int bufferSize = 1 * 1024;
	int16_t tempBuffer[bufferSize];

	void copySample(void* buffer) {
		float value = *(float*)&a2_buffer.data[a2_buffer.read_location];
		a2_buffer.read_location += 4;
		if (a2_buffer.read_location >= a2_buffer.data_size) a2_buffer.read_location = 0;
		*(int16_t*)buffer = static_cast<int16_t>(value * 32767);
	}

	void bqPlayerCallback(SLAndroidSimpleBufferQueueItf caller, void* context) {
		if (a2_callback != nullptr) {
			a2_callback(&a2_buffer, bufferSize);
			for (int i = 0; i < bufferSize; i += 1) {
				copySample(&tempBuffer[i]);
			}
			SLresult result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, tempBuffer, bufferSize * 2);
		}
		else {
			memset(tempBuffer, 0, sizeof(tempBuffer));
			SLresult result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, tempBuffer, bufferSize * 2);
		}
	}
}

void kinc_a2_init() {
	kinc_a2_samples_per_second = 44100;
	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = new uint8_t[a2_buffer.data_size];

	SLresult result;
	result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
	result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
	result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

	const SLInterfaceID ids[] = {SL_IID_VOLUME};
	const SLboolean req[] = {SL_BOOLEAN_FALSE};
	result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
	SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM,           2,
	                               SL_SAMPLINGRATE_44_1,        SL_PCMSAMPLEFORMAT_FIXED_16,
	                               SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
	                               SL_BYTEORDER_LITTLEENDIAN};
	SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
	SLDataSink audioSnk = {&loc_outmix, nullptr};

	const SLInterfaceID ids1[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
	const SLboolean req1[] = {SL_BOOLEAN_TRUE};
	result = (*engineEngine)->CreateAudioPlayer(engineEngine, &(bqPlayerObject), &audioSrc, &audioSnk, 1, ids1, req1);
	result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &(bqPlayerPlay));

	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &(bqPlayerBufferQueue));

	result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, nullptr);

	result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

	memset(tempBuffer, 0, sizeof(tempBuffer));
	result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, tempBuffer, bufferSize * 2);
}

void pauseAudio() {
	if (bqPlayerPlay == NULL) return;
	SLresult result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
}

void resumeAudio() {
	if (bqPlayerPlay == NULL) return;
	SLresult result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
}

void kinc_a2_update() {}

void kinc_a2_shutdown() {
	if (bqPlayerObject != nullptr) {
		(*bqPlayerObject)->Destroy(bqPlayerObject);
		bqPlayerObject = nullptr;
		bqPlayerPlay = nullptr;
		bqPlayerBufferQueue = nullptr;
	}
	if (outputMixObject != nullptr) {
		(*outputMixObject)->Destroy(outputMixObject);
		outputMixObject = nullptr;
	}
	if (engineObject != nullptr) {
		(*engineObject)->Destroy(engineObject);
		engineObject = nullptr;
		engineEngine = nullptr;
	}
}

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples)) {
	a2_callback = kinc_a2_audio_callback;
}
