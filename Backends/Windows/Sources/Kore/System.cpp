#include "pch.h"
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Input/Gamepad.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Log.h>

#include "Display.h"

#include <dinput.h>

#ifdef VR_RIFT 
#include "Vr/VrInterface.h"
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shlobj.h>
#include <exception>
#include <XInput.h>

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef KOREC
extern "C"
#endif
int kore(int argc, char** argv);

namespace {
	struct KoreWindow : public Kore::KoreWindowBase {
		HWND hwnd;

		KoreWindow( HWND hwnd, int x, int y, int width, int height ) : KoreWindowBase(x, y, width, height) {
			this->hwnd = hwnd;
		}
	};

	KoreWindow* windows[Kore::System::MAXIMUM_WINDOW_COUNT] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	int windowCounter = -1;

	int idFromHWND( HWND hwnd ) {
		for (int windowIndex = 0; windowIndex < Kore::System::MAXIMUM_WINDOW_COUNT; ++windowIndex) {
			KoreWindow * window = windows[windowIndex];

			if (window != nullptr && window->hwnd == hwnd) {
				return windowIndex;
			}
		}

		return -1;
	}

#ifdef VR_RIFT
	const char* windowClassName = "ORT";
#else
	const char* windowClassName = "KoreWindow";
#endif

	void registerWindowClass(HINSTANCE hInstance, const char * className) {
		WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_OWNDC/*CS_CLASSDC*/, MsgProc, 0L, 0L, hInstance, LoadIcon(hInstance, MAKEINTRESOURCE(107)), nullptr /*LoadCursor(0, IDC_ARROW)*/, 0, 0, className/*windowClassName*/, 0 };
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		RegisterClassExA(&wc);
	}
}

// TODO (DK) split to Graphics / Windowing?
namespace Kore { namespace System {
	int currentDeviceId = -1;

	// (DK) only valid during begin() => end() calls
	int currentDevice() {
		//if (currentDeviceId == -1) {
		//	log(Warning, "no current device is active");
		//}

		return currentDeviceId;
	}

	//void setCurrentDevice(int id) {
	//	currentDeviceId = id;
	//}

	int windowCount() {
		return windowCounter + 1;
	}

	int windowWidth(int id) {
		RECT vRect;
		GetClientRect(windows[id]->hwnd, &vRect);
		int i = vRect.right;
		return i;
		//return windows[id]->width;
	}

	int windowHeight(int id) {
		RECT vRect;
		GetClientRect(windows[id]->hwnd, &vRect);
		int i = vRect.bottom;
		return i;
		//return windows[id]->height;
	}
}}

using namespace Kore;

namespace {
	int mouseX, mouseY;
	bool keyPressed[256];
	Kore::KeyCode keyTranslated[256]; //http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx

	void initKeyTranslation() {
		for (int i = 0; i < 256; ++i) keyTranslated[i] = Kore::Key_unknown;

		keyTranslated[VK_OEM_5  ] = Kore::Key_Huetchen;
		keyTranslated[VK_OEM_102] = Kore::Key_GermanLessMore;

		keyTranslated[VK_BACK] = Kore::Key_Backspace;
		keyTranslated[VK_TAB] = Kore::Key_Tab;
		//keyTranslated[VK_CLEAR]
		keyTranslated[VK_RETURN] = Kore::Key_Return;
		keyTranslated[VK_SHIFT] = Kore::Key_Shift;
		keyTranslated[VK_CONTROL] = Kore::Key_Control;
		keyTranslated[VK_MENU] = Kore::Key_Alt;
		keyTranslated[VK_PAUSE] = Kore::Key_Pause;
		keyTranslated[VK_CAPITAL] = Kore::Key_CapsLock;
		//keyTranslated[VK_KANA]
		//keyTranslated[VK_HANGUEL]
		//keyTransltaed[VK_HANGUL]
		//keyTransltaed[VK_JUNJA]
		//keyTranslated[VK_FINAL]
		//keyTranslated[VK_HANJA]
		//keyTranslated[VK_KANJI]
		keyTranslated[VK_ESCAPE] = Kore::Key_Escape;
		//keyTranslated[VK_CONVERT]
		//keyTranslated[VK_NONCONVERT
		//keyTranslated[VK_ACCEPT
		//keyTranslated[VK_MODECHANGE
		keyTranslated[VK_SPACE] = Kore::Key_Space;
		keyTranslated[VK_PRIOR] = Kore::Key_PageUp;
		keyTranslated[VK_NEXT] = Kore::Key_PageDown;
		keyTranslated[VK_END] = Kore::Key_End;
		keyTranslated[VK_HOME] = Kore::Key_Home;
		keyTranslated[VK_LEFT] = Kore::Key_Left;
		keyTranslated[VK_UP] = Kore::Key_Up;
		keyTranslated[VK_RIGHT] = Kore::Key_Right;
		keyTranslated[VK_DOWN] = Kore::Key_Down;
		//keyTranslated[VK_SELECT
		keyTranslated[VK_PRINT] = Kore::Key_Print;
		//keyTranslated[VK_EXECUTE
		//keyTranslated[VK_SNAPSHOT
		keyTranslated[VK_INSERT] = Kore::Key_Insert;
		keyTranslated[VK_DELETE] = Kore::Key_Delete;
		keyTranslated[VK_HELP] = Kore::Key_Help;
		keyTranslated[0x30] = Kore::Key_0;
		keyTranslated[0x31] = Kore::Key_1;
		keyTranslated[0x32] = Kore::Key_2;
		keyTranslated[0x33] = Kore::Key_3;
		keyTranslated[0x34] = Kore::Key_4;
		keyTranslated[0x35] = Kore::Key_5;
		keyTranslated[0x36] = Kore::Key_6;
		keyTranslated[0x37] = Kore::Key_7;
		keyTranslated[0x38] = Kore::Key_8;
		keyTranslated[0x39] = Kore::Key_9;
		keyTranslated[0x41] = Kore::Key_A;
		keyTranslated[0x42] = Kore::Key_B;
		keyTranslated[0x43] = Kore::Key_C;
		keyTranslated[0x44] = Kore::Key_D;
		keyTranslated[0x45] = Kore::Key_E;
		keyTranslated[0x46] = Kore::Key_F;
		keyTranslated[0x47] = Kore::Key_G;
		keyTranslated[0x48] = Kore::Key_H;
		keyTranslated[0x49] = Kore::Key_I;
		keyTranslated[0x4A] = Kore::Key_J;
		keyTranslated[0x4B] = Kore::Key_K;
		keyTranslated[0x4C] = Kore::Key_L;
		keyTranslated[0x4D] = Kore::Key_M;
		keyTranslated[0x4E] = Kore::Key_N;
		keyTranslated[0x4F] = Kore::Key_O;
		keyTranslated[0x50] = Kore::Key_P;
		keyTranslated[0x51] = Kore::Key_Q;
		keyTranslated[0x52] = Kore::Key_R;
		keyTranslated[0x53] = Kore::Key_S;
		keyTranslated[0x54] = Kore::Key_T;
		keyTranslated[0x55] = Kore::Key_U;
		keyTranslated[0x56] = Kore::Key_V;
		keyTranslated[0x57] = Kore::Key_W;
		keyTranslated[0x58] = Kore::Key_X;
		keyTranslated[0x59] = Kore::Key_Y;
		keyTranslated[0x5A] = Kore::Key_Z;
		//keyTranslated[VK_LWIN
		//keyTranslated[VK_RWIN
		//keyTranslated[VK_APPS
		//keyTranslated[VK_SLEEP
		//keyTranslated[VK_NUMPAD0]
		//keyTranslated[VK_NUMPAD1
		//keyTranslated[VK_NUMPAD2
		//keyTranslated[VK_NUMPAD3
		//keyTranslated[VK_NUMPAD4
		//keyTranslated[VK_NUMPAD5
		//keyTranslated[VK_NUMPAD6
		//keyTranslated[VK_NUMPAD7
		//keyTranslated[VK_NUMPAD8
		//keyTranslated[VK_NUMPAD9
		keyTranslated[VK_MULTIPLY] = Kore::Key_multiply;
		//keyTranslated[VK_ADD]
		//keyTranslated[VK_SEPARATOR
		//keyTranslated[VK_SUBTRACT
		//keyTranslated[VK_DECIMAL
		//keyTranslated[VK_DIVIDE
		keyTranslated[VK_F1] = Kore::Key_F1;
		keyTranslated[VK_F2] = Kore::Key_F2;
		keyTranslated[VK_F3] = Kore::Key_F3;
		keyTranslated[VK_F4] = Kore::Key_F4;
		keyTranslated[VK_F5] = Kore::Key_F5;
		keyTranslated[VK_F6] = Kore::Key_F6;
		keyTranslated[VK_F7] = Kore::Key_F7;
		keyTranslated[VK_F8] = Kore::Key_F8;
		keyTranslated[VK_F9] = Kore::Key_F9;
		keyTranslated[VK_F10] = Kore::Key_F10;
		keyTranslated[VK_F11] = Kore::Key_F11;
		keyTranslated[VK_F12] = Kore::Key_F12;
		//keyTranslated[VK_F13
		//keyTranslated[VK_F14
		//keyTranslated[VK_F15
		//keyTranslated[VK_F16
		//keyTranslated[VK_F17
		//keyTranslated[VK_F18
		//keyTranslated[VK_F19
		//keyTranslated[VK_F20
		//keyTranslated[VK_F21
		//keyTranslated[VK_F22
		//keyTranslated[VK_F23
		//keyTranslated[VK_F24
		keyTranslated[VK_NUMLOCK] = Kore::Key_NumLock;
		keyTranslated[VK_SCROLL] = Kore::Key_ScrollLock;
		//0x92-96 //OEM specific
		//keyTranslated[VK_LSHIFT]
		//keyTranslated[VK_RSHIFT
		//keyTranslated[VK_LCONTROL]
		//keyTranslated[VK_RCONTROL
		//keyTranslated[VK_LMENU
		//keyTranslated[VK_RMENU
		//keyTranslated[VK_BROWSER_BACK
		//keyTranslated[VK_BROWSER_FORWARD
		//keyTranslated[VK_BROWSER_REFRESH
		//keyTranslated[VK_BROWSER_STOP
		//keyTranslated[VK_BROWSER_SEARCH
		//keyTranslated[VK_BROWSER_FAVORITES
		//keyTranslated[VK_BROWSER_HOME
		//keyTranslated[VK_VOLUME_MUTE
		//keyTranslated[VK_VOLUME_DOWN
		//keyTranslated[VK_VOLUME_UP
		//keyTranslated[VK_MEDIA_NEXT_TRACK
		//keyTranslated[VK_MEDIA_PREV_TRACK
		//keyTranslated[VK_MEDIA_STOP
		//keyTranslated[VK_MEDIA_PLAY_PAUSE
		//keyTranslated[VK_LAUNCH_MAIL
		//keyTranslated[VK_LAUNCH_MEDIA_SELECT
		//keyTranslated[VK_LAUNCH_APP1
		//keyTranslated[VK_LAUNCH_APP2
		//keyTranslated[VK_OEM_1 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key
		keyTranslated[VK_OEM_PLUS] = Kore::Key_Plus;
		keyTranslated[VK_OEM_COMMA] = Kore::Key_Comma;
		keyTranslated[VK_OEM_MINUS] = Kore::Key_Minus;
		keyTranslated[VK_OEM_PERIOD] = Kore::Key_Period;
		//keyTranslated[VK_OEM_2 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
		//keyTranslated[VK_OEM_3 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
		//keyTranslated[VK_OEM_4 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
		//keyTranslated[VK_OEM_5 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
		//keyTranslated[VK_OEM_6 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
		//keyTranslated[VK_OEM_7 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote' key
		//keyTranslated[VK_OEM_8 //Used for miscellaneous characters; it can vary by keyboard.
		//keyTranslated[0xE1 //OEM specific
		//keyTranslated[VK_OEM_102 //Either the angle bracket key or the backslash key on the RT 102-key keyboard
		//0xE3-E4 //OEM specific
		//keyTranslated[VK_PROCESSKEY
		//0xE6 //OEM specific
		//keyTranslated[VK_PACKET //Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods.
		//0xE9-F5 //OEM specific
		//keyTranslated[VK_ATTN
		//keyTranslated[VK_CRSEL
		//keyTranslated[VK_EXSEL
		//keyTranslated[VK_EREOF
		//keyTranslated[VK_PLAY
		//keyTranslated[VK_ZOOM
		//keyTranslated[VK_NONAME
		//keyTranslated[VK_PA1
		//keyTranslated[PA1 key
		//keyTranslated[VK_OEM_CLEAR
	}

	uint r = 0;

	wchar_t toUnicode(WPARAM wParam, LPARAM lParam) {
		wchar_t buffer[11];
		BYTE state[256];
		GetKeyboardState(state);
		ToUnicode(wParam, (lParam >> 8) & 0xFFFFFF00, state, buffer, 10, 0);
		return buffer[0];
	}
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int windowWidth;
	int windowHeight;
	
	switch (msg) {
	case WM_MOVE:
	case WM_MOVING:
	case WM_SIZING:
		//Scheduler::breakTime();
		break;
	case WM_SIZE:
		windowWidth = LOWORD(lParam);
		windowHeight = HIWORD(lParam);
		Graphics::changeResolution(windowWidth, windowHeight);
	break;
	case WM_DESTROY:
		Kore::System::stop();
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_ACTIVE)
			Mouse::the()->_activated(idFromHWND(hWnd), true);
		else
			Mouse::the()->_activated(idFromHWND(hWnd), false);
		break;
	case WM_MOUSEMOVE:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		Mouse::the()->_move(idFromHWND(hWnd), LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONDOWN:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		Mouse::the()->_press(idFromHWND(hWnd), 0, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		Mouse::the()->_release(idFromHWND(hWnd), 0, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_RBUTTONDOWN:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		Mouse::the()->_press(idFromHWND(hWnd), 1, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_RBUTTONUP:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		Mouse::the()->_release(idFromHWND(hWnd), 1, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MBUTTONDOWN:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		Mouse::the()->_press(idFromHWND(hWnd), 2, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MBUTTONUP:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		Mouse::the()->_release(idFromHWND(hWnd), 2, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_XBUTTONDOWN:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		Mouse::the()->_press(idFromHWND(hWnd), HIWORD(wParam) + 2, mouseX, mouseY);
		break;	
	case WM_XBUTTONUP:
		mouseX = LOWORD(lParam);
		mouseY = HIWORD(lParam);
		Mouse::the()->_release(idFromHWND(hWnd), HIWORD(wParam) + 2, mouseX, mouseY);
		break;
	case WM_MOUSEWHEEL:
		Mouse::the()->_scroll(idFromHWND(hWnd), GET_WHEEL_DELTA_WPARAM(wParam) / -120);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (!keyPressed[wParam]) {
			keyPressed[wParam] = true;
			Keyboard::the()->_keydown(keyTranslated[wParam], toUnicode(wParam, lParam));
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		keyPressed[wParam] = false;
		Keyboard::the()->_keyup(keyTranslated[wParam], toUnicode(wParam, lParam));
		break;
	case WM_SYSCOMMAND:
		//printf("WS_SYSCOMMAND %5d %5d %5d\n", msg, wParam, lParam);
		switch (wParam) {
		case SC_KEYMENU: // Ignorieren, wenn Alt f�r das WS_SYSMENU gedr�ckt wird.
			return 0;
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0; // Prevent From Happening

		// Pause game when window is minimized, continue when it's restored or maximized.
		//
		// Unfortunately, if the game would continue to run when minimized, the graphics in
		// the Windows Vista/7 taskbar would not be updated, even when Direct3DDevice::Present()
		// is called without error. I do not know why.
		case SC_MINIMIZE:
			//Scheduler::haltTime(); // haltTime()/unhaltTime() is incremental, meaning that this doesn't interfere with when the game itself calls these functions
			break;
		case SC_RESTORE:
		case SC_MAXIMIZE:
			//Scheduler::unhaltTime();
			break;
		}
		break;
	}
	return DefWindowProcA(hWnd, msg, wParam, lParam);
}

namespace {
	float axes[12 * 6];
	float buttons[12 * 16];

	typedef DWORD (WINAPI *XInputGetStateType)(DWORD dwUserIndex, XINPUT_STATE* pState);
	XInputGetStateType InputGetState = nullptr;

	void loadXInput() {
		HMODULE lib = LoadLibraryA("xinput1_4.dll");
		if (lib == nullptr) {
			lib = LoadLibraryA("xinput1_3.dll");
		}
		if (lib == nullptr) {
			lib = LoadLibraryA("xinput9_1_0.dll");
		}
		
		if (lib != nullptr) {
			InputGetState = (XInputGetStateType)GetProcAddress(lib, "XInputGetState");
		}
	}

	IDirectInput8 * di_instance = nullptr;
	IDirectInputDevice8 * di_pads[XUSER_MAX_COUNT];
	DIJOYSTATE2 di_padState[XUSER_MAX_COUNT];
	DIJOYSTATE2 di_lastPadState[XUSER_MAX_COUNT];
	DIDEVCAPS di_deviceCaps[XUSER_MAX_COUNT];
	int padCount = 0;

	void cleanupPad( int padIndex ) {
		if (di_pads[padIndex] != nullptr) {
			di_pads[padIndex]->Unacquire();
			di_pads[padIndex]->Release();
			di_pads[padIndex] = 0;
		}
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

	BOOL CALLBACK enumerateJoystickAxesCallback( LPCDIDEVICEOBJECTINSTANCEW ddoi, LPVOID context ) {
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
			log(Warning, "DirectInput8 / Pad%i / SetProperty() failed (HRESULT=0x%x)", padCount, hr);

			// TODO (DK) cleanup?
			//cleanupPad(padCount);

			return DIENUM_STOP;
		}

		return DIENUM_CONTINUE;
	}

	BOOL CALLBACK enumerateJoysticksCallback( LPCDIDEVICEINSTANCEW ddi, LPVOID context ) {
		HRESULT hr = di_instance->CreateDevice(ddi->guidInstance, &di_pads[padCount], nullptr);

		if (SUCCEEDED(hr)) {
			hr = di_pads[padCount]->SetDataFormat(&c_dfDIJoystick2);

			// TODO (DK) required?
			//hr = di_pads[padCount]->SetCooperativeLevel(NULL, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

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

							switch (hr) {
								case S_OK: log(Info, "DirectInput8 / Pad%i / initialized", padCount); break;
								default: {
									log(Warning, "DirectInput8 / Pad%i / GetDeviceState() failed (HRESULT=0x%x)", padCount, hr);
									//cleanupPad(padCount); // (DK) don't kill it, we try again in handleDirectInputPad()
								} break;
							}
						} else {
							log(Warning, "DirectInput8 / Pad%i / Acquire() failed (HRESULT=0x%x)", padCount, hr);
							cleanupPad(padCount);
						}
					} else {
						log(Warning, "DirectInput8 / Pad%i / EnumObjects(DIDFT_AXIS) failed (HRESULT=0x%x)", padCount, hr);
						cleanupPad(padCount);
					}
				} else {
					log(Warning, "DirectInput8 / Pad%i / GetCapabilities() failed (HRESULT=0x%x)", padCount, hr);
					cleanupPad(padCount);
				}
			} else {				
				log(Warning, "DirectInput8 / Pad%i / SetDataFormat() failed (HRESULT=0x%x)", padCount, hr);
				cleanupPad(padCount);
			}

			++padCount;

			if (padCount >= XUSER_MAX_COUNT) {
				return DIENUM_STOP;
			}
		}

		return DIENUM_CONTINUE;
	}

	void initializeDirectInput() {
		HINSTANCE hinstance = GetModuleHandleA(nullptr);
		
		memset(&di_pads, 0, sizeof(IDirectInputDevice8) * XUSER_MAX_COUNT);
		memset(&di_padState, 0, sizeof(DIJOYSTATE2) * XUSER_MAX_COUNT);
		memset(&di_lastPadState, 0, sizeof(DIJOYSTATE2) * XUSER_MAX_COUNT);
		memset(&di_deviceCaps, 0, sizeof(DIDEVCAPS) * XUSER_MAX_COUNT);

		HRESULT hr = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&di_instance, nullptr);

		if (SUCCEEDED(hr)) {
			hr = di_instance->EnumDevices(DI8DEVCLASS_GAMECTRL, enumerateJoysticksCallback, nullptr, DIEDFL_ATTACHEDONLY);

			if (SUCCEEDED(hr)) {
			} else {
				cleanupDirectInput();
			}
		} else {
			log(Warning, "DirectInput8Create failed (HRESULT=0x%x)", hr);
		}
	}

	void handleDirectInputPad( int padIndex ) {
		if (di_pads[padIndex] == nullptr) {
			return;
		}

		// TODO (DK) code is copied from xinput stuff, why is it set every frame?
		Kore::Gamepad::get(padIndex)->vendor = "DirectInput8"; // TODO (DK) figure out how to get vendor name
		Kore::Gamepad::get(padIndex)->productName = "Generic Gamepad"; // TODO (DK) figure out how to get product name
				
		HRESULT hr = di_pads[padIndex]->GetDeviceState(sizeof(DIJOYSTATE2), &di_padState[padIndex]);

		switch (hr) {
			case S_OK: {
				if (Kore::Gamepad::get(padIndex)->Axis != nullptr) {
					// TODO (DK) there is a lot more to handle
					for (int axisIndex = 0; axisIndex < 2; ++axisIndex) {
						LONG * now = nullptr;
						LONG * last = nullptr;

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
							Kore::Gamepad::get(padIndex)->Axis(axisIndex, *now / 32768.0f);
						}
					}

					if (Kore::Gamepad::get(padIndex)->Button != nullptr) {
						for (int buttonIndex = 0; buttonIndex < 128; ++buttonIndex) {
							BYTE * now = &di_padState[padIndex].rgbButtons[buttonIndex];
							BYTE * last = &di_lastPadState[padIndex].rgbButtons[buttonIndex];

							if (*now != *last) {
								Kore::Gamepad::get(padIndex)->Button(buttonIndex, *now / 255.0f);
							}
						}
					}
				}

				memcpy(&di_lastPadState[padIndex], &di_padState[padIndex], sizeof(DIJOYSTATE2));
			} break;
			case DIERR_INPUTLOST: // fall through
			case DIERR_NOTACQUIRED: {
				hr = di_pads[padIndex]->Acquire();
			} break;
		}
	}
}

bool Kore::System::handleMessages() {
	static MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		//TranslateMessage(&message);
		DispatchMessage(&message);
	}

	if (InputGetState != nullptr) {
		DWORD dwResult;
		for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));
			dwResult = InputGetState(i, &state);

			if (dwResult == ERROR_SUCCESS) {
				Kore::Gamepad::get(i)->vendor = "Microsoft";
				Kore::Gamepad::get(i)->productName = "Xbox 360 Controller";

				float newaxes[6];
				newaxes[0] = state.Gamepad.sThumbLX / 32768.0f;
				newaxes[1] = state.Gamepad.sThumbLY / 32768.0f;
				newaxes[2] = state.Gamepad.sThumbRX / 32768.0f;
				newaxes[3] = state.Gamepad.sThumbRY / 32768.0f;
				newaxes[4] = state.Gamepad.bLeftTrigger / 255.0f;
				newaxes[5] = state.Gamepad.bRightTrigger / 255.0f;
				for (int i2 = 0; i2 < 6; ++i2) {
					if (axes[i * 6 + i2] != newaxes[i2]) {
						if (Kore::Gamepad::get(i)->Axis != nullptr) Kore::Gamepad::get(i)->Axis(i2, newaxes[i2]);
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
						if (Kore::Gamepad::get(i)->Button != nullptr) Kore::Gamepad::get(i)->Button(i2, newbuttons[i2]);
						buttons[i * 16 + i2] = newbuttons[i2];
					}
				}
			}

			handleDirectInputPad(i);
		}
	}

	return true;
}

vec2i Kore::System::mousePos() {
	return vec2i(mouseX, mouseY);
}

#undef CreateWindow

int createWindow( const char * title, int x, int y, int width, int height, WindowMode windowMode, int targetDisplay ) {
	++windowCounter;

	HINSTANCE inst = GetModuleHandleA(nullptr);
#ifdef VR_RIFT 
		::registerWindowClass(inst);
		::windows[0] = new W32KoreWindow((HWND) VrInterface::Init(inst));
#else /* #ifdef VR_RIFT  */

	if (windowCounter == 0) {
		::registerWindowClass(inst, windowClassName);
	}
	
	DWORD dwExStyle;
	DWORD dwStyle;

	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.right = width;
	WindowRect.top = 0;
	WindowRect.bottom = height;
	
	switch (windowMode) {
		default: // fall through
		case WindowModeWindow: {
			dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		} break;

		case WindowModeBorderless: {
			dwStyle = WS_POPUP;
			dwExStyle = WS_EX_APPWINDOW;
		} break;

		case WindowModeFullscreen: {
			DEVMODEA dmScreenSettings;									// Device Mode
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));		// Makes Sure Memory's Cleared
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);			// Size Of The Devmode Structure
			dmScreenSettings.dmPelsWidth	= width;					// Selected Screen Width
			dmScreenSettings.dmPelsHeight	= height;					// Selected Screen Height
			dmScreenSettings.dmBitsPerPel	= 32;						// Selected Bits Per Pixel
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
			if (ChangeDisplaySettingsA(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
				//return FALSE;
			}
		
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP;
			ShowCursor(FALSE);
		} break;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	const Kore::Display::DeviceInfo * displayDevice = targetDisplay < 0
		? Kore::Display::primary()
		: Kore::Display::byId(targetDisplay)
		;

	int dstx = displayDevice->x;
	int dsty = displayDevice->y;
	uint xres = GetSystemMetrics(SM_CXSCREEN);
	uint yres = GetSystemMetrics(SM_CYSCREEN);
	uint w = width;
	uint h = height;

	switch (windowMode) {
		default: // fall through
		case WindowModeWindow: // fall through
		case WindowModeBorderless: {
			dstx += x < 0 ? (displayDevice->width - w) >> 1 : x;
			dsty += y < 0 ? (displayDevice->height - h) >> 1 : y;
		} break;

		case WindowModeFullscreen: {
			//dstx = 0;
			//dsty = 0;
		} break;
	}

	HWND hwnd = CreateWindowExA(dwExStyle, windowClassName, title, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle, dstx, dsty, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, nullptr, nullptr, inst, nullptr);
	
	if (windowCounter == 0) {
		if (windowMode == WindowModeFullscreen) {
			SetWindowPos(hwnd, nullptr, dstx, dsty, width, height, 0);
		}
	}

	GetFocus(); // TODO (DK) that seems like a useless call, as the return value isn't saved anywhere?
	::SetCursor(LoadCursor(0, IDC_ARROW));

	if (windowCounter == 0) {
		loadXInput();
		initializeDirectInput();
	}
#endif /*#else // #ifdef VR_RIFT  */

	windows[windowCounter] = new KoreWindow(hwnd, dstx, dsty, width, height);
	return windowCounter;
}

void* Kore::System::windowHandle(int windowId) {
	return windows[windowId]->hwnd;
}

void Kore::System::destroyWindow( int index ) {
	HWND hwnd = windows[index]->hwnd;

	// TODO (DK) shouldn't 'hwnd = nullptr' moved out of here?
	if (hwnd && !DestroyWindow(hwnd)) {
		//MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hwnd = nullptr;
	}

	windows[index] = nullptr;
	
	// TODO (DK) only unregister after the last window is destroyed?
	if (!UnregisterClassA(windowClassName, GetModuleHandleA(nullptr))) {
		//MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		//hInstance=NULL;
	}
}

void Kore::System::makeCurrent(int contextId) {
	if (currentDeviceId == contextId) {
		return;
	}

	currentDeviceId = contextId;
	Graphics::makeCurrent(contextId);
}

void Kore::System::clearCurrent() {
	currentDeviceId = -1;
	Graphics::clearCurrent();
}

int Kore::System::initWindow( WindowOptions options ) {
	char buffer[1024] = {0};
	strcat(buffer, name());
	
	if (options.title != nullptr) {
		strcat(buffer, options.title);
	}

	int windowId = createWindow(buffer, options.x, options.y, options.width, options.height, options.mode, options.targetDisplay);
	
	HWND hwnd = (HWND)windowHandle(windowId);
	long style = GetWindowLong(hwnd, GWL_STYLE);
	
	if(options.resizable){
		style |= WS_SIZEBOX;
	}
	
	if(options.maximizable){
		style |= WS_MAXIMIZEBOX;
	}
	
	if(!options.minimizable){
		style ^= WS_MINIMIZEBOX;
	}
	
	SetWindowLong(hwnd, GWL_STYLE, style);
	
	Graphics::setAntialiasingSamples(options.rendererOptions.antialiasing);
	Graphics::init(windowId, options.rendererOptions.depthBufferBits, options.rendererOptions.stencilBufferBits);
	
	return windowId;
}

void Kore::System::changeResolution(int width, int height, bool fullscreen) {
	
#pragma message ("TODO (DK) implement changeResolution(w,h,fs) for d3dX")
    
#if !defined(OPENGL) && !defined(SYS_VULKAN)
	/*Application::the()->setWidth(width);
	Application::the()->setHeight(height);
	Application::the()->setFullscreen(fullscreen);

	if (!Application::the()->fullscreen()) {
		uint yres = GetSystemMetrics(SM_CYSCREEN);

		// Fenster rechts von Textkonsole positionieren
		RECT r;
		r.left   = 8 * 80 + 44;
		r.top    = 0;
		r.right  = r.left + Application::the()->width() - 1;
		r.bottom = r.top + Application::the()->height() - 1;
		uint h = r.bottom - r.top + 1;
		DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		AdjustWindowRect(&r, dwStyle, FALSE); // Rahmen usw. miteinberechnen
		MoveWindow(hwnd, r.left, (yres - h) >> 1, r.right - r.left + 1, r.bottom - r.top + 1, TRUE);

		Graphics::changeResolution(width, height);
	}
	*/
#endif
}

void Kore::System::setup() {
    Display::enumerate();
	Graphics::setup();
}

bool Kore::System::isFullscreen() {
    // TODO (DK)
    return false;
}

namespace {
	bool keyboardshown = false;
}

void Kore::System::showKeyboard() {
	keyboardshown = true;
}

void Kore::System::hideKeyboard() {
	keyboardshown = false;
}

bool Kore::System::showsKeyboard() {
	return keyboardshown;
}

void Kore::System::loadURL(const char* url) {    
}

void Kore::System::setTitle(const char* title) {
	SetWindowTextA(windows[currentDevice()]->hwnd, title);
}

void Kore::System::setKeepScreenOn( bool on ) {
    
}

// TODO (DK) windowId
void Kore::System::showWindow() {
	ShowWindow(windows[0]->hwnd, SW_SHOWDEFAULT);
	UpdateWindow(windows[0]->hwnd);
}

int Kore::System::desktopWidth() {
	RECT size;
	const HWND desktop = GetDesktopWindow();
	GetWindowRect(desktop, &size);
	return size.right;
}

int Kore::System::desktopHeight() {
	RECT size;
	const HWND desktop = GetDesktopWindow();
	GetWindowRect(desktop, &size);
	return size.bottom;
}

const char* Kore::System::systemId() {
	return "Windows";
}

namespace {
	char* savePath = nullptr;

	void getSavePath() {
		//CoInitialize(NULL);
		IKnownFolderManager* folders = nullptr;
		CoCreateInstance(CLSID_KnownFolderManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&folders));
		IKnownFolder* folder = nullptr;
		folders->GetFolder(FOLDERID_SavedGames, &folder);

		LPWSTR path;
		folder->GetPath(0, &path);

		size_t length = wcslen(path);
		size_t length2 = strlen(Kore::System::name());
		savePath = new char[length + length2 + 3];
		for (size_t i = 0; i < length; ++i) {
			savePath[i] = static_cast<char>(path[i]);
		}
		savePath[length] = '\\';
		for (size_t i = 0; i < length2; ++i) {
			savePath[length + 1 + i] = Kore::System::name()[i];
		}
		savePath[length + 1 + length2] = '\\';
		savePath[length + 1 + length2 + 1] = 0;

		SHCreateDirectoryExA(nullptr, savePath, nullptr);

		CoTaskMemFree(path);
		folder->Release();
		folders->Release();
		//CoUninitialize();
	}
}

const char* Kore::System::savePath() {
	if (::savePath == nullptr) getSavePath();
	return ::savePath;
}

namespace {
	const char* videoFormats[] = { "ogv", nullptr };
}

const char** Kore::System::videoFormats() {
	return ::videoFormats;
}

double Kore::System::frequency() {
	ticks rate;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&rate));
	return (double)rate;
}

Kore::System::ticks Kore::System::timestamp() {
	ticks stamp;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stamp));
	return stamp;
}

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine, int /*nCmdShow*/) {
	initKeyTranslation();
	for (int i = 0; i < 256; ++i) keyPressed[i] = false;

	int argc = 1; 
	for (unsigned i = 0; i < strlen(lpCmdLine); ++i) { 
		while (lpCmdLine[i] == ' ' && i < strlen(lpCmdLine)) ++i;
		if (lpCmdLine[i] == '\"') {
			++i;
			while (lpCmdLine[i] != '\"' && i < strlen(lpCmdLine)) ++i;
			++argc;
		} 
		else {
			while (lpCmdLine[i] != ' ' && i < strlen(lpCmdLine)) ++i;
			++argc;
		}
	}

	char** argv = (char**)malloc(sizeof(char*) * (argc + 1)); 

	argv[0] = (char*)malloc(1024);
	GetModuleFileNameA(0, argv[0], 1024);

	for (int i = 1; i < argc; ++i) argv[i] = (char*)malloc(strlen(lpCmdLine) + 10);
	argv[argc] = 0;

	argc = 1;
	int pos = 0;
	for (unsigned i = 0; i < strlen(lpCmdLine); ++i) {
		while (lpCmdLine[i] == ' ' && i < strlen(lpCmdLine)) ++i;
		if (lpCmdLine[i] == '\"') {
			++i;
			while (lpCmdLine[i] != '\"' && i < strlen(lpCmdLine)) {
				argv[argc][pos] = lpCmdLine[i];
				++i;
				++pos;
			}
			argv[argc][pos] = '\0';
			++argc;
			pos = 0;
		}
		else {
			while (lpCmdLine[i] != ' ' && i < strlen(lpCmdLine)) {
				argv[argc][pos] = lpCmdLine[i];
				++i;
				++pos;
			}
			argv[argc][pos] = '\0';
			++argc;
			pos = 0;
		}
	}
	argv[argc] = 0; 

	int ret = 0;
#ifndef _DEBUG
	try {
#endif
		for (int i = 0; i < 256; ++i) keyPressed[i] = false;

		ret = kore(argc, argv);
#ifndef _DEBUG
	}
	catch (std::exception& ex) {
		ret = 1;
		MessageBoxA(0, ex.what(), "Exception", MB_OK);
	}
	catch (...) {
		ret = 1;
		MessageBox(0, L"Unknown Exception", L"Exception", MB_OK);
	}
#endif
	
	for(int i = 0; i < argc; ++i) free(argv[i]);
	free(argv); 

	return ret;
}
