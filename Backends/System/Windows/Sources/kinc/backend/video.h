#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *renderer;
	double duration;
	double position;
	bool finished;
	bool paused;
} kinc_video_impl_t;

#ifdef __cplusplus
}
#endif
