#include "pch.h"

#include "system.h"

#include "window.h"

#include <stdlib.h>

#if !defined(KORE_HTML5) && !defined(KORE_ANDROID) && !defined(KORE_WINDOWS) && !defined(KORE_CONSOLE)
double Kinc_Time() {
	return timestamp() / frequency();
}
#endif

static void (*update_callback)() = NULL;
static void (*foreground_callback)() = NULL;
static void (*background_callback)() = NULL;
static void (*pause_callback)() = NULL;
static void (*resume_callback)() = NULL;
static void (*shutdown_callback)() = NULL;
static void (*drop_files_callback)(wchar_t *) = NULL;
static char *(*cut_callback)() = NULL;
static char *(*copy_callback)() = NULL;
static void (*paste_callback)(char *) = NULL;

#if defined(KORE_IOS) || defined(KORE_MACOS)
bool withAutoreleasepool(bool (*f)());
#endif

void kinc_set_update_callback(void (*value)()) {
	update_callback = value;
}

void kinc_set_foreground_callback(void (*value)()) {
	foreground_callback = value;
}

void kinc_set_resume_callback(void (*value)()) {
	resume_callback = value;
}

void kinc_set_pause_callback(void (*value)()) {
	pause_callback = value;
}

void kinc_set_background_callback(void (*value)()) {
	background_callback = value;
}

void kinc_set_shutdown_callback(void (*value)()) {
	shutdown_callback = value;
}

void kinc_set_drop_files_callback(void (*value)(wchar_t *)) {
	drop_files_callback = value;
}

void kinc_set_cut_callback(char *(*value)()) {
	cut_callback = value;
}

void kinc_set_copy_callback(char *(*value)()) {
	copy_callback = value;
}

void kinc_set_paste_callback(void (*value)(char *)) {
	paste_callback = value;
}

void Kinc_Internal_UpdateCallback() {
	if (update_callback != NULL) {
		update_callback();
	}
}

void Kinc_Internal_ForegroundCallback() {
	if (foreground_callback != NULL) {
		foreground_callback();
	}
}

void Kinc_Internal_ResumeCallback() {
	if (resume_callback != NULL) {
		resume_callback();
	}
}

void Kinc_Internal_PauseCallback() {
	if (pause_callback != NULL) {
		pause_callback();
	}
}

void Kinc_Internal_BackgroundCallback() {
	if (background_callback != NULL) {
		background_callback();
	}
}

void Kinc_Internal_ShutdownCallback() {
	if (shutdown_callback != NULL) {
		shutdown_callback();
	}
}

void Kinc_Internal_DropFilesCallback(wchar_t *filePath) {
	if (drop_files_callback != NULL) {
		drop_files_callback(filePath);
	}
}

char *Kinc_Internal_CutCallback() {
	if (cut_callback != NULL) {
		return cut_callback();
	}
	return NULL;
}

char *Kinc_Internal_CopyCallback() {
	if (copy_callback != NULL) {
		return copy_callback();
	}
	return NULL;
}

void Kinc_Internal_PasteCallback(char *value) {
	if (paste_callback != NULL) {
		paste_callback(value);
	}
}

static bool running = false;
static bool showWindowFlag = true;
static char name[1024] = {"Kore Application"};

const char *kinc_application_name() {
	return name;
}

#ifdef KORE_METAL
void shutdownMetalCompute();
#endif

void kinc_stop() {
	running = false;

	// TODO (DK) destroy graphics + windows, but afaik Application::~Application() was never called, so it's the same behavior now as well

	// for (int windowIndex = 0; windowIndex < sizeof(windowIds) / sizeof(int); ++windowIndex) {
	//	Graphics::destroy(windowIndex);
	//}

#ifdef KORE_METAL
	shutdownMetalCompute();
#endif
}

bool Kinc_Internal_Frame() {
	Kinc_Internal_UpdateCallback();
	Kinc_Internal_HandleMessages();
	return running;
}

void kinc_start() {
	running = true;

#if !defined(KORE_HTML5) && !defined(KORE_TIZEN) && !defined(KORE_XBOX_ONE)
	// if (Graphics::hasWindow()) Graphics::swapBuffers();

#if defined(KORE_IOS) || defined(KORE_MACOS)
	while (withAutoreleasepool(Kinc_Internal_Frame)) {
	}
#else
	while (Kinc_Internal_Frame()) {}
#endif
	Kinc_Internal_Shutdown();
#endif
}

int kinc_width() {
	return Kinc_WindowWidth(0);
}

int kinc_height() {
	return Kinc_WindowHeight(0);
}
