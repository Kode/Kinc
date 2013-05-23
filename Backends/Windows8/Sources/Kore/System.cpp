#include "stdafx.h"
#include <Kt/System.h>
#include <Kt/Application.h>
#include <Kt/Input/KeyEvent.h>
#include <Kt/Scene.h>
#include <Kt/Scheduler.h>
#include <Kt/Input/Keyboard.h>
#include <Kt/Input/Mouse.h>
#include <Kt/Exception.h>
#include <Kt/WinException.h>
#include <Kt/Direct3D11.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

ref class Win8Application sealed : public Windows::ApplicationModel::Core::IFrameworkView {
public:
	Win8Application();
	virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
	virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
	virtual void Load(Platform::String^ entryPoint);
	virtual void Run();
	virtual void Uninitialize();
protected:
	void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
	void OnLogicalDpiChanged(Platform::Object^ sender);
	void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
	void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
	void OnResuming(Platform::Object^ sender, Platform::Object^ args);
	void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);
	void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
	void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
	void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
private:
	//CubeRenderer^ m_renderer;
	//Windows::ApplicationModel::Core::CoreApplicationView^ view;
	bool closed;
};

ref class Win8ApplicationSource : Windows::ApplicationModel::Core::IFrameworkViewSource {
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};

using namespace Kt;

namespace {
	int mouseX, mouseY;
	bool keyPressed[256];
	Kt::KeyCode keyTranslated[256]; //http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx

	void initKeyTranslation() {
		for (int i = 0; i < 256; ++i) keyTranslated[i] = Kt::Key_unknown;
		keyTranslated[VK_BACK] = Kt::Key_Backspace;
		keyTranslated[VK_TAB] = Kt::Key_Tab;
		//keyTranslated[VK_CLEAR]
		keyTranslated[VK_RETURN] = Kt::Key_Return;
		keyTranslated[VK_SHIFT] = Kt::Key_Shift;
		keyTranslated[VK_CONTROL] = Kt::Key_Control;
		keyTranslated[VK_MENU] = Kt::Key_Alt;
		keyTranslated[VK_PAUSE] = Kt::Key_Pause;
		keyTranslated[VK_CAPITAL] = Kt::Key_CapsLock;
		//keyTranslated[VK_KANA]
		//keyTranslated[VK_HANGUEL]
		//keyTransltaed[VK_HANGUL]
		//keyTransltaed[VK_JUNJA]
		//keyTranslated[VK_FINAL]
		//keyTranslated[VK_HANJA]
		//keyTranslated[VK_KANJI]
		keyTranslated[VK_ESCAPE] = Kt::Key_Escape;
		//keyTranslated[VK_CONVERT]
		//keyTranslated[VK_NONCONVERT
		//keyTranslated[VK_ACCEPT
		//keyTranslated[VK_MODECHANGE
		keyTranslated[VK_SPACE] = Kt::Key_Space;
		keyTranslated[VK_PRIOR] = Kt::Key_PageUp;
		keyTranslated[VK_NEXT] = Kt::Key_PageDown;
		keyTranslated[VK_END] = Kt::Key_End;
		keyTranslated[VK_HOME] = Kt::Key_Home;
		keyTranslated[VK_LEFT] = Kt::Key_Left;
		keyTranslated[VK_UP] = Kt::Key_Up;
		keyTranslated[VK_RIGHT] = Kt::Key_Right;
		keyTranslated[VK_DOWN] = Kt::Key_Down;
		//keyTranslated[VK_SELECT
		keyTranslated[VK_PRINT] = Kt::Key_Print;
		//keyTranslated[VK_EXECUTE
		//keyTranslated[VK_SNAPSHOT
		keyTranslated[VK_INSERT] = Kt::Key_Insert;
		keyTranslated[VK_DELETE] = Kt::Key_Delete;
		keyTranslated[VK_HELP] = Kt::Key_Help;
		keyTranslated[0x30] = Kt::Key_0;
		keyTranslated[0x31] = Kt::Key_1;
		keyTranslated[0x32] = Kt::Key_2;
		keyTranslated[0x33] = Kt::Key_3;
		keyTranslated[0x34] = Kt::Key_4;
		keyTranslated[0x35] = Kt::Key_5;
		keyTranslated[0x36] = Kt::Key_6;
		keyTranslated[0x37] = Kt::Key_7;
		keyTranslated[0x38] = Kt::Key_8;
		keyTranslated[0x39] = Kt::Key_9;
		keyTranslated[0x41] = Kt::Key_A;
		keyTranslated[0x42] = Kt::Key_B;
		keyTranslated[0x43] = Kt::Key_C;
		keyTranslated[0x44] = Kt::Key_D;
		keyTranslated[0x45] = Kt::Key_E;
		keyTranslated[0x46] = Kt::Key_F;
		keyTranslated[0x47] = Kt::Key_G;
		keyTranslated[0x48] = Kt::Key_H;
		keyTranslated[0x49] = Kt::Key_I;
		keyTranslated[0x4A] = Kt::Key_J;
		keyTranslated[0x4B] = Kt::Key_K;
		keyTranslated[0x4C] = Kt::Key_L;
		keyTranslated[0x4D] = Kt::Key_M;
		keyTranslated[0x4E] = Kt::Key_N;
		keyTranslated[0x4F] = Kt::Key_O;
		keyTranslated[0x50] = Kt::Key_P;
		keyTranslated[0x51] = Kt::Key_Q;
		keyTranslated[0x52] = Kt::Key_R;
		keyTranslated[0x53] = Kt::Key_S;
		keyTranslated[0x54] = Kt::Key_T;
		keyTranslated[0x55] = Kt::Key_U;
		keyTranslated[0x56] = Kt::Key_V;
		keyTranslated[0x57] = Kt::Key_W;
		keyTranslated[0x58] = Kt::Key_X;
		keyTranslated[0x59] = Kt::Key_Y;
		keyTranslated[0x5A] = Kt::Key_Z;
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
		keyTranslated[VK_MULTIPLY] = Kt::Key_multiply;
		//keyTranslated[VK_ADD]
		//keyTranslated[VK_SEPARATOR
		//keyTranslated[VK_SUBTRACT
		//keyTranslated[VK_DECIMAL
		//keyTranslated[VK_DIVIDE
		keyTranslated[VK_F1] = Kt::Key_F1;
		keyTranslated[VK_F2] = Kt::Key_F2;
		keyTranslated[VK_F3] = Kt::Key_F3;
		keyTranslated[VK_F4] = Kt::Key_F4;
		keyTranslated[VK_F5] = Kt::Key_F5;
		keyTranslated[VK_F6] = Kt::Key_F6;
		keyTranslated[VK_F7] = Kt::Key_F7;
		keyTranslated[VK_F8] = Kt::Key_F8;
		keyTranslated[VK_F9] = Kt::Key_F9;
		keyTranslated[VK_F10] = Kt::Key_F10;
		keyTranslated[VK_F11] = Kt::Key_F11;
		keyTranslated[VK_F12] = Kt::Key_F12;
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
		keyTranslated[VK_NUMLOCK] = Kt::Key_NumLock;
		keyTranslated[VK_SCROLL] = Kt::Key_ScrollLock;
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
		keyTranslated[VK_OEM_PLUS] = Kt::Key_Plus;
		keyTranslated[VK_OEM_COMMA] = Kt::Key_Comma;
		keyTranslated[VK_OEM_MINUS] = Kt::Key_Minus;
		keyTranslated[VK_OEM_PERIOD] = Kt::Key_Period;
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
}

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

namespace Kt {
	extern void switchDevMode();
}

void Kt::System::HandleMessages() {
	CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
}

Kt::Vector2i Kt::System::mousePos() {
	return Vector2i(mouseX, mouseY);
}

void Kt::System::SwitchBuffers() {
	
}

#undef CreateWindow

void* Kt::System::CreateWindow() {
	Scheduler::addFrameTask(HandleMessages, 1001);

	return nullptr;
}

void* Kt::System::windowHandle() {
	return nullptr;
}

void Kt::System::DestroyWindow() {
	
}

void Kt::System::ChangeResolution(int width, int height, bool fullscreen) {

}

void Kt::System::setTitle(const char*) {

}

void Kt::System::showWindow() {

}

namespace {
	bool keyboardshown = false;
}

void Kt::System::showKeyboard() {
	keyboardshown = true;
}

void Kt::System::hideKeyboard() {
	keyboardshown = false;
}

bool Kt::System::showsKeyboard() {
	return keyboardshown;
}

int ktmain(const Kt::List<Kt::Text>& params);

Win8Application::Win8Application() : closed(false) {

}

void Win8Application::Initialize(CoreApplicationView^ applicationView) {
	applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &Win8Application::OnActivated);
	CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &Win8Application::OnSuspending);
	CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &Win8Application::OnResuming);
	//m_renderer = ref new CubeRenderer();
}

void Win8Application::SetWindow(CoreWindow^ window) {
	window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &Win8Application::OnWindowSizeChanged);
	window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &Win8Application::OnWindowClosed);
	window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &Win8Application::OnPointerPressed);
	window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &Win8Application::OnPointerReleased);
	window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &Win8Application::OnPointerMoved);
	//m_renderer->Initialize(CoreWindow::GetForCurrentThread());
}

void Win8Application::Load(Platform::String^ entryPoint) {

}

void Win8Application::Run() {
	//BasicTimer^ timer = ref new BasicTimer();
	Kt::List<Kt::Text> params;
	ktmain(params);
	//while (!closed) {
		//timer->Update();
		
		//m_renderer->Update(timer->Total, timer->Delta);
		//m_renderer->Render();
		//m_renderer->Present(); // This call is synchronized to the display frame rate.
	//}
}

void Win8Application::Uninitialize() {

}

void Win8Application::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args) {
	//m_renderer->UpdateForWindowSizeChange();
}

void Win8Application::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args) {
	closed = true;
}

void Win8Application::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args) {
	CoreWindow::GetForCurrentThread()->Activate();
}

void Win8Application::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args) {
	//SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
	//deferral->Complete();
}
 
void Win8Application::OnResuming(Platform::Object^ sender, Platform::Object^ args) {

}

void Win8Application::OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args) {
	Mouse::the()->MouseDown(MouseEvent(static_cast<int>(args->CurrentPoint->Position.X), static_cast<int>(args->CurrentPoint->Position.Y)));
}

void Win8Application::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args) {
	Mouse::the()->MouseUp(MouseEvent(static_cast<int>(args->CurrentPoint->Position.X), static_cast<int>(args->CurrentPoint->Position.Y)));
}

void Win8Application::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args) {
	Mouse::the()->MouseMove(MouseEvent(static_cast<int>(args->CurrentPoint->Position.X), static_cast<int>(args->CurrentPoint->Position.Y)));
}

IFrameworkView^ Win8ApplicationSource::CreateView() {
	return ref new Win8Application;
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^) {
	CoreApplication::Run(ref new Win8ApplicationSource);
	return 0;
}