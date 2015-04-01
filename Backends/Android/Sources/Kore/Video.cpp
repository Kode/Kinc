#include "pch.h"
#include "Video.h"
#include <Kore/IO/FileReader.h>
#include <Kore/Graphics/Texture.h>
#include <Kore/System.h>
#include <Kore/VideoSoundStream.h>
#include <Kore/Audio/Mixer.h>
#include <Kore/Log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <OMXAL/OpenMAXAL.h>
#include <OMXAL/OpenMAXAL_Android.h>
#include <jni.h>
#include <android/native_window_jni.h>
#include <pthread.h>
#include <assert.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

using namespace Kore;

AAssetManager* getAssetManager();

namespace {
	int maxSamples = 0;
}

VideoSoundStream::VideoSoundStream(int nChannels, int freq) : bufferSize(1024 * 100 * 10), bufferReadPosition(0), bufferWritePosition(0), read(0), written(0) {
	buffer = new float[bufferSize];
}

void VideoSoundStream::insertData(float* data, int nSamples) {
	//printf("Samples: %i\n", nSamples);
	if (nSamples > maxSamples) {
		printf("Samples: %i\n", nSamples);
		maxSamples = nSamples;
	}
	written += nSamples;
	for (int i = 0; i < nSamples; ++i) {
		float value = data[i];// / 32767.0;
		buffer[bufferWritePosition++] = value;
		if (bufferWritePosition >= bufferSize) {
			bufferWritePosition = 0;
			printf("buffer write back\n");
		}
	}
}

float VideoSoundStream::nextSample() {
	++read;
	if (written <= read) {
		//printf("Out of audio\n");
		return 0;
	}
	if (bufferReadPosition >= bufferSize) {
		bufferReadPosition = 0;
		printf("buffer read back - %i\n", (int)(written - read));
	}
	return buffer[bufferReadPosition++];
}

bool VideoSoundStream::ended() {
	return false;
}

namespace {
	XAObjectItf engineObject = NULL;
	XAEngineItf engineEngine = NULL;
	XAObjectItf outputMixObject = NULL;
	const char* path;
	//FILE* file = NULL;
	AAsset* file = NULL;
	#define NB_MAXAL_INTERFACES 3 // XAAndroidBufferQueueItf, XAStreamInformationItf and XAPlayItf
	#define NB_BUFFERS 8
	XAObjectItf             playerObj = NULL;
	XAPlayItf               playerPlayItf = NULL;
	XAAndroidBufferQueueItf playerBQItf = NULL;
	XAStreamInformationItf  playerStreamInfoItf = NULL;
	XAVolumeItf             playerVolItf = NULL;
	#define MPEG2_TS_PACKET_SIZE 188
	#define PACKETS_PER_BUFFER 10
	#define BUFFER_SIZE (PACKETS_PER_BUFFER*MPEG2_TS_PACKET_SIZE)
	char dataCache[BUFFER_SIZE * NB_BUFFERS];
	void* theNativeWindow = NULL;
	jboolean reachedEof = JNI_FALSE;
	const int kEosBufferCntxt = 1980; // a magic value we can compare against
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	bool discontinuity = false;

	bool enqueueInitialBuffers(bool discontinuity) {

	    /* Fill our cache.
	     * We want to read whole packets (integral multiples of MPEG2_TS_PACKET_SIZE).
	     * fread returns units of "elements" not bytes, so we ask for 1-byte elements
	     * and then check that the number of elements is a multiple of the packet size.
	     */
	    size_t bytesRead;
	    //bytesRead = fread(dataCache, 1, BUFFER_SIZE * NB_BUFFERS, file);
	    bytesRead = AAsset_read(file, dataCache, BUFFER_SIZE * NB_BUFFERS);
	    if (bytesRead <= 0) {
	        // could be premature EOF or I/O error
	        return false;
	    }
	    if ((bytesRead % MPEG2_TS_PACKET_SIZE) != 0) {
	        Kore::log(Kore::Info, "Dropping last packet because it is not whole");
	    }
	    size_t packetsRead = bytesRead / MPEG2_TS_PACKET_SIZE;
	    Kore::log(Kore::Info, "Initially queueing %zu packets", packetsRead);

	    /* Enqueue the content of our cache before starting to play,
	       we don't want to starve the player */
	    size_t i;
	    for (i = 0; i < NB_BUFFERS && packetsRead > 0; i++) {
	        // compute size of this buffer
	        size_t packetsThisBuffer = packetsRead;
	        if (packetsThisBuffer > PACKETS_PER_BUFFER) {
	            packetsThisBuffer = PACKETS_PER_BUFFER;
	        }
	        size_t bufferSize = packetsThisBuffer * MPEG2_TS_PACKET_SIZE;
	        XAresult res;
	        if (discontinuity) {
	            // signal discontinuity
	            XAAndroidBufferItem items[1];
	            items[0].itemKey = XA_ANDROID_ITEMKEY_DISCONTINUITY;
	            items[0].itemSize = 0;
	            // DISCONTINUITY message has no parameters,
	            //   so the total size of the message is the size of the key
	            //   plus the size if itemSize, both XAuint32
	            res = (*playerBQItf)->Enqueue(playerBQItf, NULL /*pBufferContext*/,
	                    dataCache + i*BUFFER_SIZE, bufferSize, items /*pMsg*/,
	                    sizeof(XAuint32)*2 /*msgLength*/);
	            discontinuity = JNI_FALSE;
	        } else {
	            res = (*playerBQItf)->Enqueue(playerBQItf, NULL /*pBufferContext*/,
	                    dataCache + i*BUFFER_SIZE, bufferSize, NULL, 0);
	        }
	        assert(XA_RESULT_SUCCESS == res);
	        packetsRead -= packetsThisBuffer;
	    }

	    return true;
	}

	XAresult AndroidBufferQueueCallback(
        XAAndroidBufferQueueItf caller,
        void *pCallbackContext,        /* input */
        void *pBufferContext,          /* input */
        void *pBufferData,             /* input */
        XAuint32 dataSize,             /* input */
        XAuint32 dataUsed,             /* input */
        const XAAndroidBufferItem *pItems,/* input */
        XAuint32 itemsLength           /* input */) {
		XAresult res;
	    int ok;

	    // pCallbackContext was specified as NULL at RegisterCallback and is unused here
	    assert(NULL == pCallbackContext);

	    // note there is never any contention on this mutex unless a discontinuity request is active
	    ok = pthread_mutex_lock(&mutex);
	    assert(0 == ok);

	    // was a discontinuity requested?
	    if (discontinuity) {
	        // Note: can't rewind after EOS, which we send when reaching EOF
	        // (don't send EOS if you plan to play more content through the same player)
	        if (!reachedEof) {
	            // clear the buffer queue
	            res = (*playerBQItf)->Clear(playerBQItf);
	            assert(XA_RESULT_SUCCESS == res);
	            // rewind the data source so we are guaranteed to be at an appropriate point
	            //rewind(file);
	            AAsset_seek(file, 0, SEEK_SET);
	            // Enqueue the initial buffers, with a discontinuity indicator on first buffer
	            (void) enqueueInitialBuffers(JNI_TRUE);
	        }
	        // acknowledge the discontinuity request
	        discontinuity = JNI_FALSE;
	        ok = pthread_cond_signal(&cond);
	        assert(0 == ok);
	        goto exit;
	    }

	    if ((pBufferData == NULL) && (pBufferContext != NULL)) {
	        const int processedCommand = *(int *)pBufferContext;
	        if (kEosBufferCntxt == processedCommand) {
	            Kore::log(Kore::Info, "EOS was processed");
	            // our buffer with the EOS message has been consumed
	            assert(0 == dataSize);
	            goto exit;
	        }
	    }

	    // pBufferData is a pointer to a buffer that we previously Enqueued
	    assert((dataSize > 0) && ((dataSize % MPEG2_TS_PACKET_SIZE) == 0));
	    assert(dataCache <= (char *) pBufferData && (char *) pBufferData <
	            &dataCache[BUFFER_SIZE * NB_BUFFERS]);
	    assert(0 == (((char *) pBufferData - dataCache) % BUFFER_SIZE));

	    // don't bother trying to read more data once we've hit EOF
	    if (reachedEof) {
	        goto exit;
	    }

	    size_t nbRead;
	    // note we do call fread from multiple threads, but never concurrently
	    size_t bytesRead;
	    //bytesRead = fread(pBufferData, 1, BUFFER_SIZE, file);
	    bytesRead = AAsset_read(file, pBufferData, BUFFER_SIZE);
	    if (bytesRead > 0) {
	        if ((bytesRead % MPEG2_TS_PACKET_SIZE) != 0) {
	            Kore::log(Kore::Info, "Dropping last packet because it is not whole");
	        }
	        size_t packetsRead = bytesRead / MPEG2_TS_PACKET_SIZE;
	        size_t bufferSize = packetsRead * MPEG2_TS_PACKET_SIZE;
	        res = (*caller)->Enqueue(caller, NULL /*pBufferContext*/,
	                pBufferData /*pData*/,
	                bufferSize /*dataLength*/,
	                NULL /*pMsg*/,
	                0 /*msgLength*/);
	        assert(XA_RESULT_SUCCESS == res);
	    } else {
	        // EOF or I/O error, signal EOS
	        XAAndroidBufferItem msgEos[1];
	        msgEos[0].itemKey = XA_ANDROID_ITEMKEY_EOS;
	        msgEos[0].itemSize = 0;
	        // EOS message has no parameters, so the total size of the message is the size of the key
	        //   plus the size if itemSize, both XAuint32
	        res = (*caller)->Enqueue(caller, (void *)&kEosBufferCntxt /*pBufferContext*/,
	                NULL /*pData*/, 0 /*dataLength*/,
	                msgEos /*pMsg*/,
	                sizeof(XAuint32)*2 /*msgLength*/);
	        assert(XA_RESULT_SUCCESS == res);
	        reachedEof = JNI_TRUE;
	    }

	exit:
	    ok = pthread_mutex_unlock(&mutex);
	    assert(0 == ok);
	    return XA_RESULT_SUCCESS;
	}

	void StreamChangeCallback(XAStreamInformationItf caller,
        XAuint32 eventId,
        XAuint32 streamIndex,
        void * pEventData,
        void * pContext) {
		Kore::log(Kore::Info, "StreamChangeCallback called for stream %u", streamIndex);
	    // pContext was specified as NULL at RegisterStreamChangeCallback and is unused here
	    assert(NULL == pContext);
	    switch (eventId) {
	      case XA_STREAMCBEVENT_PROPERTYCHANGE: {
	        /** From spec 1.0.1:
	            "This event indicates that stream property change has occurred.
	            The streamIndex parameter identifies the stream with the property change.
	            The pEventData parameter for this event is not used and shall be ignored."
	         */

	        XAresult res;
	        XAuint32 domain;
	        res = (*caller)->QueryStreamType(caller, streamIndex, &domain);
	        assert(XA_RESULT_SUCCESS == res);
	        switch (domain) {
	          case XA_DOMAINTYPE_VIDEO: {
	            XAVideoStreamInformation videoInfo;
	            res = (*caller)->QueryStreamInformation(caller, streamIndex, &videoInfo);
	            assert(XA_RESULT_SUCCESS == res);
	            Kore::log(Kore::Info, "Found video size %u x %u, codec ID=%u, frameRate=%u, bitRate=%u, duration=%u ms",
	                        videoInfo.width, videoInfo.height, videoInfo.codecId, videoInfo.frameRate,
	                        videoInfo.bitRate, videoInfo.duration);
	          } break;
	          default:
	            Kore::log(Kore::Error, "Unexpected domain %u\n", domain);
	            break;
	        }
	      } break;
	      default:
	        Kore::log(Kore::Error, "Unexpected stream event ID %u\n", eventId);
	        break;
	    }
	}

	bool init() {
		XAresult res;

		// create engine
		res = xaCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
		assert(XA_RESULT_SUCCESS == res);

		// realize the engine
		res = (*engineObject)->Realize(engineObject, XA_BOOLEAN_FALSE);
		assert(XA_RESULT_SUCCESS == res);

		// get the engine interface, which is needed in order to create other objects
		res = (*engineObject)->GetInterface(engineObject, XA_IID_ENGINE, &engineEngine);
		assert(XA_RESULT_SUCCESS == res);

		// create output mix
		res = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
		assert(XA_RESULT_SUCCESS == res);

		// realize the output mix
		res = (*outputMixObject)->Realize(outputMixObject, XA_BOOLEAN_FALSE);
		assert(XA_RESULT_SUCCESS == res);





		//XAresult res;

	    // convert Java string to UTF-8
	    //const char *utf8 = (*env)->GetStringUTFChars(env, filename, NULL);
	    //assert(NULL != utf8);
		const char* utf8 = "NativeMedia.ts";
	    // open the file to play
	    //file = fopen(utf8, "rb");
	    file = AAssetManager_open(getAssetManager(), utf8, 0);
	    if (file == NULL) {
	    	Kore::log(Kore::Info, "Could not find video file.");
	        return false;
	    }

	    // configure data source
	    XADataLocator_AndroidBufferQueue loc_abq = { XA_DATALOCATOR_ANDROIDBUFFERQUEUE, NB_BUFFERS };
	    XADataFormat_MIME format_mime = {
	            XA_DATAFORMAT_MIME, XA_ANDROID_MIME_MP2TS, XA_CONTAINERTYPE_MPEG_TS };
	    XADataSource dataSrc = {&loc_abq, &format_mime};

	    // configure audio sink
	    XADataLocator_OutputMix loc_outmix = { XA_DATALOCATOR_OUTPUTMIX, outputMixObject };
	    XADataSink audioSnk = { &loc_outmix, NULL };

	    // configure image video sink
	    XADataLocator_NativeDisplay loc_nd = {
	            XA_DATALOCATOR_NATIVEDISPLAY,        // locatorType
	            // the video sink must be an ANativeWindow created from a Surface or SurfaceTexture
	            (void*)theNativeWindow,              // hWindow
	            // must be NULL
	            NULL                                 // hDisplay
	    };
	    XADataSink imageVideoSink = {&loc_nd, NULL};

	    // declare interfaces to use
	    XAboolean     required[NB_MAXAL_INTERFACES]
	                           = {XA_BOOLEAN_TRUE, XA_BOOLEAN_TRUE,           XA_BOOLEAN_TRUE};
	    XAInterfaceID iidArray[NB_MAXAL_INTERFACES]
	                           = {XA_IID_PLAY,     XA_IID_ANDROIDBUFFERQUEUESOURCE,
	                                               XA_IID_STREAMINFORMATION};

	    // create media player
	    res = (*engineEngine)->CreateMediaPlayer(engineEngine, &playerObj, &dataSrc,
	            NULL, &audioSnk, &imageVideoSink, NULL, NULL,
	            NB_MAXAL_INTERFACES /*XAuint32 numInterfaces*/,
	            iidArray /*const XAInterfaceID *pInterfaceIds*/,
	            required /*const XAboolean *pInterfaceRequired*/);
	    assert(XA_RESULT_SUCCESS == res);

	    // release the Java string and UTF-8
	    //(*env)->ReleaseStringUTFChars(env, filename, utf8);

	    // realize the player
	    res = (*playerObj)->Realize(playerObj, XA_BOOLEAN_FALSE);
	    assert(XA_RESULT_SUCCESS == res);

	    // get the play interface
	    res = (*playerObj)->GetInterface(playerObj, XA_IID_PLAY, &playerPlayItf);
	    assert(XA_RESULT_SUCCESS == res);

	    // get the stream information interface (for video size)
	    res = (*playerObj)->GetInterface(playerObj, XA_IID_STREAMINFORMATION, &playerStreamInfoItf);
	    assert(XA_RESULT_SUCCESS == res);

	    // get the volume interface
	    res = (*playerObj)->GetInterface(playerObj, XA_IID_VOLUME, &playerVolItf);
	    assert(XA_RESULT_SUCCESS == res);

	    // get the Android buffer queue interface
	    res = (*playerObj)->GetInterface(playerObj, XA_IID_ANDROIDBUFFERQUEUESOURCE, &playerBQItf);
	    assert(XA_RESULT_SUCCESS == res);

	    // specify which events we want to be notified of
	    res = (*playerBQItf)->SetCallbackEventsMask(playerBQItf, XA_ANDROIDBUFFERQUEUEEVENT_PROCESSED);
	    assert(XA_RESULT_SUCCESS == res);

	    // register the callback from which OpenMAX AL can retrieve the data to play
	    res = (*playerBQItf)->RegisterCallback(playerBQItf, AndroidBufferQueueCallback, NULL);
	    assert(XA_RESULT_SUCCESS == res);

	    // we want to be notified of the video size once it's found, so we register a callback for that
	    res = (*playerStreamInfoItf)->RegisterStreamChangeCallback(playerStreamInfoItf,
	            StreamChangeCallback, NULL);
	    assert(XA_RESULT_SUCCESS == res);

	    // enqueue the initial buffers
	    if (!enqueueInitialBuffers(false)) {
	    	Kore::log(Kore::Info, "Could not enqueue initial buffers for video decoding.");
	        return false;
	    }

	    // prepare the player
	    res = (*playerPlayItf)->SetPlayState(playerPlayItf, XA_PLAYSTATE_PAUSED);
	    assert(XA_RESULT_SUCCESS == res);

	    // set the volume
	    res = (*playerVolItf)->SetVolumeLevel(playerVolItf, 0);
	    assert(XA_RESULT_SUCCESS == res);

		// start the playback
		res = (*playerPlayItf)->SetPlayState(playerPlayItf, XA_PLAYSTATE_PLAYING);
		assert(XA_RESULT_SUCCESS == res);

		Kore::log(Kore::Info, "Successfully loaded video.");

		return true;
	}
}

extern "C" JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreMoviePlayer_nativeCreate(JNIEnv *env, jobject jobj, jstring jpath, jobject surface)
{
    path = env->GetStringUTFChars(jpath, NULL);
    theNativeWindow = ANativeWindow_fromSurface(env, surface);
    init();
    //env->ReleaseStringUTFChars(jpath, path);
}

JNIEnv* getEnv();

Video::Video(const char* filename) : playing(false), sound(nullptr) {
	char name[2048];
	strcpy(name, "android"); //iphonegetresourcepath());
	strcat(name, "/");
	strcat(name, filename);

	//init();	

	image = nullptr;
	next = 0;
	audioTime = 0;

	jclass cls = getEnv()->FindClass("com/ktxsoftware/kore/KoreMoviePlayer");
	jmethodID constructor = getEnv()->GetMethodID(cls, "<init>", "(Ljava/lang/String;)V");
	jobject object = getEnv()->NewObject(cls, constructor, getEnv()->NewStringUTF("NativeMedia.ts"));

	jmethodID getTextureId = getEnv()->GetMethodID(cls, "getTextureId", "()I");
	int texid = getEnv()->CallIntMethod(object, getTextureId);

	image = new Texture(texid);
}

Video::~Video() {
	stop();
}

void Video::play() {
	
	sound = new VideoSoundStream(2, 44100);
	Mixer::play(sound);
	
	playing = true;
	start = System::time();
}

void Video::pause() {
	playing = false;
	if (sound != nullptr) {
		Mixer::stop(sound);
		delete sound;
		sound = nullptr;
	}
}

void Video::stop() {
	pause();
}

void Video::updateImage() {
	if (!playing) return;
	{
		
	}
	
	//printf("Next %f\n", next);
	
	{
		
	}
}

void Video::update(double time) {
	if (playing && time >= start + next) {
		updateImage();
	}
}

int Video::width() {
	return myWidth;
}

int Video::height() {
	return myHeight;
}

Texture* Video::currentImage() {
	update(System::time());
	return image;
}
