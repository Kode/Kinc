#include "pch.h"
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Input/Gamepad.h>
#include <Kore/Direct3D11.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#ifndef XINPUT
	#ifdef SYS_WINDOWS
		#define XINPUT 1
	#endif

	#ifdef SYS_WINDOWSAPP
		#define XINPUT !(WINAPI_PARTITION_PHONE_APP)
	#endif
#endif
#if XINPUT
#include <Xinput.h>
#endif

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
	//void OnLogicalDpiChanged(Platform::Object^ sender);
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
	void OnKeyDown(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args);
	void OnKeyUp(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args);
};

ref class Win8ApplicationSource : Windows::ApplicationModel::Core::IFrameworkViewSource {
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};

using namespace Kore;

namespace {
	int mouseX, mouseY;
	float axes[12 * 6];
	float buttons[12 * 16];
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

	#if XINPUT
	DWORD dwResult;
	for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
		XINPUT_STATE state;
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		dwResult = XInputGetState(i, &state);

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
		else {
			Kore::Gamepad::get(i)->vendor = nullptr;
			Kore::Gamepad::get(i)->productName = nullptr;
		}
	}
    #endif

	return true;
}

Kore::vec2i Kore::System::mousePos() {
	return vec2i(mouseX, mouseY);
}

void Kore::System::swapBuffers(int windowId) {
	
}

#undef CreateWindow

int Kore::System::initWindow(WindowOptions options) {
	Graphics::init(0, options.rendererOptions.depthBufferBits, options.rendererOptions.stencilBufferBits);
	return 0;
}

void Kore::System::setup() {

}

bool Kore::System::isFullscreen() {
	return true;
}

int Kore::System::windowCount() {
	return 1;
}

void Kore::System::makeCurrent(int windowId) {

}

void* Kore::System::windowHandle(int windowId) {
	return nullptr;
}

void Kore::System::destroyWindow(int windowId) {
	
}

void Kore::System::changeResolution(int width, int height, bool fullscreen) {

}

void Kore::System::setTitle(const char*) {

}

void Kore::System::setKeepScreenOn( bool on ) {
    
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

void Kore::System::loadURL(const char* url) {
    
}

int kore(int argc, char** argv);

extern int renderTargetWidth;
extern int renderTargetHeight;

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
	window->KeyDown += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>(this, &Win8Application::OnKeyDown);
	window->KeyUp += ref new Windows::Foundation::TypedEventHandler<Windows::UI::Core::CoreWindow ^, Windows::UI::Core::KeyEventArgs ^>(this, &Win8Application::OnKeyUp);
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
	mouseX = static_cast<int>(args->CurrentPoint->Position.X);
	mouseY = static_cast<int>(args->CurrentPoint->Position.Y);
	Mouse::the()->_press(0, 0, mouseX, mouseY);
}

void Win8Application::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args) {
	mouseX = static_cast<int>(args->CurrentPoint->Position.X);
	mouseY = static_cast<int>(args->CurrentPoint->Position.Y);
	Mouse::the()->_release(0, 0, mouseX, mouseY);
}

void Win8Application::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args) {
	mouseX = static_cast<int>(args->CurrentPoint->Position.X);
	mouseY = static_cast<int>(args->CurrentPoint->Position.Y);
	Mouse::the()->_move(0, mouseX, mouseY);
}

void Win8Application::OnKeyDown(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args) {
	switch (args->VirtualKey) {
	case Windows::System::VirtualKey::Left:
		Keyboard::the()->_keydown(Kore::Key_Left, L' ');
		break;
	case Windows::System::VirtualKey::Right:
		Keyboard::the()->_keydown(Kore::Key_Right, L' ');
		break;
	case Windows::System::VirtualKey::Up:
		Keyboard::the()->_keydown(Kore::Key_Up, L' ');
		break;
	case Windows::System::VirtualKey::Down:
		Keyboard::the()->_keydown(Kore::Key_Down, L' ');
		break;
	}
}

void Win8Application::OnKeyUp(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::KeyEventArgs ^args) {
	switch (args->VirtualKey) {
	case Windows::System::VirtualKey::Left:
		Keyboard::the()->_keyup(Kore::Key_Left, L' ');
		break;
	case Windows::System::VirtualKey::Right:
		Keyboard::the()->_keyup(Kore::Key_Right, L' ');
		break;
	case Windows::System::VirtualKey::Up:
		Keyboard::the()->_keyup(Kore::Key_Up, L' ');
		break;
	case Windows::System::VirtualKey::Down:
		Keyboard::the()->_keyup(Kore::Key_Down, L' ');
		break;
	}
}

IFrameworkView^ Win8ApplicationSource::CreateView() {
	return ref new Win8Application;
}

const char* Kore::System::savePath() {
	return "\\";
}

const char* Kore::System::systemId() {
	return "WindowsApp";
}

namespace {
	const char* videoFormats[] = { "ogv", nullptr };
}

const char** Kore::System::videoFormats() {
	return ::videoFormats;
}

int Kore::System::windowWidth(int windowId) {
	return renderTargetWidth;
}

int Kore::System::windowHeight(int windowId) {
	return renderTargetHeight;
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

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^) {
	CoreApplication::Run(ref new Win8ApplicationSource);
	return 0;
}
