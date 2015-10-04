#pragma once

#include <android_native_app_glue.h>

namespace KoreAndroid {
    // name in usual Java syntax (points, no slashes)
    jclass findClass(JNIEnv* env, const char* name);

    ANativeActivity* getActivity();

    AAssetManager* getAssetManager();
}
