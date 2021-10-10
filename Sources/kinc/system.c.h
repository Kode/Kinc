#include "system.h"

#include "window.h"

#include <kinc/io/filereader.h>
#include <kinc/io/filewriter.h>

#include <stdlib.h>
#include <string.h>

#if !defined(KORE_HTML5) && !defined(KORE_ANDROID) && !defined(KORE_WINDOWS) && !defined(KORE_CONSOLE)
double kinc_time(void) {
	return kinc_timestamp() / kinc_frequency();
}
#endif

static void (*update_callback)(void) = NULL;
static void (*foreground_callback)(void) = NULL;
static void (*background_callback)(void) = NULL;
static void (*pause_callback)(void) = NULL;
static void (*resume_callback)(void) = NULL;
static void (*shutdown_callback)(void) = NULL;
static void (*drop_files_callback)(wchar_t *) = NULL;
static char *(*cut_callback)(void) = NULL;
static char *(*copy_callback)(void) = NULL;
static void (*paste_callback)(char *) = NULL;
static void (*login_callback)(void) = NULL;
static void (*logout_callback)(void) = NULL;

#if defined(KORE_IOS) || defined(KORE_MACOS)
bool withAutoreleasepool(bool (*f)(void));
#endif

void kinc_set_update_callback(void (*value)(void)) {
	update_callback = value;
}

void kinc_set_foreground_callback(void (*value)(void)) {
	foreground_callback = value;
}

void kinc_set_resume_callback(void (*value)(void)) {
	resume_callback = value;
}

void kinc_set_pause_callback(void (*value)(void)) {
	pause_callback = value;
}

void kinc_set_background_callback(void (*value)(void)) {
	background_callback = value;
}

void kinc_set_shutdown_callback(void (*value)(void)) {
	shutdown_callback = value;
}

void kinc_set_drop_files_callback(void (*value)(wchar_t *)) {
	drop_files_callback = value;
}

void kinc_set_cut_callback(char *(*value)(void)) {
	cut_callback = value;
}

void kinc_set_copy_callback(char *(*value)(void)) {
	copy_callback = value;
}

void kinc_set_paste_callback(void (*value)(char *)) {
	paste_callback = value;
}

void kinc_set_login_callback(void (*value)(void)) {
	login_callback = value;
}

void kinc_set_logout_callback(void (*value)(void)) {
	logout_callback = value;
}

void kinc_internal_update_callback(void) {
	if (update_callback != NULL) {
		update_callback();
	}
}

void kinc_internal_foreground_callback(void) {
	if (foreground_callback != NULL) {
		foreground_callback();
	}
}

void kinc_internal_resume_callback(void) {
	if (resume_callback != NULL) {
		resume_callback();
	}
}

void kinc_internal_pause_callback(void) {
	if (pause_callback != NULL) {
		pause_callback();
	}
}

void kinc_internal_background_callback(void) {
	if (background_callback != NULL) {
		background_callback();
	}
}

void kinc_internal_shutdown_callback(void) {
	if (shutdown_callback != NULL) {
		shutdown_callback();
	}
}

void kinc_internal_drop_files_callback(wchar_t *filePath) {
	if (drop_files_callback != NULL) {
		drop_files_callback(filePath);
	}
}

char *kinc_internal_cut_callback(void) {
	if (cut_callback != NULL) {
		return cut_callback();
	}
	return NULL;
}

char *kinc_internal_copy_callback(void) {
	if (copy_callback != NULL) {
		return copy_callback();
	}
	return NULL;
}

void kinc_internal_paste_callback(char *value) {
	if (paste_callback != NULL) {
		paste_callback(value);
	}
}

void kinc_internal_login_callback(void) {
	if (login_callback != NULL) {
		login_callback();
	}
}

void kinc_internal_logout_callback(void) {
	if (logout_callback != NULL) {
		logout_callback();
	}
}

static bool running = false;
// static bool showWindowFlag = true;
static char application_name[1024] = {"Kinc Application"};

const char *kinc_application_name(void) {
	return application_name;
}

void kinc_set_application_name(const char *name) {
	kinc_string_copy(application_name, name);
}

#ifdef KORE_METAL
void shutdownMetalCompute(void);
#endif

void kinc_stop(void) {
	running = false;

	// TODO (DK) destroy graphics + windows, but afaik Application::~Application() was never called, so it's the same behavior now as well

	// for (int windowIndex = 0; windowIndex < sizeof(windowIds) / sizeof(int); ++windowIndex) {
	//	Graphics::destroy(windowIndex);
	//}

#ifdef KORE_METAL
	shutdownMetalCompute();
#endif
}

bool kinc_internal_frame(void) {
	kinc_internal_update_callback();
	kinc_internal_handle_messages();
	return running;
}

void kinc_start(void) {
	running = true;

#if !defined(KORE_HTML5) && !defined(KORE_TIZEN)
	// if (Graphics::hasWindow()) Graphics::swapBuffers();

#if defined(KORE_IOS) || defined(KORE_MACOS)
	while (withAutoreleasepool(kinc_internal_frame)) {
	}
#else
	while (kinc_internal_frame()) {
	}
#endif
	kinc_internal_shutdown();
#endif
}

int kinc_width(void) {
	return kinc_window_width(0);
}

int kinc_height(void) {
	return kinc_window_height(0);
}

#ifndef KHA
void kinc_memory_emergency(void) {}
#endif

#if !defined(KORE_SONY) && !defined(KORE_SWITCH)
static float safe_zone = 0.9f;

float kinc_safe_zone(void) {
	return safe_zone;
}

bool kinc_automatic_safe_zone() {
	return false;
}

void kinc_set_safe_zone(float value) {
	safe_zone = value;
}
#endif

#if !defined(KORE_SONY)
bool is_save_load_initialized(void) {
	return true;
}

bool is_ps4_japanese_button_style(void) {
	return false;
}

bool is_save_load_broken(void) {
	return false;
}
#endif

#if !defined(KORE_CONSOLE)

#define SAVE_RESULT_NONE 0
#define SAVE_RESULT_SUCCESS 1
#define SAVE_RESULT_FAILURE 2
volatile int save_result = SAVE_RESULT_SUCCESS;

void kinc_disallow_user_change(void) {}

void kinc_allow_user_change(void) {}

static uint8_t *current_file = NULL;
static size_t current_file_size = 0;

bool kinc_save_file_loaded(void) {
	return true;
}

uint8_t *kinc_get_save_file(void) {
	return current_file;
}

size_t kinc_get_save_file_size(void) {
	return current_file_size;
}

void kinc_load_save_file(const char *filename) {
	kinc_free(current_file);
	current_file = NULL;
	current_file_size = 0;

	kinc_file_reader_t reader;
	if (kinc_file_reader_open(&reader, filename, KINC_FILE_TYPE_SAVE)) {
		current_file_size = kinc_file_reader_size(&reader);
		current_file = (uint8_t *)kinc_allocate(current_file_size);
		kinc_file_reader_read(&reader, current_file, current_file_size);
		kinc_file_reader_close(&reader);
	}
}

void kinc_save_save_file(const char *filename, uint8_t *data, size_t size) {
	kinc_file_writer_t writer;
	if (kinc_file_writer_open(&writer, filename)) {
		kinc_file_writer_write(&writer, data, (int)size);
		kinc_file_writer_close(&writer);
	}
}

bool kinc_save_is_saving(void) {
	return false;
}

bool kinc_waiting_for_login(void) {
	return false;
}

#if !defined(KORE_WINDOWS) && !defined(KORE_LINUX) && !defined(KORE_MACOS)
void kinc_copy_to_clipboard(const char *text) {}
#endif

#endif
