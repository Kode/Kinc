#include "pch.h"

#include <Kore/Android.h>
#include <Kore/Display.h>
#include <Kore/Log.h>

#include <stdexcept>

using namespace Kore;

namespace {
    Display display;
}

int Display::count() {
	return 1;
}

Display* Display::primary() {
	return &display;
}

Display* Display::get(int index) {
    if (index > 0) {
        return nullptr;
    }
	return &display;
}

DisplayMode Display::availableMode(int index) {
	DisplayMode mode;
	mode.width = 800;
	mode.height = 600;
	mode.frequency = 60;
	mode.bitsPerPixel = 32;
	return mode;
}

int Display::countAvailableModes() {
	return 1;
}

int Display::pixelsPerInch() {
    JNIEnv* env;
    KoreAndroid::getActivity()->vm->AttachCurrentThread(&env, nullptr);
    jclass koreActivityClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreActivity");
    jmethodID koreActivityGetScreenDpi = env->GetStaticMethodID(koreActivityClass, "getScreenDpi", "()I");
    int dpi = env->CallStaticIntMethod(koreActivityClass, koreActivityGetScreenDpi);
    KoreAndroid::getActivity()->vm->DetachCurrentThread();
    return dpi;
}

DisplayData::DisplayData() {}

bool Display::available() {
	return true;
}

const char* Display::name() {
	return "Display";
}

int Display::x() {
	return 0;
}

int Display::y() {
	return 0;
}

int Display::width() {
	JNIEnv* env;
	KoreAndroid::getActivity()->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreActivity");
	jmethodID koreActivityGetScreenDpi = env->GetStaticMethodID(koreActivityClass, "getDisplayWidth", "()I");
	int width = env->CallStaticIntMethod(koreActivityClass, koreActivityGetScreenDpi);
	KoreAndroid::getActivity()->vm->DetachCurrentThread();
	return width;
}

int Display::height() {
	JNIEnv* env;
	KoreAndroid::getActivity()->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreActivity");
	jmethodID koreActivityGetScreenDpi = env->GetStaticMethodID(koreActivityClass, "getDisplayHeight", "()I");
	int height = env->CallStaticIntMethod(koreActivityClass, koreActivityGetScreenDpi);
	KoreAndroid::getActivity()->vm->DetachCurrentThread();
	return height;
}

int Display::frequency() {
	return 60;
}

Display::Display() {

}
