#include "pch.h"
#include <Kore/System.h>
#include <Kore/Application.h>
#include <Kore/Input/KeyEvent.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Direct3D11.h>
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

using namespace Kore;

namespace {
	int mouseX, mouseY;
	bool keyPressed[256];
	Kore::KeyCode keyTranslated[256]; //http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx

	void initKeyTranslation() {
		for (int i = 0; i < 256; ++i) keyTranslated[i] = Kore::Key_unknown;
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
}

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

bool Kore::System::handleMessages() {
	CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
	return true;
}

Kore::vec2i Kore::System::mousePos() {
	return vec2i(mouseX, mouseY);
}

void Kore::System::swapBuffers() {
	
}

#undef CreateWindow

void* Kore::System::createWindow() {
	return nullptr;
}

void* Kore::System::windowHandle() {
	return nullptr;
}

void Kore::System::destroyWindow() {
	
}

void Kore::System::changeResolution(int width, int height, bool fullscreen) {

}

void Kore::System::setTitle(const char*) {

}

void Kore::System::showWindow() {

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

int kore(int argc, char** argv);

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
	kore(0, nullptr);
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
	Mouse::the()->_pressLeft(MouseEvent(static_cast<int>(args->CurrentPoint->Position.X), static_cast<int>(args->CurrentPoint->Position.Y)));
}

void Win8Application::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args) {
	Mouse::the()->_releaseLeft(MouseEvent(static_cast<int>(args->CurrentPoint->Position.X), static_cast<int>(args->CurrentPoint->Position.Y)));
}

void Win8Application::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args) {
	Mouse::the()->_move(MouseEvent(static_cast<int>(args->CurrentPoint->Position.X), static_cast<int>(args->CurrentPoint->Position.Y)));
}

IFrameworkView^ Win8ApplicationSource::CreateView() {
	return ref new Win8Application;
}

Kore::System::ticks Kore::System::getFrequency() {
	ticks rate;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&rate));
	return rate;
}

Kore::System::ticks Kore::System::getTimestamp() {
	ticks stamp;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stamp));
	return stamp;
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^) {
	CoreApplication::Run(ref new Win8ApplicationSource);
	return 0;
}
