#include "pch.h"

#include "System.h"

#include <Kore/Convert.h>
#include <Kore/Window.h>
#include <Kore/Math/Random.h>

#include <kinc/input/keyboard.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include <assert.h>
#include <limits>
#include <string.h>

double Kore::System::time() {
	return kinc_time();
}

void Kore::System::setCallback(void (*value)()) {
	kinc_set_update_callback(value);
}

void Kore::System::setForegroundCallback(void (*value)()) {
	kinc_set_foreground_callback(value);
}

void Kore::System::setResumeCallback(void (*value)()) {
	kinc_set_resume_callback(value);
}

void Kore::System::setPauseCallback(void (*value)()) {
	kinc_set_pause_callback(value);
}

void Kore::System::setBackgroundCallback(void (*value)()) {
	kinc_set_background_callback(value);
}

void Kore::System::setShutdownCallback(void (*value)()) {
	kinc_set_shutdown_callback(value);
}

void Kore::System::setOrientationCallback(void (*value)(Orientation)) {
	
}

void Kore::System::setDropFilesCallback(void (*value)(wchar_t*)) {
	kinc_set_drop_files_callback(value);
}

void Kore::System::setCutCallback(char* (*value)()) {
	kinc_set_cut_callback(value);
}

void Kore::System::setCopyCallback(char* (*value)()) {
	kinc_set_copy_callback(value);
}

void Kore::System::setPasteCallback(void (*value)(char*)) {
	kinc_set_paste_callback(value);
}

void Kore::System::_callback() {
	kinc_internal_update_callback();
}

void Kore::System::_foregroundCallback() {
	kinc_internal_foreground_callback();
}

void Kore::System::_resumeCallback() {
	kinc_internal_resume_callback();
}

void Kore::System::_pauseCallback() {
	kinc_internal_pause_callback();
}

void Kore::System::_backgroundCallback() {
	kinc_internal_background_callback();
}

void Kore::System::_shutdownCallback() {
	kinc_internal_shutdown_callback();
}

void Kore::System::_orientationCallback(Orientation orientation) {
	
}

void Kore::System::_dropFilesCallback(wchar_t* filePath) {
	kinc_internal_drop_files_callback(filePath);
}

char* Kore::System::_cutCallback() {
	return kinc_internal_cut_callback();
}

char* Kore::System::_copyCallback() {
	return kinc_internal_copy_callback();
}

void Kore::System::_pasteCallback(char* value) {
	kinc_internal_paste_callback(value);
}

namespace {
	bool running = false;
	bool showWindowFlag = true;
	char name[1024] = {"KoreApplication"};
	Kore::WindowOptions defaultWin;
	Kore::FramebufferOptions defaultFrame;
}
/*
void Kore::System::setShowWindowFlag(bool value) {
	appstate::showWindowFlag = value;
}

bool Kore::System::hasShowWindowFlag() {
	return appstate::showWindowFlag;
}

void Kore::System::setName(const char* value) {
	strcpy(appstate::name, value);
}
*/
const char* Kore::System::name() {
	return kinc_application_name();
}

void Kore::System::_init(const char* name, int width, int height, WindowOptions** win, FramebufferOptions** frame) {
	if (*win == nullptr) {
		*win = &defaultWin;
	}

	strcpy(::name, name);
	if (strcmp((*win)->title, "Kore") == 0) {
		(*win)->title = name;
	}
	(*win)->width = width;
	(*win)->height = height;

	if (*frame == nullptr) {
		*frame = &defaultFrame;
	}
}

#ifdef KORE_METAL
void shutdownMetalCompute();
#endif

void Kore::System::stop() {
	running = false;
	kinc_stop();
}

int Kore::System::windowWidth(int window) {
	assert(window < Window::count());
	return Window::get(window)->width();
}

int Kore::System::windowHeight(int window) {
	assert(window < Window::count());
	return Window::get(window)->height();
}

Kore::Window *Kore::System::init(const char *name, int width, int height, Kore::WindowOptions *win, Kore::FramebufferOptions *frame) {
	kinc_window_options_t kwin;
	if (win != nullptr) {
		kwin = convert(win);
	}

	kinc_framebuffer_options_t kframe;
	if (frame != nullptr) {
		kframe = convert(frame);
	}

	int window = kinc_init(name, width, height, win == nullptr ? nullptr : &kwin, frame == nullptr ? nullptr : &kframe);
	return Window::get(window);
}

const char *Kore::System::savePath() {
	return kinc_internal_save_path();
}

bool Kore::System::handleMessages() {
	return kinc_internal_handle_messages();
}

void Kore::System::_shutdown() {
	kinc_internal_shutdown();
}

void Kore::System::start() {
	kinc_start();
}

void Kore::System::setKeepScreenOn(bool on) {
	kinc_set_keep_screen_on(on);
}

const char* Kore::System::systemId() {
	return kinc_system_id();
}

void Kore::System::vibrate(int milliseconds) {
	kinc_vibrate(milliseconds);
}

const char* Kore::System::language() {
	return kinc_language();
}

void Kore::System::showKeyboard() {
	kinc_keyboard_show();
}

void Kore::System::hideKeyboard() {
	kinc_keyboard_hide();
}

bool Kore::System::showsKeyboard() {
	return kinc_keyboard_active();
}

void Kore::System::loadURL(const char* url) {
	kinc_load_url(url);
}

const char** Kore::System::videoFormats() {
	return kinc_video_formats();
}
