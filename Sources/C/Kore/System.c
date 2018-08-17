#include "pch.h"

#include "System.h"

#include "Window.h"

#if !defined(KORE_HTML5) && !defined(KORE_ANDROID) && !defined(KORE_WINDOWS) && !defined(KORE_CONSOLE)
double Kore_Time() {
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

void Kore_SetUpdateCallback(void (*value)()) {
	update_callback = value;
}

void Kore_SetForegroundCallback(void (*value)()) {
	foreground_callback = value;
}

void Kore_SetResumeCallback(void (*value)()) {
	resume_callback = value;
}

void Kore_SetPauseCallback(void (*value)()) {
	pause_callback = value;
}

void Kore_SetBackgroundCallback(void (*value)()) {
	background_callback = value;
}

void Kore_SetShutdownCallback(void (*value)()) {
	shutdown_callback = value;
}

void Kore_SetDropFilesCallback(void (*value)(wchar_t *)) {
	drop_files_callback = value;
}

void Kore_SetCutCallback(char *(*value)()) {
	cut_callback = value;
}

void Kore_SetCopyCallback(char *(*value)()) {
	copy_callback = value;
}

void Kore_SetPasteCallback(void (*value)(char *)) {
	paste_callback = value;
}

void Kore_Internal_UpdateCallback() {
	if (update_callback != NULL) {
		update_callback();
	}
}

void Kore_Internal_ForegroundCallback() {
	if (foreground_callback != NULL) {
		foreground_callback();
	}
}

void Kore_Internal_ResumeCallback() {
	if (resume_callback != NULL) {
		resume_callback();
	}
}

void Kore_Internal_PauseCallback() {
	if (pause_callback != NULL) {
		pause_callback();
	}
}

void Kore_Internal_BackgroundCallback() {
	if (background_callback != NULL) {
		background_callback();
	}
}

void Kore_Internal_ShutdownCallback() {
	if (shutdown_callback != NULL) {
		shutdown_callback();
	}
}

void Kore_Internal_DropFilesCallback(wchar_t *filePath) {
	if (drop_files_callback != NULL) {
		drop_files_callback(filePath);
	}
}

char *Kore_Internal_CutCallback() {
	if (cut_callback != NULL) {
		return cut_callback();
	}
	return NULL;
}

char *Kore_Internal_CopyCallback() {
	if (copy_callback != NULL) {
		return copy_callback();
	}
	return NULL;
}

void Kore_Internal_PasteCallback(char *value) {
	if (paste_callback != NULL) {
		paste_callback(value);
	}
}

static bool running = false;
static bool showWindowFlag = true;
static char name[1024] = {"Kore Application"};

const char *Kore_ApplicationName() {
	return name;
}

#ifdef KORE_METAL
void shutdownMetalCompute();
#endif

void Kore_Stop() {
	running = false;

	// TODO (DK) destroy graphics + windows, but afaik Application::~Application() was never called, so it's the same behavior now as well

	// for (int windowIndex = 0; windowIndex < sizeof(windowIds) / sizeof(int); ++windowIndex) {
	//	Graphics::destroy(windowIndex);
	//}

#ifdef KORE_METAL
	shutdownMetalCompute();
#endif
}

bool Kore_Internal_Frame() {
	Kore_Internal_UpdateCallback();
	Kore_Internal_HandleMessages();
	return running;
}

void Kore_Start() {
	running = true;

#if !defined(KORE_HTML5) && !defined(KORE_TIZEN) && !defined(KORE_XBOX_ONE)
	// if (Graphics::hasWindow()) Graphics::swapBuffers();

#if defined(KORE_IOS) || defined(KORE_MACOS)
	while (withAutoreleasepool(Kore_Internal_Frame))
		;
#else
	while (Kore_Internal_Frame())
		;
#endif
	Kore_Internal_Shutdown();
#endif
}

int Kore_Width() {
	return Kore_WindowWidth(0);
}

int Kore_Height() {
	return Kore_WindowHeight(0);
}
