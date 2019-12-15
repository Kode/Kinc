#include "pch.h"

#ifdef KORE_G4ONG5
#include <Kore/Graphics5/Graphics.h>
#elif KORE_G4
#include <Kore/Graphics4/Graphics.h>
#else
#include <Kore/Graphics3/Graphics.h>
#endif

#include <kinc/input/gamepad.h>

#include <Kore/Display.h>
#include <Kore/Window.h>
#include <Kore/Windows.h>

#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/input/pen.h>
#include <kinc/log.h>
#include <kinc/system.h>
#include <kinc/threads/thread.h>
#include <kinc/window.h>

#define DIRECTINPUT_VERSION 0x0800
#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#include <dinput.h>
#include <oleauto.h>
#include <stdio.h>
#include <wbemidl.h>

#ifdef KORE_OCULUS
#include "Kore/Vr/VrInterface.h"
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <XInput.h>
#include <exception>
#include <shlobj.h>

#ifdef KORE_G4ONG5
#define Graphics Graphics5
#elif KORE_G4
#define Graphics Graphics4
#else
#define Graphics Graphics3
#endif

extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

typedef BOOL(WINAPI *GetPointerInfoType)(UINT32 pointerId, POINTER_INFO *pointerInfo);
static GetPointerInfoType MyGetPointerInfo = NULL;
typedef BOOL(WINAPI *GetPointerPenInfoType)(UINT32 pointerId, POINTER_PEN_INFO *penInfo);
static GetPointerPenInfoType MyGetPointerPenInfo = NULL;
typedef BOOL(WINAPI *EnableNonClientDpiScalingType)(HWND hwnd);
static EnableNonClientDpiScalingType MyEnableNonClientDpiScaling = NULL;

static int mouseX, mouseY;
static bool keyPressed[256];
int keyTranslated[256]; // http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx

static void initKeyTranslation() {
	for (int i = 0; i < 256; ++i) keyTranslated[i] = KINC_KEY_UNKNOWN;

	keyTranslated[VK_BACK] = KINC_KEY_BACKSPACE;
	keyTranslated[VK_TAB] = KINC_KEY_TAB;
	keyTranslated[VK_CLEAR] = KINC_KEY_CLEAR;
	keyTranslated[VK_RETURN] = KINC_KEY_RETURN;
	keyTranslated[VK_SHIFT] = KINC_KEY_SHIFT;
	keyTranslated[VK_CONTROL] = KINC_KEY_CONTROL;
	keyTranslated[VK_MENU] = KINC_KEY_ALT;
	keyTranslated[VK_PAUSE] = KINC_KEY_PAUSE;
	keyTranslated[VK_CAPITAL] = KINC_KEY_CAPS_LOCK;
	keyTranslated[VK_KANA] = KINC_KEY_KANA;
	// keyTranslated[VK_HANGUEL]
	keyTranslated[VK_HANGUL] = KINC_KEY_HANGUL;
	keyTranslated[VK_JUNJA] = KINC_KEY_JUNJA;
	keyTranslated[VK_FINAL] = KINC_KEY_FINAL;
	keyTranslated[VK_HANJA] = KINC_KEY_HANJA;
	keyTranslated[VK_KANJI] = KINC_KEY_KANJI;
	keyTranslated[VK_ESCAPE] = KINC_KEY_ESCAPE;
	// keyTranslated[VK_CONVERT]
	// keyTranslated[VK_NONCONVERT
	// keyTranslated[VK_ACCEPT
	// keyTranslated[VK_MODECHANGE
	keyTranslated[VK_SPACE] = KINC_KEY_SPACE;
	keyTranslated[VK_PRIOR] = KINC_KEY_PAGE_UP;
	keyTranslated[VK_NEXT] = KINC_KEY_PAGE_DOWN;
	keyTranslated[VK_END] = KINC_KEY_END;
	keyTranslated[VK_HOME] = KINC_KEY_HOME;
	keyTranslated[VK_LEFT] = KINC_KEY_LEFT;
	keyTranslated[VK_UP] = KINC_KEY_UP;
	keyTranslated[VK_RIGHT] = KINC_KEY_RIGHT;
	keyTranslated[VK_DOWN] = KINC_KEY_DOWN;
	// keyTranslated[VK_SELECT
	keyTranslated[VK_PRINT] = KINC_KEY_PRINT;
	// keyTranslated[VK_EXECUTE
	// keyTranslated[VK_SNAPSHOT
	keyTranslated[VK_INSERT] = KINC_KEY_INSERT;
	keyTranslated[VK_DELETE] = KINC_KEY_DELETE;
	keyTranslated[VK_HELP] = KINC_KEY_HELP;
	keyTranslated[0x30] = KINC_KEY_0;
	keyTranslated[0x31] = KINC_KEY_1;
	keyTranslated[0x32] = KINC_KEY_2;
	keyTranslated[0x33] = KINC_KEY_3;
	keyTranslated[0x34] = KINC_KEY_4;
	keyTranslated[0x35] = KINC_KEY_5;
	keyTranslated[0x36] = KINC_KEY_6;
	keyTranslated[0x37] = KINC_KEY_7;
	keyTranslated[0x38] = KINC_KEY_8;
	keyTranslated[0x39] = KINC_KEY_9;
	keyTranslated[0x41] = KINC_KEY_A;
	keyTranslated[0x42] = KINC_KEY_B;
	keyTranslated[0x43] = KINC_KEY_C;
	keyTranslated[0x44] = KINC_KEY_D;
	keyTranslated[0x45] = KINC_KEY_E;
	keyTranslated[0x46] = KINC_KEY_F;
	keyTranslated[0x47] = KINC_KEY_G;
	keyTranslated[0x48] = KINC_KEY_H;
	keyTranslated[0x49] = KINC_KEY_I;
	keyTranslated[0x4A] = KINC_KEY_J;
	keyTranslated[0x4B] = KINC_KEY_K;
	keyTranslated[0x4C] = KINC_KEY_L;
	keyTranslated[0x4D] = KINC_KEY_M;
	keyTranslated[0x4E] = KINC_KEY_N;
	keyTranslated[0x4F] = KINC_KEY_O;
	keyTranslated[0x50] = KINC_KEY_P;
	keyTranslated[0x51] = KINC_KEY_Q;
	keyTranslated[0x52] = KINC_KEY_R;
	keyTranslated[0x53] = KINC_KEY_S;
	keyTranslated[0x54] = KINC_KEY_T;
	keyTranslated[0x55] = KINC_KEY_U;
	keyTranslated[0x56] = KINC_KEY_V;
	keyTranslated[0x57] = KINC_KEY_W;
	keyTranslated[0x58] = KINC_KEY_X;
	keyTranslated[0x59] = KINC_KEY_Y;
	keyTranslated[0x5A] = KINC_KEY_Z;
	keyTranslated[VK_LWIN] = KINC_KEY_WIN;
	keyTranslated[VK_RWIN] = KINC_KEY_WIN;
	keyTranslated[VK_APPS] = KINC_KEY_CONTEXT_MENU;
	// keyTranslated[VK_SLEEP
	keyTranslated[VK_NUMPAD0] = KINC_KEY_NUMPAD_0;
	keyTranslated[VK_NUMPAD1] = KINC_KEY_NUMPAD_1;
	keyTranslated[VK_NUMPAD2] = KINC_KEY_NUMPAD_2;
	keyTranslated[VK_NUMPAD3] = KINC_KEY_NUMPAD_3;
	keyTranslated[VK_NUMPAD4] = KINC_KEY_NUMPAD_4;
	keyTranslated[VK_NUMPAD5] = KINC_KEY_NUMPAD_5;
	keyTranslated[VK_NUMPAD6] = KINC_KEY_NUMPAD_6;
	keyTranslated[VK_NUMPAD7] = KINC_KEY_NUMPAD_7;
	keyTranslated[VK_NUMPAD8] = KINC_KEY_NUMPAD_8;
	keyTranslated[VK_NUMPAD9] = KINC_KEY_NUMPAD_9;
	keyTranslated[VK_MULTIPLY] = KINC_KEY_MULTIPLY;
	keyTranslated[VK_ADD] = KINC_KEY_ADD;
	// keyTranslated[VK_SEPARATOR
	keyTranslated[VK_SUBTRACT] = KINC_KEY_SUBTRACT;
	keyTranslated[VK_DECIMAL] = KINC_KEY_DECIMAL;
	keyTranslated[VK_DIVIDE] = KINC_KEY_DIVIDE;
	keyTranslated[VK_F1] = KINC_KEY_F1;
	keyTranslated[VK_F2] = KINC_KEY_F2;
	keyTranslated[VK_F3] = KINC_KEY_F3;
	keyTranslated[VK_F4] = KINC_KEY_F4;
	keyTranslated[VK_F5] = KINC_KEY_F5;
	keyTranslated[VK_F6] = KINC_KEY_F6;
	keyTranslated[VK_F7] = KINC_KEY_F7;
	keyTranslated[VK_F8] = KINC_KEY_F8;
	keyTranslated[VK_F9] = KINC_KEY_F9;
	keyTranslated[VK_F10] = KINC_KEY_F10;
	keyTranslated[VK_F11] = KINC_KEY_F11;
	keyTranslated[VK_F12] = KINC_KEY_F12;
	keyTranslated[VK_F13] = KINC_KEY_F13;
	keyTranslated[VK_F14] = KINC_KEY_F14;
	keyTranslated[VK_F15] = KINC_KEY_F15;
	keyTranslated[VK_F16] = KINC_KEY_F16;
	keyTranslated[VK_F17] = KINC_KEY_F17;
	keyTranslated[VK_F18] = KINC_KEY_F18;
	keyTranslated[VK_F19] = KINC_KEY_F19;
	keyTranslated[VK_F20] = KINC_KEY_F20;
	keyTranslated[VK_F21] = KINC_KEY_F21;
	keyTranslated[VK_F22] = KINC_KEY_F22;
	keyTranslated[VK_F23] = KINC_KEY_F23;
	keyTranslated[VK_F24] = KINC_KEY_F24;
	keyTranslated[VK_NUMLOCK] = KINC_KEY_NUM_LOCK;
	keyTranslated[VK_SCROLL] = KINC_KEY_SCROLL_LOCK;
	// 0x92-96 //OEM specific
	keyTranslated[VK_LSHIFT] = KINC_KEY_SHIFT;
	keyTranslated[VK_RSHIFT] = KINC_KEY_SHIFT;
	keyTranslated[VK_LCONTROL] = KINC_KEY_CONTROL;
	keyTranslated[VK_RCONTROL] = KINC_KEY_CONTROL;
	// keyTranslated[VK_LMENU
	// keyTranslated[VK_RMENU
	// keyTranslated[VK_BROWSER_BACK
	// keyTranslated[VK_BROWSER_FORWARD
	// keyTranslated[VK_BROWSER_REFRESH
	// keyTranslated[VK_BROWSER_STOP
	// keyTranslated[VK_BROWSER_SEARCH
	// keyTranslated[VK_BROWSER_FAVORITES
	// keyTranslated[VK_BROWSER_HOME
	// keyTranslated[VK_VOLUME_MUTE
	// keyTranslated[VK_VOLUME_DOWN
	// keyTranslated[VK_VOLUME_UP
	// keyTranslated[VK_MEDIA_NEXT_TRACK
	// keyTranslated[VK_MEDIA_PREV_TRACK
	// keyTranslated[VK_MEDIA_STOP
	// keyTranslated[VK_MEDIA_PLAY_PAUSE
	// keyTranslated[VK_LAUNCH_MAIL
	// keyTranslated[VK_LAUNCH_MEDIA_SELECT
	// keyTranslated[VK_LAUNCH_APP1
	// keyTranslated[VK_LAUNCH_APP2
	keyTranslated[VK_OEM_1] = KINC_KEY_SEMICOLON; // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key
	keyTranslated[VK_OEM_PLUS] = KINC_KEY_PLUS;
	keyTranslated[VK_OEM_COMMA] = KINC_KEY_COMMA;
	keyTranslated[VK_OEM_MINUS] = KINC_KEY_HYPHEN_MINUS;
	keyTranslated[VK_OEM_PERIOD] = KINC_KEY_PERIOD;
	keyTranslated[VK_OEM_2] = KINC_KEY_SLASH;         // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
	keyTranslated[VK_OEM_3] = KINC_KEY_BACK_QUOTE;    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
	keyTranslated[VK_OEM_4] = KINC_KEY_OPEN_BRACKET;  // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
	keyTranslated[VK_OEM_5] = KINC_KEY_BACK_SLASH;    // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
	keyTranslated[VK_OEM_6] = KINC_KEY_CLOSE_BRACKET; // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
	keyTranslated[VK_OEM_7] = KINC_KEY_QUOTE;         // Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the
	                                          // 'single-quote/double-quote' key keyTranslated[VK_OEM_8 //Used for miscellaneous characters; it can vary by
	                                          // keyboard. keyTranslated[0xE1 //OEM specific keyTranslated[VK_OEM_102 //Either the angle bracket key or the
	                                          // backslash key on the RT 102-key keyboard 0xE3-E4 //OEM specific keyTranslated[VK_PROCESSKEY 0xE6 //OEM specific
	// keyTranslated[VK_PACKET //Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value
	// used for non-keyboard input methods.
	// 0xE9-F5 //OEM specific
	// keyTranslated[VK_ATTN
	// keyTranslated[VK_CRSEL
	// keyTranslated[VK_EXSEL
	// keyTranslated[VK_EREOF
	// keyTranslated[VK_PLAY
	// keyTranslated[VK_ZOOM
	// keyTranslated[VK_NONAME
	// keyTranslated[VK_PA1
	// keyTranslated[PA1 key
	// keyTranslated[VK_OEM_CLEAR
}

static bool detectGamepad = true;
static bool gamepadFound = false;
static unsigned r = 0;

namespace {
	wchar_t toUnicode(WPARAM wParam, LPARAM lParam) {
		wchar_t buffer[11];
		BYTE state[256];
		GetKeyboardState(state);
		ToUnicode((UINT)wParam, (lParam >> 8) & 0xFFFFFF00, state, buffer, 10, 0);
		return buffer[0];
	}
}

extern "C" LRESULT WINAPI KoreWindowsMessageProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int windowId;
	DWORD pointerId;
	POINTER_INFO pointerInfo = {NULL};
	POINTER_PEN_INFO penInfo = {NULL};
	static bool controlDown = false;

	switch (msg) {
	case WM_NCCREATE:
		if (MyEnableNonClientDpiScaling != nullptr) {
			MyEnableNonClientDpiScaling(hWnd);
		}
		break;
	case WM_DPICHANGED: {
		int window = kinc_windows_window_index_from_hwnd(hWnd);
		if (window >= 0) {
			kinc_internal_call_ppi_changed_callback(window, LOWORD(wParam));
		}
		break;
	}
	case WM_MOVE:
	case WM_MOVING:
	case WM_SIZING:
		// Scheduler::breakTime();
		break;
	case WM_SIZE: {
		int window = kinc_windows_window_index_from_hwnd(hWnd);
		if (window >= 0) {
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			Kore::Graphics::_resize(window, width, height);
			kinc_internal_call_resize_callback(window, width, height);
		}
		break;
	}
	case WM_DESTROY:
		kinc_stop();
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE) {
			kinc_internal_mouse_window_activated(kinc_windows_window_index_from_hwnd(hWnd));
			kinc_internal_foreground_callback();
		}
		else {
			kinc_internal_mouse_window_deactivated(kinc_windows_window_index_from_hwnd(hWnd));
			kinc_internal_background_callback();
		}
		break;
	case WM_MOUSELEAVE:
		windowId = kinc_windows_window_index_from_hwnd(hWnd);
		//**windows[windowId]->isMouseInside = false;
		kinc_internal_mouse_trigger_leave_window(windowId);
		break;
	case WM_MOUSEMOVE:
		windowId = kinc_windows_window_index_from_hwnd(hWnd);
		/*if (!windows[windowId]->isMouseInside) {
		    windows[windowId]->isMouseInside = true;
		    TRACKMOUSEEVENT tme;
		    tme.cbSize = sizeof(TRACKMOUSEEVENT);
		    tme.dwFlags = TME_LEAVE;
		    tme.hwndTrack = hWnd;
		    TrackMouseEvent(&tme);
		}*/
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		kinc_internal_mouse_trigger_move(windowId, mouseX, mouseY);
		break;
	case WM_LBUTTONDOWN:
		if (!kinc_mouse_is_locked(kinc_windows_window_index_from_hwnd(hWnd))) SetCapture(hWnd);
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		kinc_internal_mouse_trigger_press(kinc_windows_window_index_from_hwnd(hWnd), 0, mouseX, mouseY);
		break;
	case WM_LBUTTONUP:
		if (!kinc_mouse_is_locked(kinc_windows_window_index_from_hwnd(hWnd))) ReleaseCapture();
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		kinc_internal_mouse_trigger_release(kinc_windows_window_index_from_hwnd(hWnd), 0, mouseX, mouseY);
		break;
	case WM_RBUTTONDOWN:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		kinc_internal_mouse_trigger_press(kinc_windows_window_index_from_hwnd(hWnd), 1, mouseX, mouseY);
		break;
	case WM_RBUTTONUP:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		kinc_internal_mouse_trigger_release(kinc_windows_window_index_from_hwnd(hWnd), 1, mouseX, mouseY);
		break;
	case WM_MBUTTONDOWN:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		kinc_internal_mouse_trigger_press(kinc_windows_window_index_from_hwnd(hWnd), 2, mouseX, mouseY);
		break;
	case WM_MBUTTONUP:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		kinc_internal_mouse_trigger_release(kinc_windows_window_index_from_hwnd(hWnd), 2, mouseX, mouseY);
		break;
	case WM_XBUTTONDOWN:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		kinc_internal_mouse_trigger_press(kinc_windows_window_index_from_hwnd(hWnd), HIWORD(wParam) + 2, mouseX, mouseY);
		break;
	case WM_XBUTTONUP:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		kinc_internal_mouse_trigger_release(kinc_windows_window_index_from_hwnd(hWnd), HIWORD(wParam) + 2, mouseX, mouseY);
		break;
	case WM_MOUSEWHEEL:
		kinc_internal_mouse_trigger_scroll(kinc_windows_window_index_from_hwnd(hWnd), GET_WHEEL_DELTA_WPARAM(wParam) / -120);
		break;
	case WM_POINTERDOWN:
		pointerId = GET_POINTERID_WPARAM(wParam);
		MyGetPointerInfo(pointerId, &pointerInfo);
		if (pointerInfo.pointerType == PT_PEN) {
			MyGetPointerPenInfo(pointerId, &penInfo);
			ScreenToClient(hWnd, &pointerInfo.ptPixelLocation);
			kinc_internal_pen_trigger_press(kinc_windows_window_index_from_hwnd(hWnd), pointerInfo.ptPixelLocation.x, pointerInfo.ptPixelLocation.y,
			                                float(penInfo.pressure) / 1024.0f);
		}
		break;
	case WM_POINTERUP:
		pointerId = GET_POINTERID_WPARAM(wParam);
		MyGetPointerInfo(pointerId, &pointerInfo);
		if (pointerInfo.pointerType == PT_PEN) {
			MyGetPointerPenInfo(pointerId, &penInfo);
			ScreenToClient(hWnd, &pointerInfo.ptPixelLocation);
			kinc_internal_pen_trigger_release(kinc_windows_window_index_from_hwnd(hWnd), pointerInfo.ptPixelLocation.x, pointerInfo.ptPixelLocation.y,
			                                  float(penInfo.pressure) / 1024.0f);
		}
		break;
	case WM_POINTERUPDATE:
		pointerId = GET_POINTERID_WPARAM(wParam);
		MyGetPointerInfo(pointerId, &pointerInfo);
		if (pointerInfo.pointerType == PT_PEN) {
			MyGetPointerPenInfo(pointerId, &penInfo);
			ScreenToClient(hWnd, &pointerInfo.ptPixelLocation);
			kinc_internal_pen_trigger_move(kinc_windows_window_index_from_hwnd(hWnd), pointerInfo.ptPixelLocation.x, pointerInfo.ptPixelLocation.y,
			                               float(penInfo.pressure) / 1024.0f);
		}
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (!keyPressed[wParam]) {
			keyPressed[wParam] = true;

			if (keyTranslated[wParam] == KINC_KEY_CONTROL) {
				controlDown = true;
			}
			else {
				if (controlDown && keyTranslated[wParam] == KINC_KEY_X) {
					char *text = kinc_internal_cut_callback();
					if (text != nullptr) {
						wchar_t wtext[4096];
						MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, 4096);
						OpenClipboard(hWnd);
						EmptyClipboard();
						size_t size = (wcslen(wtext) + 1) * sizeof(wchar_t);
						HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, size);
						void *data = GlobalLock(handle);
						memcpy(data, wtext, size);
						GlobalUnlock(handle);
						SetClipboardData(CF_UNICODETEXT, handle);
						CloseClipboard();
					}
				}

				if (controlDown && keyTranslated[wParam] == KINC_KEY_C) {
					char *text = kinc_internal_copy_callback();
					if (text != nullptr) {
						wchar_t wtext[4096];
						MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, 4096);
						OpenClipboard(hWnd);
						EmptyClipboard();
						size_t size = (wcslen(wtext) + 1) * sizeof(wchar_t);
						HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, size);
						void *data = GlobalLock(handle);
						memcpy(data, wtext, size);
						GlobalUnlock(handle);
						SetClipboardData(CF_UNICODETEXT, handle);
						CloseClipboard();
					}
				}

				if (controlDown && keyTranslated[wParam] == KINC_KEY_V) {
					if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
						OpenClipboard(hWnd);
						HANDLE handle = GetClipboardData(CF_UNICODETEXT);
						if (handle != nullptr) {
							wchar_t *wtext = (wchar_t *)GlobalLock(handle);
							if (wtext != nullptr) {
								char text[4096];
								WideCharToMultiByte(CP_UTF8, 0, wtext, -1, text, 4096, nullptr, nullptr);
								kinc_internal_paste_callback(text);
								GlobalUnlock(handle);
							}
						}
						CloseClipboard();
					}
				}
			}

			kinc_internal_keyboard_trigger_key_down(keyTranslated[wParam]);
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		keyPressed[wParam] = false;

		if (keyTranslated[wParam] == KINC_KEY_CONTROL) {
			controlDown = false;
		}

		kinc_internal_keyboard_trigger_key_up(keyTranslated[wParam]);
		break;
	case WM_CHAR:
		switch (wParam) {
		case 0x08: // backspace
			break;
		case 0x0A: // linefeed
			kinc_internal_keyboard_trigger_key_press(L'\n');
			break;
		case 0x1B: // escape
			break;
		case 0x09: // tab
			kinc_internal_keyboard_trigger_key_press(L'\t');
			break;
		case 0x0D: // carriage return
			kinc_internal_keyboard_trigger_key_press(L'\r');
			break;
		default:
			kinc_internal_keyboard_trigger_key_press((unsigned)wParam);
			break;
		}
		break;
	case WM_SYSCOMMAND:
		switch (wParam) {
		case SC_KEYMENU:
			return 0; // Prevent from happening
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0; // Prevent from happening

		// Pause game when window is minimized, continue when it's restored or maximized.
		//
		// Unfortunately, if the game would continue to run when minimized, the graphics in
		// the Windows Vista/7 taskbar would not be updated, even when Direct3DDevice::Present()
		// is called without error. I do not know why.
		case SC_MINIMIZE:
			// Scheduler::haltTime(); // haltTime()/unhaltTime() is incremental, meaning that this doesn't interfere with when the game itself calls these
			// functions
			break;
		case SC_RESTORE:
		case SC_MAXIMIZE:
			// Scheduler::unhaltTime();
			break;
		}
		break;
	case WM_DEVICECHANGE:
		detectGamepad = true;
		break;
	case WM_DROPFILES:
		HDROP hDrop = (HDROP)wParam;
		unsigned count = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, NULL);
		for (unsigned i = 0; i < count; ++i) {
			wchar_t filePath[260];
			if (DragQueryFileW(hDrop, i, filePath, 260)) {
				kinc_internal_drop_files_callback(filePath);
			}
		}
		DragFinish(hDrop);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

namespace {
	float axes[12 * 6];
	float buttons[12 * 16];

	typedef DWORD(WINAPI *XInputGetStateType)(DWORD dwUserIndex, XINPUT_STATE *pState);
	XInputGetStateType InputGetState = nullptr;
}

void loadXInput() {
	HMODULE lib = LoadLibrary(L"xinput1_4.dll");
	if (lib == nullptr) {
		lib = LoadLibrary(L"xinput1_3.dll");
	}
	if (lib == nullptr) {
		lib = LoadLibrary(L"xinput9_1_0.dll");
	}

	if (lib != nullptr) {
		InputGetState = (XInputGetStateType)GetProcAddress(lib, "XInputGetState");
	}
}

namespace {
	IDirectInput8 *di_instance = nullptr;
	IDirectInputDevice8 *di_pads[XUSER_MAX_COUNT];
	DIJOYSTATE2 di_padState[XUSER_MAX_COUNT];
	DIJOYSTATE2 di_lastPadState[XUSER_MAX_COUNT];
	DIDEVCAPS di_deviceCaps[XUSER_MAX_COUNT];
	int padCount = 0;

	void cleanupPad(int padIndex) {
		if (di_pads[padIndex] != nullptr) {
			di_pads[padIndex]->Unacquire();
			di_pads[padIndex]->Release();
			di_pads[padIndex] = 0;
		}
	}

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)                                                                                                                                        \
	if (x != NULL) {                                                                                                                                           \
		x->Release();                                                                                                                                          \
		x = NULL;                                                                                                                                              \
	}
#endif

	// From
	//-----------------------------------------------------------------------------
	// Enum each PNP device using WMI and check each device ID to see if it contains
	// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
	// Unfortunately this information can not be found by just using DirectInput
	//-----------------------------------------------------------------------------
	BOOL IsXInputDevice(const GUID *pGuidProductFromDirectInput) {
		IWbemLocator *pIWbemLocator = NULL;
		IEnumWbemClassObject *pEnumDevices = NULL;
		IWbemClassObject *pDevices[20] = {0};
		IWbemServices *pIWbemServices = NULL;
		BSTR bstrNamespace = NULL;
		BSTR bstrDeviceID = NULL;
		BSTR bstrClassName = NULL;
		DWORD uReturned = 0;
		bool bIsXinputDevice = false;
		UINT iDevice = 0;
		VARIANT var;
		HRESULT hr;

		// CoInit if needed
		hr = CoInitialize(NULL);
		bool bCleanupCOM = SUCCEEDED(hr);

		// Create WMI
		hr = CoCreateInstance(__uuidof(WbemLocator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID *)&pIWbemLocator);
		if (FAILED(hr) || pIWbemLocator == NULL) goto LCleanup;

		bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2");
		if (bstrNamespace == NULL) goto LCleanup;
		bstrClassName = SysAllocString(L"Win32_PNPEntity");
		if (bstrClassName == NULL) goto LCleanup;
		bstrDeviceID = SysAllocString(L"DeviceID");
		if (bstrDeviceID == NULL) goto LCleanup;

		// Connect to WMI
		hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L, 0L, NULL, NULL, &pIWbemServices);
		if (FAILED(hr) || pIWbemServices == NULL) goto LCleanup;

		// Switch security level to IMPERSONATE.
		CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

		hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices);
		if (FAILED(hr) || pEnumDevices == NULL) goto LCleanup;

		// Loop over all devices
		for (;;) {
			// Get 20 at a time
			hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
			if (FAILED(hr)) goto LCleanup;
			if (uReturned == 0) break;

			for (iDevice = 0; iDevice < uReturned; iDevice++) {
				// For each device, get its device ID
				hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
				if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL) {
					// Check if the device ID contains "IG_".  If it does, then it's an XInput device
					// This information can not be found from DirectInput
					if (wcsstr(var.bstrVal, L"IG_")) {
						// If it does, then get the VID/PID from var.bstrVal
						DWORD dwPid = 0, dwVid = 0;
						WCHAR *strVid = wcsstr(var.bstrVal, L"VID_");
						if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1) dwVid = 0;
						WCHAR *strPid = wcsstr(var.bstrVal, L"PID_");
						if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1) dwPid = 0;

						// Compare the VID/PID to the DInput device
						DWORD dwVidPid = MAKELONG(dwVid, dwPid);
						if (dwVidPid == pGuidProductFromDirectInput->Data1) {
							bIsXinputDevice = true;
							goto LCleanup;
						}
					}
				}
				SAFE_RELEASE(pDevices[iDevice]);
			}
		}

	LCleanup:
		if (bstrNamespace) SysFreeString(bstrNamespace);
		if (bstrDeviceID) SysFreeString(bstrDeviceID);
		if (bstrClassName) SysFreeString(bstrClassName);
		for (iDevice = 0; iDevice < 20; iDevice++) SAFE_RELEASE(pDevices[iDevice]);
		SAFE_RELEASE(pEnumDevices);
		SAFE_RELEASE(pIWbemLocator);
		SAFE_RELEASE(pIWbemServices);

		if (bCleanupCOM) CoUninitialize();

		return bIsXinputDevice;
	}

	// TODO (DK) this should probably be called from somewhere?
	void cleanupDirectInput() {
		for (int padIndex = 0; padIndex < XUSER_MAX_COUNT; ++padIndex) {
			cleanupPad(padIndex);
		}

		if (di_instance != nullptr) {
			di_instance->Release();
			di_instance = nullptr;
		}
	}

	BOOL CALLBACK enumerateJoystickAxesCallback(LPCDIDEVICEOBJECTINSTANCEW ddoi, LPVOID context) {
		HWND hwnd = (HWND)context;

		DIPROPRANGE propertyRange;
		propertyRange.diph.dwSize = sizeof(DIPROPRANGE);
		propertyRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		propertyRange.diph.dwHow = DIPH_BYID;
		propertyRange.diph.dwObj = ddoi->dwType;
		propertyRange.lMin = -32768;
		propertyRange.lMax = 32768;

		HRESULT hr = di_pads[padCount]->SetProperty(DIPROP_RANGE, &propertyRange.diph);

		if (FAILED(hr)) {
			kinc_log(KINC_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / SetProperty() failed (HRESULT=0x%x)", padCount, hr);

			// TODO (DK) cleanup?
			// cleanupPad(padCount);

			return DIENUM_STOP;
		}

		return DIENUM_CONTINUE;
	}

	BOOL CALLBACK enumerateJoysticksCallback(LPCDIDEVICEINSTANCEW ddi, LPVOID context) {
		if (IsXInputDevice(&ddi->guidProduct)) return DIENUM_CONTINUE;

		HRESULT hr = di_instance->CreateDevice(ddi->guidInstance, &di_pads[padCount], nullptr);

		if (SUCCEEDED(hr)) {
			hr = di_pads[padCount]->SetDataFormat(&c_dfDIJoystick2);

			// TODO (DK) required?
			// hr = di_pads[padCount]->SetCooperativeLevel(NULL, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

			if (SUCCEEDED(hr)) {
				di_deviceCaps[padCount].dwSize = sizeof(DIDEVCAPS);
				hr = di_pads[padCount]->GetCapabilities(&di_deviceCaps[padCount]);

				if (SUCCEEDED(hr)) {
					hr = di_pads[padCount]->EnumObjects(enumerateJoystickAxesCallback, nullptr, DIDFT_AXIS);

					if (SUCCEEDED(hr)) {
						hr = di_pads[padCount]->Acquire();

						if (SUCCEEDED(hr)) {
							memset(&di_padState[padCount], 0, sizeof(DIJOYSTATE2));
							hr = di_pads[padCount]->GetDeviceState(sizeof(DIJOYSTATE2), &di_padState[padCount]);

							if (SUCCEEDED(hr)) {
								kinc_log(KINC_LOG_LEVEL_INFO, "DirectInput8 / Pad%i / initialized", padCount);
							}
							else {
								kinc_log(KINC_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / GetDeviceState() failed (HRESULT=0x%x)", padCount, hr);
								// cleanupPad(padCount); // (DK) don't kill it, we try again in handleDirectInputPad()
							}
						}
						else {
							kinc_log(KINC_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / Acquire() failed (HRESULT=0x%x)", padCount, hr);
							cleanupPad(padCount);
						}
					}
					else {
						kinc_log(KINC_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / EnumObjects(DIDFT_AXIS) failed (HRESULT=0x%x)", padCount, hr);
						cleanupPad(padCount);
					}
				}
				else {
					kinc_log(KINC_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / GetCapabilities() failed (HRESULT=0x%x)", padCount, hr);
					cleanupPad(padCount);
				}
			}
			else {
				kinc_log(KINC_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / SetDataFormat() failed (HRESULT=0x%x)", padCount, hr);
				cleanupPad(padCount);
			}

			++padCount;

			if (padCount >= XUSER_MAX_COUNT) {
				return DIENUM_STOP;
			}
		}

		return DIENUM_CONTINUE;
	}
}

static void initializeDirectInput() {
	HINSTANCE hinstance = GetModuleHandle(nullptr);

	memset(&di_pads, 0, sizeof(IDirectInputDevice8) * XUSER_MAX_COUNT);
	memset(&di_padState, 0, sizeof(DIJOYSTATE2) * XUSER_MAX_COUNT);
	memset(&di_lastPadState, 0, sizeof(DIJOYSTATE2) * XUSER_MAX_COUNT);
	memset(&di_deviceCaps, 0, sizeof(DIDEVCAPS) * XUSER_MAX_COUNT);

	HRESULT hr = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&di_instance, nullptr);

	if (SUCCEEDED(hr)) {
		hr = di_instance->EnumDevices(DI8DEVCLASS_GAMECTRL, enumerateJoysticksCallback, nullptr, DIEDFL_ATTACHEDONLY);

		if (SUCCEEDED(hr)) {
		}
		else {
			cleanupDirectInput();
		}
	}
	else {
		kinc_log(KINC_LOG_LEVEL_WARNING, "DirectInput8Create failed (HRESULT=0x%x)", hr);
	}
}

void handleDirectInputPad(int padIndex) {
	if (di_pads[padIndex] == nullptr) {
		return;
	}

	HRESULT hr = di_pads[padIndex]->GetDeviceState(sizeof(DIJOYSTATE2), &di_padState[padIndex]);

	switch (hr) {
	case S_OK: {
		// TODO (DK) there is a lot more to handle
		for (int axisIndex = 0; axisIndex < 2; ++axisIndex) {
			LONG *now = nullptr;
			LONG *last = nullptr;

			switch (axisIndex) {
			case 0: {
				now = &di_padState[padIndex].lX;
				last = &di_lastPadState[padIndex].lX;
			} break;
			case 1: {
				now = &di_padState[padIndex].lY;
				last = &di_lastPadState[padIndex].lY;
			} break;
			case 2: {
				now = &di_padState[padIndex].lZ;
				last = &di_lastPadState[padIndex].lZ;
			} break;
			}

			if (*now != *last) {
				kinc_internal_gamepad_trigger_axis(padIndex, axisIndex, *now / 32768.0f);
			}
		}

		for (int buttonIndex = 0; buttonIndex < 128; ++buttonIndex) {
			BYTE *now = &di_padState[padIndex].rgbButtons[buttonIndex];
			BYTE *last = &di_lastPadState[padIndex].rgbButtons[buttonIndex];

			if (*now != *last) {
				kinc_internal_gamepad_trigger_button(padIndex, buttonIndex, *now / 255.0f);
			}
		}

		memcpy(&di_lastPadState[padIndex], &di_padState[padIndex], sizeof(DIJOYSTATE2));
		break;
	}
	case DIERR_INPUTLOST: // fall through
	case DIERR_NOTACQUIRED: {
		hr = di_pads[padIndex]->Acquire();
		break;
	}
	}
}

static bool isXInputGamepad(int gamepad) {
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
	DWORD dwResult = InputGetState(gamepad, &state);
	return dwResult == ERROR_SUCCESS;
}

const char *kinc_gamepad_vendor(int gamepad) {
	if (isXInputGamepad(gamepad)) {
		return "Microsoft";
	}
	else {
		return "DirectInput8";
	}
}

const char *kinc_gamepad_product_name(int gamepad) {
	if (isXInputGamepad(gamepad)) {
		return "Xbox 360 Controller";
	}
	else {
		return "Generic Gamepad";
	}
}

bool kinc_internal_handle_messages() {
	MSG message;

	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	if (InputGetState != nullptr && (detectGamepad || gamepadFound)) {
		detectGamepad = false;
		for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));
			DWORD dwResult = InputGetState(i, &state);

			if (dwResult == ERROR_SUCCESS) {
				gamepadFound = true;

				float newaxes[6];
				newaxes[0] = state.Gamepad.sThumbLX / 32768.0f;
				newaxes[1] = state.Gamepad.sThumbLY / 32768.0f;
				newaxes[2] = state.Gamepad.sThumbRX / 32768.0f;
				newaxes[3] = state.Gamepad.sThumbRY / 32768.0f;
				newaxes[4] = state.Gamepad.bLeftTrigger / 255.0f;
				newaxes[5] = state.Gamepad.bRightTrigger / 255.0f;
				for (int i2 = 0; i2 < 6; ++i2) {
					if (axes[i * 6 + i2] != newaxes[i2]) {
						kinc_internal_gamepad_trigger_axis(i, i2, newaxes[i2]);
						axes[i * 6 + i2] = newaxes[i2];
					}
				}
				float newbuttons[16];
				newbuttons[0] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1.0f : 0.0f;
				newbuttons[1] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1.0f : 0.0f;
				newbuttons[2] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 1.0f : 0.0f;
				newbuttons[3] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 1.0f : 0.0f;
				newbuttons[4] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1.0f : 0.0f;
				newbuttons[5] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1.0f : 0.0f;
				newbuttons[6] = state.Gamepad.bLeftTrigger / 255.0f;
				newbuttons[7] = state.Gamepad.bRightTrigger / 255.0f;
				newbuttons[8] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 1.0f : 0.0f;
				newbuttons[9] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 1.0f : 0.0f;
				newbuttons[10] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1.0f : 0.0f;
				newbuttons[11] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1.0f : 0.0f;
				newbuttons[12] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1.0f : 0.0f;
				newbuttons[13] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1.0f : 0.0f;
				newbuttons[14] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1.0f : 0.0f;
				newbuttons[15] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1.0f : 0.0f;
				for (int i2 = 0; i2 < 16; ++i2) {
					if (buttons[i * 16 + i2] != newbuttons[i2]) {
						kinc_internal_gamepad_trigger_button(i, i2, newbuttons[i2]);
						buttons[i * 16 + i2] = newbuttons[i2];
					}
				}
			}
			else {
				handleDirectInputPad(i);
			}
		}
	}

	return true;
}

//**vec2i Kore::System::mousePos() {
//**	return vec2i(mouseX, mouseY);
//**}

namespace {
	bool keyboardshown = false;
}

void kinc_keyboard_show() {
	keyboardshown = true;
}

void kinc_keyboard_hide() {
	keyboardshown = false;
}

bool kinc_keyboard_active() {
	return keyboardshown;
}

void kinc_load_url(const char *url) {}

void kinc_set_keep_screen_on(bool on) {}
void kinc_vibrate(int ms) {}

const char *kinc_language() {
	return "en";
}

const char *kinc_system_id() {
	return "Windows";
}

namespace {
	wchar_t savePathw[2048] = {0};
	char savePath[2048] = {0};

	void findSavePath() {
		// CoInitialize(NULL);
		IKnownFolderManager *folders = nullptr;
		CoCreateInstance(CLSID_KnownFolderManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&folders));
		IKnownFolder *folder = nullptr;
		folders->GetFolder(FOLDERID_SavedGames, &folder);

		LPWSTR path;
		folder->GetPath(0, &path);

		wcscpy(savePathw, path);
		wcscat(savePathw, L"\\");
		wchar_t name[1024];
		MultiByteToWideChar(CP_UTF8, 0, kinc_application_name(), -1, name, 1024);
		wcscat(savePathw, name);
		wcscat(savePathw, L"\\");

		SHCreateDirectoryEx(nullptr, savePathw, nullptr);
		WideCharToMultiByte(CP_UTF8, 0, savePathw, -1, savePath, 1024, nullptr, nullptr);

		CoTaskMemFree(path);
		folder->Release();
		folders->Release();
		// CoUninitialize();
	}
}

const char *kinc_internal_save_path() {
	if (::savePath[0] == 0) findSavePath();
	return ::savePath;
}

namespace {
	const char *videoFormats[] = {"ogv", nullptr};
	LARGE_INTEGER frequency;
	LARGE_INTEGER startCount;
}

const char **kinc_video_formats() {
	return ::videoFormats;
}

void kinc_login() {}

void kinc_unlock_achievement(int id) {}

bool kinc_gamepad_connected(int num) {
	return true;
}

double kinc_frequency() {
	return (double)::frequency.QuadPart;
}

kinc_ticks_t kinc_timestamp(void) {
	LARGE_INTEGER stamp;
	QueryPerformanceCounter(&stamp);
	return stamp.QuadPart - startCount.QuadPart;
}

double kinc_time(void) {
	LARGE_INTEGER stamp;
	QueryPerformanceCounter(&stamp);
	return double(stamp.QuadPart - startCount.QuadPart) / (double)::frequency.QuadPart;
}

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine, int /*nCmdShow*/) {
	// Pen functions are only in Windows 8 and later, so load them dynamically
	HMODULE user32 = LoadLibraryA("user32.dll");
	MyGetPointerInfo = (GetPointerInfoType)GetProcAddress(user32, "GetPointerInfo");
	MyGetPointerPenInfo = (GetPointerPenInfoType)GetProcAddress(user32, "GetPointerPenInfo");
	MyEnableNonClientDpiScaling = (EnableNonClientDpiScalingType)GetProcAddress(user32, "EnableNonClientDpiScaling");
	initKeyTranslation();
	for (int i = 0; i < 256; ++i) keyPressed[i] = false;

	kinc_windows_init_displays();

	QueryPerformanceCounter(&startCount);
	QueryPerformanceFrequency(&::frequency);

	int ret = 0;
#ifndef _DEBUG
	try {
#endif
		for (int i = 0; i < 256; ++i) keyPressed[i] = false;
		ret = kickstart(__argc, __argv);

#ifndef _DEBUG
	} catch (std::exception &ex) {
		ret = 1;
		MessageBoxA(0, ex.what(), "Exception", MB_OK);
	} catch (...) {
		ret = 1;
		MessageBox(0, L"Unknown Exception", L"Exception", MB_OK);
	}
#endif

	return ret;
}

int kinc_init(const char *name, int width, int height, kinc_window_options_t *win, kinc_framebuffer_options_t *frame) {
	// Kore::System::_init(name, width, height, &win, &frame);
	kinc_set_application_name(name);
	kinc_window_options_t defaultWin;
	if (win == nullptr) {
		kinc_internal_init_window_options(&defaultWin);
		win = &defaultWin;
	}
	win->width = width;
	win->height = height;
	int window = kinc_window_create(win, frame);
	loadXInput();
	initializeDirectInput();
	return window;
}

void kinc_internal_shutdown() {
	kinc_windows_hide_windows();
	kinc_internal_shutdown_callback();
	kinc_windows_destroy_windows();
	kinc_windows_restore_displays();
}
