#include "pch.h"
#include <Kore/Audio/Audio.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <string.h>

using namespace Kore;

namespace {
	SLObjectItf engineObject;
	SLEngineItf engineEngine;
	SLObjectItf outputMixObject;
	SLObjectItf bqPlayerObject;
	SLPlayItf bqPlayerPlay = NULL;
	SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
	const int bufferSize = 1 * 1024;
	s16 tempBuffer[bufferSize];

	void copySample(void* buffer) {
		float value = *(float*)&Audio::buffer.data[Audio::buffer.readLocation];
		Audio::buffer.readLocation += 4;
		if (Audio::buffer.readLocation >= Audio::buffer.dataSize) Audio::buffer.readLocation = 0;
		*(s16*)buffer = static_cast<s16>(value * 32767);
	}

	void bqPlayerCallback(SLAndroidSimpleBufferQueueItf caller, void* context) {
		if (Kore::Audio::audioCallback != nullptr) {
			Kore::Audio::audioCallback(bufferSize);
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

void Kore::Audio::init() {
	buffer.readLocation = 0;
	buffer.writeLocation = 0;
	buffer.dataSize = 128 * 1024;
	buffer.data = new u8[buffer.dataSize];

	SLresult result;
	result = slCreateEngine(&engineObject, 0, nullptr, 0, nullptr, nullptr);
	result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
	result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

	const SLInterfaceID ids[] = { SL_IID_VOLUME };
	const SLboolean req[] = { SL_BOOLEAN_FALSE };
	result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2 };
	SLDataFormat_PCM format_pcm = { SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN };
	SLDataSource audioSrc = { &loc_bufq, &format_pcm };

	SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, outputMixObject };
	SLDataSink audioSnk = { &loc_outmix, nullptr };

	const SLInterfaceID ids1[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
	const SLboolean req1[] = { SL_BOOLEAN_TRUE };
	result = (*engineEngine)->CreateAudioPlayer(engineEngine, &(bqPlayerObject), &audioSrc, &audioSnk, 1, ids1, req1);
	result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY,&(bqPlayerPlay));

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

void Kore::Audio::update() {

}

void Kore::Audio::shutdown() {
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
