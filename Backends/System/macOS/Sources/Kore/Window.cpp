#include "pch.h"

#include <Kore/Graphics4/Graphics.h>

#include <Kinc/Bridge.h>
#include <Kinc/Display.h>
#include <Kinc/Window.h>

int Kinc_WindowX() {
	return 0;
}

int Kinc_WindowY() {
	return 0;
}

void Kinc_WindowResize(int width, int height) {
	
}

void Kinc_WindowMove(int x, int y) {
	
}

void Kinc_WindowChangeFramebuffer(Kinc_FramebufferOptions *frame) {
	Kinc_Bridge_G4_Internal_ChangeFramebuffer(0, frame);
}

void Kinc_WindowChangeWindowFeatures(int window, int features) {
	
}

void Kinc_WindowDestroy(int window) {
	
}

void Kinc_WindowShow() {
	
}

void Kinc_WindowHide() {
	
}

void Kinc_WindowSetTitle(const char *title) {
	
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

Kinc_WindowMode Kinc_WindowGetMode() {
	return KINC_WINDOW_MODE_WINDOW;
}
