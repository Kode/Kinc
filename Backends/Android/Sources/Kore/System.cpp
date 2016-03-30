#include "pch.h"
/*#include <Kore/System.h>
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
#if SYS_ANDROID_API >= 15
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

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
#if SYS_ANDROID_API >= 15
	AAssetManager* assets;
#endif
	JNIEnv* env;
	bool firstinit = true;
}

JNIEnv* getEnv() {
	return env;
}

#if SYS_ANDROID_API >= 15
AAssetManager* getAssetManager() {
	return assets;
}
#endif

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
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onCreate(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onStart(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onPause(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onResume(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onStop(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onRestart(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onDestroy(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_gaze(JNIEnv* env, jobject obj, jfloat x, jfloat y, jfloat z, jfloat w);
};

void initAndroidFileReader(AAssetManager* assets);

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_init(JNIEnv* env, jobject obj, jint width, jint height, jobject assetManager, jstring apkPath, jstring filesDir) {
	if (firstinit) {
		{
			const char* path = env->GetStringUTFChars(filesDir, nullptr);
			std::strcpy(::filesDir, path);
			std::strcat(::filesDir, "/");
			env->ReleaseStringUTFChars(filesDir, path);
		}

    	//(*env)->NewGlobalRef(env, foo);
    	assets = AAssetManager_fromJava(env, assetManager);
		initAndroidFileReader(assets);

		firstinit = false;
	}

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

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onCreate(JNIEnv* env, jobject obj) {

}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onStart(JNIEnv* env, jobject obj) {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->foregroundCallback != nullptr) Kore::Application::the()->foregroundCallback();
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onPause(JNIEnv* env, jobject obj) {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->pauseCallback != nullptr) Kore::Application::the()->pauseCallback();
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onResume(JNIEnv* env, jobject obj) {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->resumeCallback != nullptr) Kore::Application::the()->resumeCallback();
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onStop(JNIEnv* env, jobject obj) {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->backgroundCallback != nullptr) Kore::Application::the()->backgroundCallback();
}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onRestart(JNIEnv* env, jobject obj) {

}

JNIEXPORT void JNICALL Java_com_ktxsoftware_kore_KoreLib_onDestroy(JNIEnv* env, jobject obj) {
	if (Kore::Application::the() != nullptr && Kore::Application::the()->shutdownCallback != nullptr) Kore::Application::the()->shutdownCallback();
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
*/

#include <Kore/Android.h>
#include <Kore/System.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>
#include <Kore/Input/Gamepad.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Input/Sensor.h>
#include <Kore/Input/Surface.h>
#include <EGL/egl.h>
#include <android/sensor.h>
#include <android/window.h>
#include <android_native_app_glue.h>
#include <GLContext.h>
#include <stdlib.h>

extern int kore(int argc, char** argv);

void pauseAudio();
void resumeAudio();

namespace {
	android_app* app = nullptr;
	ANativeActivity* activity = nullptr;
	ASensorManager* sensorManager = nullptr;
	const ASensor* accelerometerSensor = nullptr;
	const ASensor* gyroSensor = nullptr;
	ASensorEventQueue* sensorEventQueue = nullptr;
	bool shift = false;
	//int screenRotation = 0;

	ndk_helper::GLContext* glContext = nullptr;
	
	bool started = false;
	bool paused = true;

	void initDisplay() {
		if (glContext->Resume(app->window) != EGL_SUCCESS) {
			Kore::log(Kore::Warning, "GL context lost.");
		}
	}

	void termDisplay() {
		glContext->Suspend();
	}

	int32_t input(android_app* app, AInputEvent* event) {
		if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
			int action = AMotionEvent_getAction(event);
			int index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			int id = AMotionEvent_getPointerId(event, index);
			float x = AMotionEvent_getX(event, index);
			float y = AMotionEvent_getY(event, index);
			switch (action & AMOTION_EVENT_ACTION_MASK) {
				case AMOTION_EVENT_ACTION_DOWN:
				case AMOTION_EVENT_ACTION_POINTER_DOWN:
					if (id == 0) {
						Kore::Mouse::the()->_press(0, 0, x, y);
					}
					Kore::Surface::the()->_touchStart(id, x, y);
					// __android_log_print(ANDROID_LOG_INFO, "GAME", "#DOWN %d %d %d %f %f", action, index, id, x, y);
					break;
				case AMOTION_EVENT_ACTION_MOVE: {
						size_t count = AMotionEvent_getPointerCount(event);
						for(int i = 0; i < count; ++i) {
							id = AMotionEvent_getPointerId(event, i);
							x = AMotionEvent_getX(event, i);
							y = AMotionEvent_getY(event, i);
							if (id == 0) {
								Kore::Mouse::the()->_move(0, x, y);
							}
							Kore::Surface::the()->_move(id, x, y);
							// __android_log_print(ANDROID_LOG_INFO, "GAME", "#MOVE %d %d %d %f %f", action, index, id, x, y);
						}
					}
					break;
				case AMOTION_EVENT_ACTION_UP:
				case AMOTION_EVENT_ACTION_CANCEL:
				case AMOTION_EVENT_ACTION_POINTER_UP:
					if (id == 0) {
						Kore::Mouse::the()->_release(0, 0, x, y);
					}
					Kore::Surface::the()->_touchEnd(id, x, y);
					// __android_log_print(ANDROID_LOG_INFO, "GAME", "#UP %d %d %d %f %f", action, index, id, x, y);
					break;
			}
			return 1;
		}
		else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
			int32_t code = AKeyEvent_getKeyCode(event);

			if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN) {
				switch (code) {
					case AKEYCODE_SHIFT_LEFT:
					case AKEYCODE_SHIFT_RIGHT:
						shift = true;
						Kore::Keyboard::the()->_keydown(Kore::Key_Shift, 0);
						return 1;
					case AKEYCODE_DEL:
						Kore::Keyboard::the()->_keydown(Kore::Key_Backspace, 0);
						return 1;
					case AKEYCODE_ENTER:
					case AKEYCODE_NUMPAD_ENTER:
						Kore::Keyboard::the()->_keydown(Kore::Key_Return, 0);
						return 1;
					case AKEYCODE_DPAD_LEFT:
						Kore::Gamepad::get(0)->_axis(0, -1);
						return 1;
					case AKEYCODE_DPAD_RIGHT:
						Kore::Gamepad::get(0)->_axis(0, 1);
						return 1;
					case AKEYCODE_DPAD_UP:
						Kore::Gamepad::get(0)->_axis(1, -1);
						return 1;
					case AKEYCODE_DPAD_DOWN:
						Kore::Gamepad::get(0)->_axis(1, 1);
						return 1;
					case AKEYCODE_DPAD_CENTER:
					case AKEYCODE_BUTTON_B:
						Kore::Gamepad::get(0)->_button(0, 1);
						return 1;
					case AKEYCODE_BACK:
						if (AKeyEvent_getMetaState(event) & AMETA_ALT_ON) { // Xperia Play
							Kore::Gamepad::get(0)->_button(1, 1);
							return 1;
						}
						else {
							Kore::Keyboard::the()->_keydown(Kore::Key_Back, 1);
							return 1;
						}
					case AKEYCODE_BUTTON_A:
						Kore::Gamepad::get(0)->_button(1, 1);
						return 1;
					case AKEYCODE_BUTTON_X:
						Kore::Gamepad::get(0)->_button(2, 1);
						return 1;
					case AKEYCODE_BUTTON_Y:
						Kore::Gamepad::get(0)->_button(3, 1);
						return 1;
					case AKEYCODE_STAR:
					case AKEYCODE_NUMPAD_MULTIPLY:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '*', '*');
						return 1;
					case AKEYCODE_POUND:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '&', '&');
						return 1;
					case AKEYCODE_COMMA:
					case AKEYCODE_NUMPAD_COMMA:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) ',', ',');
						return 1;
					case AKEYCODE_PERIOD:
					case AKEYCODE_NUMPAD_DOT:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '.', '.');
						return 1;
					case AKEYCODE_SPACE:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) ' ', ' ');
						return 1;
					case AKEYCODE_MINUS:
					case AKEYCODE_NUMPAD_SUBTRACT:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '-', '-');
						return 1;
					case AKEYCODE_EQUALS:
					case AKEYCODE_NUMPAD_EQUALS:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '=', '=');
						return 1;
					case AKEYCODE_LEFT_BRACKET:
					case AKEYCODE_NUMPAD_LEFT_PAREN:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '(', '(');
						return 1;
					case AKEYCODE_RIGHT_BRACKET:
					case AKEYCODE_NUMPAD_RIGHT_PAREN:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) ')', ')');
						return 1;
					case AKEYCODE_BACKSLASH:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '\\', '\\');
						return 1;
					case AKEYCODE_SEMICOLON:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) ';', ';');
						return 1;
					case AKEYCODE_APOSTROPHE:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '\'', '\'');
						return 1;
					case AKEYCODE_SLASH:
					case AKEYCODE_NUMPAD_DIVIDE:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '/', '/');
						return 1;
					case AKEYCODE_AT:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '@', '@');
						return 1;
					case AKEYCODE_PLUS:
					case AKEYCODE_NUMPAD_ADD:
						Kore::Keyboard::the()->_keydown((Kore::KeyCode) '+', '+');
						return 1;
					default:
						if (code >= AKEYCODE_NUMPAD_0 && code <= AKEYCODE_NUMPAD_9) {
							Kore::Keyboard::the()->_keydown((Kore::KeyCode) (code + '0' - AKEYCODE_NUMPAD_0), code + '0' - AKEYCODE_NUMPAD_0);
							return 1;
						}
						else if (code >= AKEYCODE_0 && code <= AKEYCODE_9) {
							Kore::Keyboard::the()->_keydown((Kore::KeyCode) (code + '0' - AKEYCODE_0), code + '0' - AKEYCODE_0);
							return 1;
						}
						else if (code >= AKEYCODE_A && code <= AKEYCODE_Z) {
							if (shift)
								Kore::Keyboard::the()->_keydown((Kore::KeyCode) (code + 'A' - AKEYCODE_A), code + 'A' - AKEYCODE_A);
							else
								Kore::Keyboard::the()->_keydown((Kore::KeyCode) (code + 'a' - AKEYCODE_A), code + 'a' - AKEYCODE_A);
							return 1;
						}
				}
			}
			else if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_UP) {
				switch (code) {
					case AKEYCODE_SHIFT_LEFT:
					case AKEYCODE_SHIFT_RIGHT:
						shift = false;
						Kore::Keyboard::the()->_keyup(Kore::Key_Shift, 0);
						return 1;
					case AKEYCODE_DEL:
						Kore::Keyboard::the()->_keyup(Kore::Key_Backspace, 0);
						return 1;
					case AKEYCODE_ENTER:
						Kore::Keyboard::the()->_keyup(Kore::Key_Return, 0);
						return 1;
					case AKEYCODE_DPAD_LEFT:
						Kore::Gamepad::get(0)->_axis(0, 0);
						return 1;
					case AKEYCODE_DPAD_RIGHT:
						Kore::Gamepad::get(0)->_axis(0, 0);
						return 1;
					case AKEYCODE_DPAD_UP:
						Kore::Gamepad::get(0)->_axis(1, 0);
						return 1;
					case AKEYCODE_DPAD_DOWN:
						Kore::Gamepad::get(0)->_axis(1, 0);
						return 1;
					case AKEYCODE_DPAD_CENTER:
					case AKEYCODE_BUTTON_B:
						Kore::Gamepad::get(0)->_button(0, 0);
						return 1;
					case AKEYCODE_BACK:
						if (AKeyEvent_getMetaState(event) & AMETA_ALT_ON) { // Xperia Play
							Kore::Gamepad::get(0)->_button(1, 0);
							return 1;
						}
						else {
							Kore::Keyboard::the()->_keyup(Kore::Key_Back, 0);
							return 1;
						}
					case AKEYCODE_BUTTON_A:
						Kore::Gamepad::get(0)->_button(1, 0);
						return 1;
					case AKEYCODE_BUTTON_X:
						Kore::Gamepad::get(0)->_button(2, 0);
						return 1;
					case AKEYCODE_BUTTON_Y:
						Kore::Gamepad::get(0)->_button(3, 0);
						return 1;
					case AKEYCODE_STAR:
					case AKEYCODE_NUMPAD_MULTIPLY:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '*', '*');
						return 1;
					case AKEYCODE_POUND:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '&', '&');
						return 1;
					case AKEYCODE_COMMA:
					case AKEYCODE_NUMPAD_COMMA:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) ',', ',');
						return 1;
					case AKEYCODE_PERIOD:
					case AKEYCODE_NUMPAD_DOT:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '.', '.');
						return 1;
					case AKEYCODE_SPACE:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) ' ', ' ');
						return 1;
					case AKEYCODE_MINUS:
					case AKEYCODE_NUMPAD_SUBTRACT:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '-', '-');
						return 1;
					case AKEYCODE_EQUALS:
					case AKEYCODE_NUMPAD_EQUALS:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '=', '=');
						return 1;
					case AKEYCODE_LEFT_BRACKET:
					case AKEYCODE_NUMPAD_LEFT_PAREN:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '(', '(');
						return 1;
					case AKEYCODE_RIGHT_BRACKET:
					case AKEYCODE_NUMPAD_RIGHT_PAREN:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) ')', ')');
						return 1;
					case AKEYCODE_BACKSLASH:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '\\', '\\');
						return 1;
					case AKEYCODE_SEMICOLON:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) ';', ';');
						return 1;
					case AKEYCODE_APOSTROPHE:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '\'', '\'');
						return 1;
					case AKEYCODE_SLASH:
					case AKEYCODE_NUMPAD_DIVIDE:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '/', '/');
						return 1;
					case AKEYCODE_AT:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '@', '@');
						return 1;
					case AKEYCODE_PLUS:
					case AKEYCODE_NUMPAD_ADD:
						Kore::Keyboard::the()->_keyup((Kore::KeyCode) '+', '+');
						return 1;
					default:
						if (code >= AKEYCODE_NUMPAD_0 && code <= AKEYCODE_NUMPAD_9) {
							Kore::Keyboard::the()->_keyup((Kore::KeyCode) (code + '0' - AKEYCODE_NUMPAD_0), code + '0' - AKEYCODE_NUMPAD_0);
							return 1;
						}
						else if (code >= AKEYCODE_0 && code <= AKEYCODE_9) {
							Kore::Keyboard::the()->_keyup((Kore::KeyCode)(code + '0' - AKEYCODE_0), code + '0' - AKEYCODE_0);
							return 1;
						}
						else if (code >= AKEYCODE_A && code <= AKEYCODE_Z) {
							if (shift)
								Kore::Keyboard::the()->_keyup((Kore::KeyCode)(code + 'A' - AKEYCODE_A), code + 'A' - AKEYCODE_A);
							else
								Kore::Keyboard::the()->_keyup((Kore::KeyCode)(code + 'a' - AKEYCODE_A), code + 'a' - AKEYCODE_A);
							return 1;
						}
				}
			}
		}
		return 0;
	}

	void cmd(android_app* app, int32_t cmd) {
		switch (cmd) {
			case APP_CMD_SAVE_STATE:
				break;
			case APP_CMD_INIT_WINDOW:
				if (app->window != NULL) {
					initDisplay();
					if (!started) {
						started = true;
					}
					Kore::System::swapBuffers(0);
				}
				break;
			case APP_CMD_TERM_WINDOW:
				termDisplay();
				break;
			case APP_CMD_GAINED_FOCUS:
				if (accelerometerSensor != NULL) {
					ASensorEventQueue_enableSensor(sensorEventQueue, accelerometerSensor);
					ASensorEventQueue_setEventRate(sensorEventQueue, accelerometerSensor, (1000L / 60) * 1000);
				}
				if (gyroSensor != NULL) {
					ASensorEventQueue_enableSensor(sensorEventQueue, gyroSensor);
					ASensorEventQueue_setEventRate(sensorEventQueue, gyroSensor, (1000L / 60) * 1000);
				}
				break;
			case APP_CMD_LOST_FOCUS:
				if (accelerometerSensor != NULL) {
					ASensorEventQueue_disableSensor(sensorEventQueue, accelerometerSensor);
				}
				if (gyroSensor != NULL) {
					ASensorEventQueue_disableSensor(sensorEventQueue, gyroSensor);
				}
				break;
			case APP_CMD_START:
				Kore::System::foregroundCallback();
				break;
			case APP_CMD_RESUME:
                Kore::System::resumeCallback();
				resumeAudio();
				paused = false;
				break;
			case APP_CMD_PAUSE:
                Kore::System::pauseCallback();
				pauseAudio();
				paused = true;
				break;
			case APP_CMD_STOP:
                Kore::System::backgroundCallback();
				break;
			case APP_CMD_DESTROY:
                Kore::System::shutdownCallback();
				break;
			case APP_CMD_CONFIG_CHANGED: {

				break;
			}
		}
	}
}

ANativeActivity* KoreAndroid::getActivity() {
	return activity;
}

AAssetManager* KoreAndroid::getAssetManager() {
	return activity->assetManager;
}

jclass KoreAndroid::findClass(JNIEnv* env, const char* name) {
	jobject nativeActivity = activity->clazz;
	jclass acl = env->GetObjectClass(nativeActivity);
	jmethodID getClassLoader = env->GetMethodID(acl, "getClassLoader", "()Ljava/lang/ClassLoader;");
	jobject cls = env->CallObjectMethod(nativeActivity, getClassLoader);
	jclass classLoader = env->FindClass("java/lang/ClassLoader");
	jmethodID findClass = env->GetMethodID(classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
	jstring strClassName = env->NewStringUTF(name);
	jclass clazz = (jclass) (env->CallObjectMethod(cls, findClass, strClassName));
	env->DeleteLocalRef(strClassName);
	return clazz;
}

void Kore::System::swapBuffers(int) {
	if (glContext->Swap() != EGL_SUCCESS) {
		Kore::log(Kore::Warning, "GL context lost.");
	}
}

void Kore::System::destroyWindow(int) {

}

void Kore::System::showKeyboard() {
	JNIEnv* env;
	activity->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreActivity");
	env->CallStaticVoidMethod(koreActivityClass, env->GetStaticMethodID(koreActivityClass, "showKeyboard", "()V"));
	activity->vm->DetachCurrentThread();
}

void Kore::System::hideKeyboard() {
	JNIEnv* env;
	activity->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreActivity");
	env->CallStaticVoidMethod(koreActivityClass, env->GetStaticMethodID(koreActivityClass, "hideKeyboard", "()V"));
	activity->vm->DetachCurrentThread();
}

void Kore::System::loadURL(const char* url) {
	JNIEnv* env;
	activity->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreActivity");
	jstring jurl = env->NewStringUTF(url);
	env->CallStaticVoidMethod(koreActivityClass, env->GetStaticMethodID(koreActivityClass, "loadURL", "(Ljava/lang/String;)V"), jurl);
	activity->vm->DetachCurrentThread();
}

int Kore::System::windowWidth(int windowId) {
	glContext->UpdateSize();
	return glContext->GetScreenWidth();
}

int Kore::System::windowHeight(int windowId) {
	glContext->UpdateSize();
	return glContext->GetScreenHeight();
}

const char* Kore::System::savePath() {
	return KoreAndroid::getActivity()->internalDataPath;
}

const char* Kore::System::systemId() {
	return "Android";
}

namespace {
	const char* videoFormats[] = { "ts", nullptr };
}

const char** Kore::System::videoFormats() {
	return ::videoFormats;
}

void Kore::System::changeResolution(int, int, bool) {

}

void Kore::System::setTitle(const char*) {

}

void Kore::System::setKeepScreenOn( bool on ) {
    if(on)
    {
        ANativeActivity_setWindowFlags(activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
    }else{
        ANativeActivity_setWindowFlags(activity, 0, AWINDOW_FLAG_KEEP_SCREEN_ON);
    }
}

void Kore::System::showWindow() {
}

int Kore::System::windowCount() {
    return 1;
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
	return (double)now.tv_sec + (now.tv_usec / 1000000.0);
}

bool Kore::System::handleMessages() {
	int ident;
	int events;
	android_poll_source *source;

	while ((ident = ALooper_pollAll(paused ? -1 : 0, NULL, &events, (void **) &source)) >= 0) {
		if (source != NULL) {
			source->process(app, source);
		}

		if (ident == LOOPER_ID_USER) {
			if (accelerometerSensor != NULL) {
				ASensorEvent event;
				while (ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0) {
					if (event.type == ASENSOR_TYPE_ACCELEROMETER) {
						Kore::Sensor::_changed(Kore::SensorAccelerometer, event.acceleration.x,
											   event.acceleration.y, event.acceleration.z);
					}
					else if (event.type == ASENSOR_TYPE_GYROSCOPE) {
						Kore::Sensor::_changed(Kore::SensorGyroscope, event.vector.x,
											   event.vector.x, event.vector.z);
					}
				}
			}
		}

		if (app->destroyRequested != 0) {
			termDisplay();
			Kore::System::stop();
			return true;
		}
	}

	{
		JNIEnv* env = nullptr;
		KoreAndroid::getActivity()->vm->AttachCurrentThread(&env, nullptr);
		jclass koreMoviePlayerClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreMoviePlayer");

		jmethodID updateAll = env->GetStaticMethodID(koreMoviePlayerClass, "updateAll", "()V");
		env->CallStaticVoidMethod(koreMoviePlayerClass, updateAll);

		KoreAndroid::getActivity()->vm->DetachCurrentThread();
	}

	// Get screen rotation
	/*
	JNIEnv* env;
	activity->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreActivity");
	jmethodID koreActivityGetRotation = env->GetStaticMethodID(koreActivityClass, "getRotation", "()I");
	screenRotation = env->CallStaticIntMethod(koreActivityClass, koreActivityGetRotation);
	activity->vm->DetachCurrentThread();
	*/

	return true;
}

bool Kore::Mouse::canLock(int windowId) {
	return false;
}

void Kore::Mouse::setPosition(int windowId, int, int) {

}

void Kore::Mouse::_lock(int windowId, bool) {

}

void Kore::Mouse::getPosition(int windowId, int& x, int& y) {
	x = 0;
	y = 0;
}

void initAndroidFileReader();
void KoreAndroidVideoInit();

extern "C" void android_main(android_app* app) {
	app_dummy();

	::app = app;
	activity = app->activity;
	initAndroidFileReader();
	KoreAndroidVideoInit();
	app->onAppCmd = cmd;
	app->onInputEvent = input;

	glContext = ndk_helper::GLContext::GetInstance();
	sensorManager = ASensorManager_getInstance();
	accelerometerSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	gyroSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_GYROSCOPE);
	sensorEventQueue = ASensorManager_createEventQueue(sensorManager, app->looper, LOOPER_ID_USER, NULL, NULL);

	while (!started) {
		Kore::System::handleMessages();
	}
	kore(0, nullptr);
	exit(0);
}

void Kore::System::setup() {
}

int Kore::System::initWindow( Kore::WindowOptions options ) {
    Graphics::init(0, options.rendererOptions.depthBufferBits, options.rendererOptions.stencilBufferBits);
    return 0;
}

namespace appstate {
    int currentDeviceId = -1;
}

bool Kore::System::isFullscreen() {
    return true;
}

int Kore::System::currentDevice() {
    return appstate::currentDeviceId;
}

void Kore::System::makeCurrent( int id ) {
    appstate::currentDeviceId = id;
}

void Kore::System::clearCurrent() {
}
