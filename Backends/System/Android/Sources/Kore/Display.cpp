#include "pch.h"

#include <Kore/Android.h>

#include <kinc/display.h>
#include <kinc/log.h>

#include <stdexcept>

namespace {
	kinc_display_t display;
}

int kinc_count_displays(void) {
	return 1;
}

int kinc_primary_display(void) {
	return 0;
}

/*Display* Display::get(int index) {
    if (index > 0) {
        return nullptr;
    }
	return &display;
}*/


static int width() {
	JNIEnv* env;
	KoreAndroid::getActivity()->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = KoreAndroid::findClass(env, "tech.kode.kore.KoreActivity");
	jmethodID koreActivityGetScreenDpi = env->GetStaticMethodID(koreActivityClass, "getDisplayWidth", "()I");
	int width = env->CallStaticIntMethod(koreActivityClass, koreActivityGetScreenDpi);
	KoreAndroid::getActivity()->vm->DetachCurrentThread();
	return width;
}

static int height() {
	JNIEnv* env;
	KoreAndroid::getActivity()->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = KoreAndroid::findClass(env, "tech.kode.kore.KoreActivity");
	jmethodID koreActivityGetScreenDpi = env->GetStaticMethodID(koreActivityClass, "getDisplayHeight", "()I");
	int height = env->CallStaticIntMethod(koreActivityClass, koreActivityGetScreenDpi);
	KoreAndroid::getActivity()->vm->DetachCurrentThread();
	return height;
}

static int pixelsPerInch() {
	JNIEnv* env;
	KoreAndroid::getActivity()->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = KoreAndroid::findClass(env, "tech.kode.kore.KoreActivity");
	jmethodID koreActivityGetScreenDpi = env->GetStaticMethodID(koreActivityClass, "getScreenDpi", "()I");
	int dpi = env->CallStaticIntMethod(koreActivityClass, koreActivityGetScreenDpi);
	KoreAndroid::getActivity()->vm->DetachCurrentThread();
	return dpi;
}

kinc_display_mode_t kinc_display_available_mode(int display_index, int mode_index) {
	kinc_display_mode_t mode;
	mode.x = 0;
	mode.y = 0;
	mode.width = width();
	mode.height = height();
	mode.frequency = 60;
	mode.bits_per_pixel = 32;
	mode.pixels_per_inch = pixelsPerInch();
	return mode;
}

int kinc_display_count_available_modes(int display_index) {
	return 1;
}
