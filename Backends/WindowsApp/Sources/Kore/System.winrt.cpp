#include "pch.h"
#include <Kore/System.h>
#include <Kore/Application.h>
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
	Mouse::the()->_press(0, mouseX, mouseY);
}

void Win8Application::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args) {
	mouseX = static_cast<int>(args->CurrentPoint->Position.X);
	mouseY = static_cast<int>(args->CurrentPoint->Position.Y);
	Mouse::the()->_release(0, mouseX, mouseY);
}

void Win8Application::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args) {
	mouseX = static_cast<int>(args->CurrentPoint->Position.X);
	mouseY = static_cast<int>(args->CurrentPoint->Position.Y);
	Mouse::the()->_move(mouseX, mouseY);
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

int Kore::System::screenWidth() {
	return renderTargetWidth;
}

int Kore::System::screenHeight() {
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
