#include "pch.h"

#include <Kore/Log.h>
#include <Kore/Android.h>

#include <stdexcept>

namespace Kore { namespace Display {
	int count() {
		return 1;
	}

	int width(int index) {
		JNIEnv* env;
		KoreAndroid::getActivity()->vm->AttachCurrentThread(&env, nullptr);
		jclass koreActivityClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreActivity");
		jmethodID koreActivityGetScreenDpi = env->GetStaticMethodID(koreActivityClass, "getDisplayWidth", "()I");
		int width = env->CallStaticIntMethod(koreActivityClass, koreActivityGetScreenDpi);
		KoreAndroid::getActivity()->vm->DetachCurrentThread();

		return width;
	}

	int height(int index) {
		JNIEnv* env;
		KoreAndroid::getActivity()->vm->AttachCurrentThread(&env, nullptr);
		jclass koreActivityClass = KoreAndroid::findClass(env, "com.ktxsoftware.kore.KoreActivity");
		jmethodID koreActivityGetScreenDpi = env->GetStaticMethodID(koreActivityClass, "getDisplayHeight", "()I");
		int height = env->CallStaticIntMethod(koreActivityClass, koreActivityGetScreenDpi);
		KoreAndroid::getActivity()->vm->DetachCurrentThread();

		return height;
	}

	int x(int index) {
		log(Warning, "TODO (DK) Kore::Display::x(int) implement me");
		return -1;
	}

	int y(int index) {
		log(Warning, "TODO (DK) Kore::Display::y(int) implement me");
		return -1;
	}

	bool isPrimary(int index) {
		log(Warning, "TODO (DK) Kore::Display::isPrimary(int) implement me");
		return -1;
	}
}}
