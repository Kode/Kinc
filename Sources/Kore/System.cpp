#include "pch.h"

#include "System.h"

#include <Kore/Convert.h>
#include <Kore/Window.h>
#include <Kore/Math/Random.h>

#include <Kinc/System.h>
#include <Kinc/Window.h>

#include <assert.h>
#include <limits>
#include <string.h>

double Kore::System::time() {
	return Kinc_Time();
}

void Kore::System::setCallback(void (*value)()) {
	Kinc_SetUpdateCallback(value);
}

void Kore::System::setForegroundCallback(void (*value)()) {
	Kinc_SetForegroundCallback(value);
}

void Kore::System::setResumeCallback(void (*value)()) {
	Kinc_SetResumeCallback(value);
}

void Kore::System::setPauseCallback(void (*value)()) {
	Kinc_SetPauseCallback(value);
}

void Kore::System::setBackgroundCallback(void (*value)()) {
	Kinc_SetBackgroundCallback(value);
}

void Kore::System::setShutdownCallback(void (*value)()) {
	Kinc_SetShutdownCallback(value);
}

void Kore::System::setOrientationCallback(void (*value)(Orientation)) {
	
}

void Kore::System::setDropFilesCallback(void (*value)(wchar_t*)) {
	Kinc_SetDropFilesCallback(value);
}

void Kore::System::setCutCallback(char* (*value)()) {
	Kinc_SetCutCallback(value);
}

void Kore::System::setCopyCallback(char* (*value)()) {
	Kinc_SetCopyCallback(value);
}

void Kore::System::setPasteCallback(void (*value)(char*)) {
	Kinc_SetPasteCallback(value);
}

void Kore::System::_callback() {
	Kinc_Internal_UpdateCallback();
}

void Kore::System::_foregroundCallback() {
	Kinc_Internal_ForegroundCallback();
}

void Kore::System::_resumeCallback() {
	Kinc_Internal_ResumeCallback();
}

void Kore::System::_pauseCallback() {
	Kinc_Internal_PauseCallback();
}

void Kore::System::_backgroundCallback() {
	Kinc_Internal_BackgroundCallback();
}

void Kore::System::_shutdownCallback() {
	Kinc_Internal_ShutdownCallback();
}

void Kore::System::_orientationCallback(Orientation orientation) {
	
}

void Kore::System::_dropFilesCallback(wchar_t* filePath) {
	Kinc_Internal_DropFilesCallback(filePath);
}

char* Kore::System::_cutCallback() {
	return Kinc_Internal_CutCallback();
}

char* Kore::System::_copyCallback() {
	return Kinc_Internal_CopyCallback();
}

void Kore::System::_pasteCallback(char* value) {
	Kinc_Internal_PasteCallback(value);
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
	return Kinc_ApplicationName();
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
	Kinc_Stop();
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
	Kinc_WindowOptions kwin;
	if (win != nullptr) {
		kwin = convert(win);
	}

	Kinc_FramebufferOptions kframe;
	if (frame != nullptr) {
		kframe = convert(frame);
	}

	int window = Kinc_Init(name, width, height, win == nullptr ? nullptr : &kwin, frame == nullptr ? nullptr : &kframe);
	return Window::get(window);
}

const char *Kore::System::savePath() {
	return Kinc_Internal_SavePath();
}

bool Kore::System::handleMessages() {
	return Kinc_Internal_HandleMessages();
}

void Kore::System::_shutdown() {
	Kinc_Internal_Shutdown();
}

void Kore::System::start() {
	Kinc_Start();
}
