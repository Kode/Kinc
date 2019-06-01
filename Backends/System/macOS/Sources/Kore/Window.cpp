#include "pch.h"

#include <Kore/Graphics4/Graphics.h>

#include <kinc/bridge.h>
#include <kinc/display.h>
#include <kinc/window.h>

int Kinc_WindowX(int window) {
	return 0;
}

int Kinc_WindowY(int window) {
	return 0;
}

void Kinc_WindowResize(int window, int width, int height) {
	
}

void Kinc_WindowMove(int window, int x, int y) {
	
}

void Kinc_WindowChangeFramebuffer(int window, Kinc_FramebufferOptions *frame) {
	Kinc_Bridge_G4_Internal_ChangeFramebuffer(0, frame);
}

void Kinc_WindowChangeFeatures(int window, int features) {
	
}

void Kinc_WindowChangeMode(int window, Kinc_WindowMode mode) {
	
}

void Kinc_WindowDestroy(int window) {
	
}

void Kinc_WindowShow(int window) {
	
}

void Kinc_WindowHide(int window) {
	
}

void Kinc_WindowSetTitle(int window, const char *title) {
	
}

int Kinc_WindowCreate(Kinc_WindowOptions *win, Kinc_FramebufferOptions *frame) {
	return 0;
}

void Kinc_WindowSetResizeCallback(int window, void (*callback)(int x, int y, void* data), void* data) {
	//**_data.resizeCallback = callback;
	//**_data.resizeCallbackData = data;
}

void Kinc_WindowSetPpiChangedCallback(int window, void(*callback)(int ppi, void* data), void* data) {
	
}

Kinc_WindowMode Kinc_WindowGetMode(int window) {
	return KINC_WINDOW_MODE_WINDOW;
}

int Kinc_WindowDisplay(int window) {
	return 0;
}
