#include "pch.h"

#include <Kore/Video.h>

using namespace Kore;

Video::Video(const char *filename) {
	kinc_video_init(&video, filename);
}

Video::~Video() {
	kinc_video_destroy(&video);
}

void Video::play() {
	kinc_video_play(&video);
}

void Video::pause() {
	kinc_video_pause(&video);
}

void Video::stop() {
	kinc_video_stop(&video);
}

int Video::width() {
	return kinc_video_width(&video);
}

int Video::height() {
	return kinc_video_height(&video);
}

kinc_g4_texture_t *Video::currentImage() {
	return kinc_video_current_image(&video);
}

double Video::duration() {
	return kinc_video_duration(&video);
}

double Video::position() {
	return kinc_video_position(&video);
}

bool Video::finished() {
	return kinc_video_finished(&video);
}

bool Video::paused() {
	return kinc_video_paused(&video);
}

void Video::update(double time) {
	kinc_video_update(&video, time);
}
