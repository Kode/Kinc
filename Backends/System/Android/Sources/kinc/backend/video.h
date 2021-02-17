#pragma once

#include <kinc/graphics4/texture.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void* assetReader;
	void* videoTrackOutput;
	void* audioTrackOutput;
	double start;
	double next;
	// double audioTime;
	unsigned long long audioTime;
	bool playing;
	void* sound;
	void* androidVideo;
	int id;
	kinc_g4_texture_t image;
	double lastTime;
	int myWidth;
	int myHeight;
} kinc_video_impl_t;

#ifdef __cplusplus
}
#endif
