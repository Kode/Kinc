#include "pch.h"
#include "KeyEvent.h"

using namespace Kore;

KeyEvent::KeyEvent(int code) : code(code) {

}

bool KeyEvent::isChar() {
	return code >= Key_Space && code <= Key_Z;
}

wchar_t KeyEvent::tochar() {
	return (wchar_t)code;
}

int KeyEvent::keycode() {
	return code;
}
