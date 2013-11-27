/**
 * Name        : OGLTemplate
 * Version     :
 * Vendor      :
 * Description :
 */


#include "OGLTemplate.h"
#include "OGLTemplateFrame.h"
#include <Kore/Input/Mouse.h>
#include <Kore/Audio/Audio.h>

using namespace Tizen::App;
using namespace Tizen::Base;
using namespace Tizen::System;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;
using namespace Kore;

OGLTemplateApp::OGLTemplateApp(void)
{
}

OGLTemplateApp::~OGLTemplateApp(void)
{
}

UiApp*
OGLTemplateApp::CreateInstance(void)
{
	// Create the instance through the constructor.
	return new OGLTemplateApp();
}

bool
OGLTemplateApp::OnAppInitializing(AppRegistry& appRegistry)
{
	// TODO:
	// Initialize Frame and App specific data.
	// The App's permanent data and context can be obtained from the appRegistry.
	//
	// If this method is successful, return true; otherwise, return false.
	// If this method returns false, the App will be terminated.

	// Uncomment the following statement to listen to the screen on/off events.
	//PowerManager::SetScreenEventListener(*this);

	// TODO:
	// Add your initialization code here
	return true;
}

bool
OGLTemplateApp::OnAppInitialized(void)
{
	// TODO:
	// Add code to do after initialization here. 

	// Create a Frame
	OGLTemplateFrame* pOGLTemplateFrame = new OGLTemplateFrame();
	pOGLTemplateFrame->Construct();
	pOGLTemplateFrame->SetName(L"OGLTemplate");
	//pOGLTemplateFrame->SetOrientation(ORIENTATION_LANDSCAPE);	// To set orientation
	//Tizen::Graphics::CoordinateSystem::GetPhysicalResolution().width; // To get resolution
	AddFrame(*pOGLTemplateFrame);

	pOGLTemplateFrame->AddKeyEventListener(*this);
	pOGLTemplateFrame->AddTouchEventListener(*this);

	{
		__player = new Tizen::Graphics::Opengl::GlPlayer;
		__player->Construct(Tizen::Graphics::Opengl::EGL_CONTEXT_CLIENT_VERSION_2_X, pOGLTemplateFrame);

		__player->SetFps(60);
		__player->SetEglAttributePreset(Tizen::Graphics::Opengl::EGL_ATTRIBUTES_PRESET_RGB565);

		__player->Start();
	}

	__renderer = new GlRendererTemplate();
	__player->SetIGlRenderer(__renderer);

	StartAudio(); // Start AudioOut

	return true;
}

bool
OGLTemplateApp::OnAppWillTerminate(void)
{
	// TODO:
	// Add code to do somethiing before application termination. 
	return true;
}

bool
OGLTemplateApp::OnAppTerminating(AppRegistry& appRegistry, bool forcedTermination)
{
	// TODO:
	// Deallocate resources allocated by this App for termination.
	// The App's permanent data and context can be saved via appRegistry.

	StopAudio();

	__player->Stop();

	if(__renderer != null)
	{
		delete __renderer;
	}
	delete __player;

	return true;
}

void
OGLTemplateApp::OnForeground(void)
{
	// TODO:
	// Start or resume drawing when the application is moved to the foreground.

	if (__audioOut.GetState() == AUDIOOUT_STATE_STOPPED) __audioOut.Start();
}

void
OGLTemplateApp::OnBackground(void)
{
	// TODO:
	// Stop drawing when the application is moved to the background.

	if (__audioOut.GetState() == AUDIOOUT_STATE_PLAYING) __audioOut.Stop();
}

void
OGLTemplateApp::OnLowMemory(void)
{
	// TODO:
	// Free unused resources or close the application.
}

void
OGLTemplateApp::OnBatteryLevelChanged(BatteryLevel batteryLevel)
{
	// TODO:
	// Handle any changes in battery level here.
	// Stop using multimedia features(camera, mp3 etc.) if the battery level is CRITICAL.
}

void
OGLTemplateApp::OnScreenOn(void)
{
	// TODO:
	// Get the released resources or resume the operations that were paused or stopped in OnScreenOff().

	if (__audioOut.GetState() == AUDIOOUT_STATE_STOPPED) __audioOut.Start();
}

void
OGLTemplateApp::OnScreenOff(void)
{
	// TODO:
	// Unless there is a strong reason to do otherwise, release resources (such as 3D, media, and sensors) to allow the device
	// to enter the sleep mode to save the battery.
	// Invoking a lengthy asynchronous method within this listener method can be risky, because it is not guaranteed to invoke a
	// callback before the device enters the sleep mode.
	// Similarly, do not perform lengthy operations in this listener method. Any operation must be a quick one.

	if (__audioOut.GetState() == AUDIOOUT_STATE_PLAYING) __audioOut.Stop();
}

void
OGLTemplateApp::OnKeyPressed(const Control& source, KeyCode keyCode)
{
	// TODO:
}

void
OGLTemplateApp::OnKeyReleased(const Control& source, KeyCode keyCode)
{
	// TODO:

	if (keyCode == Tizen::Ui::KEY_BACK || keyCode == Tizen::Ui::KEY_ESC)
	{
		Terminate();
	}
}

void
OGLTemplateApp::OnKeyLongPressed(const Control& source, KeyCode keyCode)
{
	// TODO:
}

void
OGLTemplateApp::OnTouchMoved(const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	Kore::Mouse::the()->_move(Kore::MouseEvent(currentPosition.x, currentPosition.y));
}

void
OGLTemplateApp::OnTouchPressed(const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	Kore::Mouse::the()->_pressLeft(Kore::MouseEvent(currentPosition.x, currentPosition.y));
}

void
OGLTemplateApp::OnTouchReleased(const Tizen::Ui::Control &source, const Tizen::Graphics::Point &currentPosition, const Tizen::Ui::TouchEventInfo &touchInfo)
{
	Kore::Mouse::the()->_releaseLeft(Kore::MouseEvent(currentPosition.x, currentPosition.y));
}

result
OGLTemplateApp::StartAudio(void)
{
	result r;

	// Constructs the AudioOut instance with a listener
	r = __audioOut.Construct(*this);
	if (IsFailed(r)) return r;

	// Prepares the AudioOut instance
	__audioOut.Prepare(AUDIO_TYPE_PCM_S16_LE, AUDIO_CHANNEL_TYPE_STEREO, 44100);
	__numSamples = 882; //__audioOut.GetMinBufferSize();

	// Constructs the pcm buffer and enqueue the buffer to the __audioOut
	__buffer.Construct(__numSamples * 4);

	writeAudio();
	__audioOut.WriteBuffer(__buffer);

	// Starts playing
	r = __audioOut.Start();
	if (IsFailed(r)) return r;

	return E_SUCCESS;
}

void
OGLTemplateApp::StopAudio(void)
{
    __audioOut.Stop();
    __audioOut.Unprepare();
}

void
OGLTemplateApp::OnAudioOutBufferEndReached(Tizen::Media::AudioOut& src)
{
	writeAudio();
	__audioOut.WriteBuffer(__buffer);
}

void
OGLTemplateApp::copySample()
{
	float value = *(float*)&Audio::buffer.data[Audio::buffer.readLocation];
	Audio::buffer.readLocation += 4;
	if (Audio::buffer.readLocation >= Audio::buffer.dataSize) Audio::buffer.readLocation = 0;
	__buffer.SetShort(static_cast<s16>(value * 32767));
}

void
OGLTemplateApp::writeAudio()
{
	if (Kore::Audio::audioCallback != nullptr)
	{
		Kore::Audio::audioCallback(__numSamples * 2);

		__buffer.Clear();
		for (int i = 0; i < __numSamples; ++i)
		{
			copySample();
			copySample();
		}
	}
}