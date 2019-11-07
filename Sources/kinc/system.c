#include "pch.h"

#include "system.h"

#include "window.h"

#include <kinc/io/filereader.h>
#include <kinc/io/filewriter.h>

#include <stdlib.h>
#include <string.h>

#if !defined(KORE_HTML5) && !defined(KORE_ANDROID) && !defined(KORE_WINDOWS) && !defined(KORE_CONSOLE)
double kinc_time() {
	return kinc_timestamp() / kinc_frequency();
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
static void (*login_callback)() = NULL;
static void (*logout_callback)() = NULL;

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

void kinc_set_login_callback(void (*value)()) {
	login_callback = value;
}

void kinc_set_logout_callback(void (*value)()) {
	logout_callback = value;
}

void kinc_internal_update_callback() {
	if (update_callback != NULL) {
		update_callback();
	}
}

void kinc_internal_foreground_callback() {
	if (foreground_callback != NULL) {
		foreground_callback();
	}
}

void kinc_internal_resume_callback() {
	if (resume_callback != NULL) {
		resume_callback();
	}
}

void kinc_internal_pause_callback() {
	if (pause_callback != NULL) {
		pause_callback();
	}
}

void kinc_internal_background_callback() {
	if (background_callback != NULL) {
		background_callback();
	}
}

void kinc_internal_shutdown_callback() {
	if (shutdown_callback != NULL) {
		shutdown_callback();
	}
}

void kinc_internal_drop_files_callback(wchar_t *filePath) {
	if (drop_files_callback != NULL) {
		drop_files_callback(filePath);
	}
}

char *kinc_internal_cut_callback() {
	if (cut_callback != NULL) {
		return cut_callback();
	}
	return NULL;
}

char *kinc_internal_copy_callback() {
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

void kinc_internal_login_callback() {
	if (login_callback != NULL) {
		login_callback();
	}
}

void kinc_internal_logout_callback() {
	if (logout_callback != NULL) {
		logout_callback();
	}
}

static bool running = false;
// static bool showWindowFlag = true;
static char application_name[1024] = {"Kinc Application"};

const char *kinc_application_name() {
	return application_name;
}

void kinc_set_application_name(const char *name) {
	strcpy(application_name, name);
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

bool kinc_internal_frame() {
	kinc_internal_update_callback();
	kinc_internal_handle_messages();
	return running;
}

void kinc_start() {
	running = true;

#if !defined(KORE_HTML5) && !defined(KORE_TIZEN) && !defined(KORE_XBOX_ONE)
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

int kinc_width() {
	return kinc_window_width(0);
}

int kinc_height() {
	return kinc_window_height(0);
}

#if !defined(KORE_PS4) && !defined(KORE_SWITCH)
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

#if !defined(KORE_PS4)
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

void kinc_disallow_user_change() {}

void kinc_allow_user_change() {}

static uint8_t *current_file = NULL;
static size_t current_file_size = 0;

bool kinc_save_file_loaded() {
	return true;
}

uint8_t *kinc_get_save_file() {
	return current_file;
}

size_t kinc_get_save_file_size() {
	return current_file_size;
}

void kinc_load_save_file(const char *filename) {
	free(current_file);
	current_file = NULL;
	current_file_size = 0;

	kinc_file_reader_t reader;
	if (kinc_file_reader_open(&reader, filename, KINC_FILE_TYPE_SAVE)) {
		current_file_size = kinc_file_reader_size(&reader);
		current_file = (uint8_t *)malloc(current_file_size);
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

bool kinc_save_is_saving() {
	return false;
}

#endif
