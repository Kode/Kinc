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
#include <android_native_app_glue.h>
#if SYS_ANDROID_API >= 15
#include <OMXAL/OpenMAXAL.h>
#include <OMXAL/OpenMAXAL_Android.h>
#endif
#include <Kore/Android.h>
#include <jni.h>
#include <pthread.h>
#include <assert.h>
#if SYS_ANDROID_API >= 15
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

using namespace Kore;

VideoSoundStream::VideoSoundStream(int nChannels, int freq) : bufferSize(1), bufferReadPosition(0), bufferWritePosition(0), read(0), written(0) {

}

void VideoSoundStream::insertData(float* data, int nSamples) {
	
}

float VideoSoundStream::nextSample() {
	return 0;
}

bool VideoSoundStream::ended() {
	return false;
}

#if SYS_ANDROID_API >= 15

namespace {
	Video* videos[] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
	
	#define NB_MAXAL_INTERFACES 3 // XAAndroidBufferQueueItf, XAStreamInformationItf and XAPlayItf
	#define NB_BUFFERS 8
	#define MPEG2_TS_PACKET_SIZE 188
	#define PACKETS_PER_BUFFER 10
	#define BUFFER_SIZE (PACKETS_PER_BUFFER*MPEG2_TS_PACKET_SIZE)
	const int kEosBufferCntxt = 1980; // a magic value we can compare against
	
	class AndroidVideo {
	public:
		AndroidVideo();
		bool enqueueInitialBuffers(bool discontinuity);
		bool init(const char* filename);
		void shutdown();
	public:
		XAObjectItf engineObject;
		XAEngineItf engineEngine;
		XAObjectItf outputMixObject;
		const char* path;
		//FILE* file = NULL;
		AAsset* file;
		XAObjectItf             playerObj;
		XAPlayItf               playerPlayItf;
		XAAndroidBufferQueueItf playerBQItf;
		XAStreamInformationItf  playerStreamInfoItf;
		XAVolumeItf             playerVolItf;
		char dataCache[BUFFER_SIZE * NB_BUFFERS];
		ANativeWindow* theNativeWindow;
		jboolean reachedEof;
		pthread_mutex_t mutex;
		pthread_cond_t cond;
		bool discontinuity;
	};
	
	AndroidVideo::AndroidVideo() {
		engineObject = NULL;
		engineEngine = NULL;
		outputMixObject = NULL;
		file = NULL;
		playerObj = NULL;
		playerPlayItf = NULL;
		playerBQItf = NULL;
		playerStreamInfoItf = NULL;
		playerVolItf = NULL;
		theNativeWindow = NULL;
		reachedEof = JNI_FALSE;
		mutex = PTHREAD_MUTEX_INITIALIZER;
		cond = PTHREAD_COND_INITIALIZER;
		discontinuity = false;
	}

	bool AndroidVideo::enqueueInitialBuffers(bool discontinuity) {
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
        AndroidVideo* self = (AndroidVideo*)pCallbackContext;
		XAresult res;
	    int ok;

	    // pCallbackContext was specified as NULL at RegisterCallback and is unused here
	    //assert(NULL == pCallbackContext);

	    // note there is never any contention on this mutex unless a discontinuity request is active
	    ok = pthread_mutex_lock(&self->mutex);
	    assert(0 == ok);

	    // was a discontinuity requested?
	    if (self->discontinuity) {
	        // Note: can't rewind after EOS, which we send when reaching EOF
	        // (don't send EOS if you plan to play more content through the same player)
	        if (!self->reachedEof) {
	            // clear the buffer queue
	            res = (*self->playerBQItf)->Clear(self->playerBQItf);
	            assert(XA_RESULT_SUCCESS == res);
	            // rewind the data source so we are guaranteed to be at an appropriate point
	            //rewind(file);
	            AAsset_seek(self->file, 0, SEEK_SET);
	            // Enqueue the initial buffers, with a discontinuity indicator on first buffer
	            (void) self->enqueueInitialBuffers(JNI_TRUE);
	        }
	        // acknowledge the discontinuity request
	        self->discontinuity = JNI_FALSE;
	        ok = pthread_cond_signal(&self->cond);
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
	    assert(self->dataCache <= (char *) pBufferData && (char *) pBufferData <
	            &self->dataCache[BUFFER_SIZE * NB_BUFFERS]);
	    assert(0 == (((char *) pBufferData - self->dataCache) % BUFFER_SIZE));

	    // don't bother trying to read more data once we've hit EOF
	    if (self->reachedEof) {
	        goto exit;
	    }

	    size_t nbRead;
	    // note we do call fread from multiple threads, but never concurrently
	    size_t bytesRead;
	    //bytesRead = fread(pBufferData, 1, BUFFER_SIZE, file);
	    bytesRead = AAsset_read(self->file, pBufferData, BUFFER_SIZE);
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
	        self->reachedEof = JNI_TRUE;
	    }

	exit:
	    ok = pthread_mutex_unlock(&self->mutex);
	    assert(0 == ok);
	    return XA_RESULT_SUCCESS;
	}

	void StreamChangeCallback(XAStreamInformationItf caller,
        XAuint32 eventId,
        XAuint32 streamIndex,
        void* pEventData,
        void* pContext) {
		Kore::log(Kore::Info, "StreamChangeCallback called for stream %u", streamIndex);
		AndroidVideo* self = (AndroidVideo*)pContext;
	    // pContext was specified as NULL at RegisterStreamChangeCallback and is unused here
	    //assert(NULL == pContext);
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

	bool AndroidVideo::init(const char* filename) {
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

	    // open the file to play
	    file = AAssetManager_open(KoreAndroid::getAssetManager(), filename, AASSET_MODE_STREAMING);
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
	    res = (*playerBQItf)->RegisterCallback(playerBQItf, AndroidBufferQueueCallback, this);
	    assert(XA_RESULT_SUCCESS == res);

	    // we want to be notified of the video size once it's found, so we register a callback for that
	    res = (*playerStreamInfoItf)->RegisterStreamChangeCallback(playerStreamInfoItf,
	            StreamChangeCallback, this);
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


	void AndroidVideo::shutdown() {
	    // destroy streaming media player object, and invalidate all associated interfaces
	    if (playerObj != NULL) {
	        (*playerObj)->Destroy(playerObj);
	        playerObj = NULL;
	        playerPlayItf = NULL;
	        playerBQItf = NULL;
	        playerStreamInfoItf = NULL;
	        playerVolItf = NULL;
	    }

	    // destroy output mix object, and invalidate all associated interfaces
	    if (outputMixObject != NULL) {
	        (*outputMixObject)->Destroy(outputMixObject);
	        outputMixObject = NULL;
	    }

	    // destroy engine object, and invalidate all associated interfaces
	    if (engineObject != NULL) {
	        (*engineObject)->Destroy(engineObject);
	        engineObject = NULL;
	        engineEngine = NULL;
	    }

	    // close the file
	    if (file != NULL) {
	        AAsset_close(file);
	        file = NULL;
	    }

	    // make sure we don't leak native windows
	    if (theNativeWindow != NULL) {
	        ANativeWindow_release(theNativeWindow);
	        theNativeWindow = NULL;
	    }
	}
}

#endif

extern "C" JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreMoviePlayer_nativeCreate(JNIEnv *env, jobject jobj, jstring jpath, jobject surface, jint id) {
#if SYS_ANDROID_API >= 15
	const char* path = env->GetStringUTFChars(jpath, NULL);
	AndroidVideo* av = new AndroidVideo;
	av->theNativeWindow = ANativeWindow_fromSurface(env, surface);
	av->init(path);
	for (int i = 0; i < 10; ++i) {
		if (videos[i] != nullptr && videos[i]->id == id) {
			videos[i]->androidVideo = av;
			break;
		}
	}
	env->ReleaseStringUTFChars(jpath, path);
#endif
}

Video::Video(const char* filename) : playing(false), sound(nullptr) {
#if SYS_ANDROID_API >= 15
	Kore::log(Kore::Info, "Opening video %s.", filename);
	myWidth = 1023;
	myHeight = 684;

	next = 0;
	audioTime = 0;

	JNIEnv* env;
	KoreAndroid::getActivity()->vm->AttachCurrentThread(&env, NULL);
	jclass koreMoviePlayerClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreMoviePlayer");
	jmethodID constructor = env->GetMethodID(koreMoviePlayerClass, "<init>", "(Ljava/lang/String;)V");
	jobject object = env->NewObject(koreMoviePlayerClass, constructor, env->NewStringUTF(filename));

	jmethodID getId = env->GetMethodID(koreMoviePlayerClass, "getId", "()I");
	id = env->CallIntMethod(object, getId);

	for (int i = 0; i < 10; ++i) {
		if (videos[i] == nullptr) {
			videos[i] = this;
			break;
		}
	}

	jmethodID jinit = env->GetMethodID(koreMoviePlayerClass, "init", "()V");
	env->CallVoidMethod(object, jinit);

	jmethodID getTextureId = env->GetMethodID(koreMoviePlayerClass, "getTextureId", "()I");
	int texid = env->CallIntMethod(object, getTextureId);

	KoreAndroid::getActivity()->vm->DetachCurrentThread();

	image = new Texture(texid);
#endif
}

Video::~Video() {
#if SYS_ANDROID_API >= 15
	stop();
	AndroidVideo* av = (AndroidVideo*)androidVideo;
	av->shutdown();
	for (int i = 0; i < 10; ++i) {
		if (videos[i] == this) {
			videos[i] = nullptr;
			break;
		}
	}
#endif
}

void Video::play() {
#if SYS_ANDROID_API >= 15
	playing = true;
	start = System::time();
#endif
}

void Video::pause() {
#if SYS_ANDROID_API >= 15
	playing = false;
#endif
}

void Video::stop() {
#if SYS_ANDROID_API >= 15
	pause();
#endif
}

void Video::updateImage() {
	
}

void Video::update(double time) {
	
}

int Video::width() {
#if SYS_ANDROID_API >= 15
	return myWidth;
#else
	return 512;
#endif
}

int Video::height() {
#if SYS_ANDROID_API >= 15
	return myHeight;
#else
	return 512;
#endif
}

Texture* Video::currentImage() {
#if SYS_ANDROID_API >= 15
	return image;
#else
	return nullptr;
#endif
}
