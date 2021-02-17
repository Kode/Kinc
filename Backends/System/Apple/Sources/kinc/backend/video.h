#pragma once

#include <objc/runtime.h>

#include <kinc/graphics4/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double start;
    double videoStart;
    double next;
    // double audioTime;
    unsigned long long audioTime;
    bool playing;
    void *sound;
    bool image_initialized;
    kinc_g4_texture_t image;
    double lastTime;
    int myWidth;
    int myHeight;
   
    id videoAsset;
    id assetReader;
    id videoTrackOutput;
    id audioTrackOutput;
    id url;
} kinc_video_impl_t;

#ifdef __cplusplus
}
#endif
