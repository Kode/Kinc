#include "pch.h"
#include <Kore/System.h>
#include <Kore/Application.h>
#include <Kore/Audio/Audio.h>
#include <Kore/IO/FileReader.h>
#include <Kore/IO/miniz.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Input/Sensor.h>
#include <Kore/Input/Surface.h>
#include <Kore/Log.h>
#include <jni.h>
#include <GLES2/gl2.h>
#include <cstring>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

// TODO: FM: Inconsistent namespaces
#include <Kore/Vr/VrInterface.h>

void* Kore::System::createWindow() {
	return nullptr;
}

void Kore::System::swapBuffers() {

}

void Kore::System::destroyWindow() {

}

void Kore::System::changeResolution(int, int, bool) {

}

void Kore::System::setTitle(const char*) {

}

void Kore::System::showWindow() {

}

namespace {
	mz_zip_archive apk;
	char theApkPath[1001];
	char filesDir[1001];
	int width;
	int height;
	bool keyboardShown = false;

	//Android GDB does not attach immediately after a native lib is loaded.
	//To debug startup behavior set the debuggingDelay to about 200.
	bool initialized = false;
	int debuggingDelayCount = 0;
	const int debuggingDelay = 0;
    AAssetManager* assets;
    JNIEnv* env;
}

JNIEnv* getEnv() {
    return env;
}

AAssetManager* getAssetManager() {
    return assets;
}

void Kore::System::showKeyboard() {
	keyboardShown = true;
}

void Kore::System::hideKeyboard() {
	keyboardShown = false;
}

void Kore::System::loadURL(const char* url) {
    
}

int Kore::System::screenWidth() {
    return width;
}

int Kore::System::screenHeight() {
    return height;
}

char* getApkPath() {
	return theApkPath;
}

mz_zip_archive* getApk() {
	return &apk;
}

const char* Kore::System::savePath() {
	return filesDir;
}

const char* Kore::System::systemId() {
	return "Android";
}

extern int kore(int argc, char** argv);

extern "C" {
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_init(JNIEnv* env, jobject obj, jint width, jint height, jobject assetManager, jstring apkPath, jstring filesDir);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_step(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_touch(JNIEnv* env, jobject obj, jint index, jint x, jint y, jint action);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_writeAudio(JNIEnv* env, jobject obj, jbyteArray buffer, jint size);
	JNIEXPORT bool JNICALL Java_com_ktxsoftware_kore_KoreLib_keyboardShown(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_keyUp(JNIEnv* env, jobject obj, jint code);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_keyDown(JNIEnv* env, jobject obj, jint code);
    JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_accelerometerChanged(JNIEnv* env, jobject obj, jfloat x, jfloat y, jfloat z);
    JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_gyroChanged(JNIEnv* env, jobject obj, jfloat x, jfloat y, jfloat z);
    JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_gaze(JNIEnv* env, jobject obj, jfloat x, jfloat y, jfloat z, jfloat w);
};

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_init(JNIEnv* env, jobject obj, jint width, jint height, jobject assetManager, jstring apkPath, jstring filesDir) {
	{
		const char* path = env->GetStringUTFChars(apkPath, nullptr);
		std::strcpy(theApkPath, path);
		env->ReleaseStringUTFChars(apkPath, path);
	}
	{
		const char* path = env->GetStringUTFChars(filesDir, nullptr);
		std::strcpy(::filesDir, path);
		std::strcat(::filesDir, "/");
		env->ReleaseStringUTFChars(filesDir, path);
	}

    //(*env)->NewGlobalRef(env, foo);
    assets = AAssetManager_fromJava(env, assetManager);

	glViewport(0, 0, width, height);
	::width = width;
	::height = height;

	// Save the JVM for the Vr system
	JavaVM* cachedJVM;
	env->GetJavaVM(&cachedJVM);
	Kore::VrInterface::SetJVM(cachedJVM);
}

namespace {
	void init() {
		kore(0, nullptr);
		initialized = true;
	}
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_step(JNIEnv* env, jobject obj) {
    ::env = env;
	if (!initialized) {
		++debuggingDelayCount;
		if (debuggingDelayCount > debuggingDelay) {
			memset(&apk, 0, sizeof(apk));
			mz_bool status = mz_zip_reader_init_file(&apk, theApkPath, 0);
			if (!status) {
				return;
			}
			init();
			Kore::Application::the()->callback();
		}
	}
	else {
		Kore::Application::the()->callback();
	}
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_touch(JNIEnv* env, jobject obj, jint index, jint x, jint y, jint action) {
    ::env = env;
	switch (action) {
	case 0: //DOWN
		if (index == 0) {
			Kore::Mouse::the()->_press(0, x, y);
		}
		Kore::Surface::the()->_touchStart(index, x, y);
		break;
	case 1: //MOVE
		if (index == 0) {
			Kore::Mouse::the()->_move(x, y);
		}
		Kore::Surface::the()->_move(index, x, y);
		break;
	case 2: //UP
		if (index == 0) {
			Kore::Mouse::the()->_release(0, x, y);
		}
		Kore::Surface::the()->_touchEnd(index, x, y);
		break;
	}
}

JNIEXPORT bool JNICALL Java_com_ktxsoftware_kore_KoreLib_keyboardShown(JNIEnv* env, jobject obj) {
	::env = env;
	return keyboardShown;
}

namespace {
	bool shift = false;
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_keyUp(JNIEnv* env, jobject obj, jint code) {
	::env = env;
	switch (code) {
	case 0x00000120:
		shift = false;
		Kore::Keyboard::the()->_keyup(Kore::Key_Shift, 0);
		break;
	case 0x00000103:
		Kore::Keyboard::the()->_keyup(Kore::Key_Backspace, 0);
		break;
	case 0x00000104:
		Kore::Keyboard::the()->_keyup(Kore::Key_Return, 0);
		break;
	default:
		if (shift)
			Kore::Keyboard::the()->_keyup((Kore::KeyCode)code, code);
		else
			Kore::Keyboard::the()->_keyup((Kore::KeyCode)(code + 'a' - 'A'), code + 'a' - 'A');
		break;
	}
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_keyDown(JNIEnv* env, jobject obj, jint code) {
	::env = env;
	switch (code) {
	case 0x00000120:
		shift = true;
		Kore::Keyboard::the()->_keydown(Kore::Key_Shift, 0);
		break;
	case 0x00000103:
		Kore::Keyboard::the()->_keydown(Kore::Key_Backspace, 0);
		break;
	case 0x00000104:
		Kore::Keyboard::the()->_keydown(Kore::Key_Return, 0);
		break;
	default:
		if (shift)
			Kore::Keyboard::the()->_keydown((Kore::KeyCode)code, code);
		else
			Kore::Keyboard::the()->_keydown((Kore::KeyCode)(code + 'a' - 'A'), code + 'a' - 'A');
		break;
	}
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_accelerometerChanged(JNIEnv* env, jobject obj, jfloat x, jfloat y, jfloat z) {
    ::env = env;
    Kore::Sensor::_changed(Kore::SensorAccelerometer, x, y, z);
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_gyroChanged(JNIEnv* env, jobject obj, jfloat x, jfloat y, jfloat z) {
    ::env = env;
    Kore::Sensor::_changed(Kore::SensorGyroscope, x, y, z);
}

#ifdef VR_CARDBOARD

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_gaze(JNIEnv* env, jobject obj, jfloat x, jfloat y, jfloat z, jfloat w) {
    Kore::VrInterface::updateGaze(x, y, z, w);
}

#endif

namespace {
	using namespace Kore;

	void copySample(void* buffer) {
		float value = *(float*)&Audio::buffer.data[Audio::buffer.readLocation];
		Audio::buffer.readLocation += 4;
		if (Audio::buffer.readLocation >= Audio::buffer.dataSize) Audio::buffer.readLocation = 0;
		*(s16*)buffer = static_cast<s16>(value * 32767);
	}
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_writeAudio(JNIEnv* env, jobject obj, jbyteArray buffer, jint size) {
    //::env = env;
	if (Kore::Audio::audioCallback != nullptr) {
		Kore::Audio::audioCallback(size / 2);
		jbyte* arr = env->GetByteArrayElements(buffer, 0);
		for (int i = 0; i < size; i += 2) {
			copySample(&arr[i]);
		}
		env->ReleaseByteArrayElements(buffer, arr, 0);
	}
}

bool Kore::System::handleMessages() {
	return true;
}

#include <sys/time.h>
#include <time.h>

double Kore::System::frequency() {
	return 1000000.0;
}

Kore::System::ticks Kore::System::timestamp() {
	timeval now;
	gettimeofday(&now, NULL);
	return static_cast<ticks>(now.tv_sec) * 1000000 + static_cast<ticks>(now.tv_usec);
}

double Kore::System::time() {
    timeval now;
    gettimeofday(&now, NULL);
    return (double)now.tv_sec + (double)(now.tv_usec / 1000000.0);
}

/*
#include <errno.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

extern int ktmain(int argc, char** argv);

static android_app* state;

struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

static engine* engine;

void Kt::System::HandleMessages() {
	//__android_log_write(ANDROID_LOG_ERROR, "kt", "Kt - handlemessages");
	// Read all pending events.
	int ident;
	int events;
	struct android_poll_source* source;
	
	// If not animating, we will block forever waiting for events.
	// If animating, we loop until all events are read, then continue
	// to draw the next frame of animation.
	engine->animating = 1;
	while ((ident=ALooper_pollAll(engine->animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0) {
	
	  // Process this event.
	  if (source != NULL) {
	      source->process(state, source);
	  }
	
	  // If a sensor has data, process it now.
	  if (ident == LOOPER_ID_USER) {
	      if (engine->accelerometerSensor != NULL) {
	          ASensorEvent event;
	          while (ASensorEventQueue_getEvents(engine->sensorEventQueue,
	                  &event, 1) > 0) {
	              LOGI("accelerometer: x=%f y=%f z=%f",
	                      event.acceleration.x, event.acceleration.y,
	                      event.acceleration.z);
	          }
	      }
	  }
	
	  // Check if we are exiting.
	  if (state->destroyRequested != 0) {
	      Kt::Scheduler::stop();
	      return;
	  }
	}
}

static int engine_init_display() {
    // initialize OpenGL ES and EGL

    //
    // Here specify the attributes of the desired configuration.
    // Below, we select an EGLConfig with at least 8 bits per color
    // component compatible with on-screen windows
    //
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	int version1, version2;
    eglInitialize(display, &version1, &version2);
    
    __android_log_print(ANDROID_LOG_ERROR, "kt", "OpenGL ES version %d %d", version1, version2);

    // Here, the application chooses the configuration it desires. In this
    // sample, we have a very simplified selection process, where we pick
    // the first EGLConfig that matches our criteria
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
    // guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
    // As soon as we picked a EGLConfig, we can safely reconfigure the
    // ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID.
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    // Initialize GL state.
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    //glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    //glEnable(GL_DEPTH_TEST);

__android_log_write(ANDROID_LOG_ERROR, "kt", "OpenGL initialized");

    return 0;
}

void* Kt::System::CreateWindow() {
	__android_log_write(ANDROID_LOG_ERROR, "kt", "Kt - createwindow");
	engine_init_display();
	Scheduler::addFrameTask(HandleMessages, 1001);
	return nullptr;
}

void Kt::System::SwitchBuffers() {
	if (engine->display == NULL) return;
	eglSwapBuffers(engine->display, engine->surface);
}

void Kt::System::DestroyWindow() {
	if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

static bool windowshown = false;

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                //engine_init_display();
            //    engine_draw_frame(engine);
            	windowshown = true;
            	__android_log_write(ANDROID_LOG_ERROR, "kt", "Kt - init_window");
            	ktmain(0, nullptr);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            //engine_term_display();
            Kt::Scheduler::stop();
            	__android_log_write(ANDROID_LOG_ERROR, "kt", "Kt - term_window");
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                        engine->accelerometerSensor, (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
        //    engine_draw_frame(engine);
        Kt::System::SwitchBuffers();
            break;
    }
}

namespace {
	AAssetManager* assets = nullptr;
}

AAssetManager* getAssetManager() {
	return assets;
}

extern "C" void android_main(struct android_app* state) {
	struct engine engine;
	::engine = &engine;
	::state = state;
	assets = state->activity->assetManager;
		// Make sure glue isn't stripped.
    app_dummy();
    
	__android_log_write(ANDROID_LOG_ERROR, "kt", "Kt - starting");

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
            state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }
    
    while (!windowshown) Kt::System::HandleMessages();
	//ktmain(0, nullptr);
}*/
