#ifndef _OGLTEMPLATE_H_
#define _OGLTEMPLATE_H_

#include <FApp.h>
#include <FBase.h>
#include <FSystem.h>
#include <FUi.h>
#include <FUiIme.h>
#include <FGraphics.h>
#include <gl.h>
#include <FGrpGlPlayer.h>

#include "GlRendererTemplate.h"

/**
 * [OGLTemplateApp] UiApp must inherit from UiApp class
 * which provides basic features necessary to define an UiApp.
 */
class OGLTemplateApp
	: public Tizen::App::UiApp
	, public Tizen::System::IScreenEventListener
	, public Tizen::Ui::IKeyEventListener
{
public:
	/**
	 * [Test] UiApp must have a factory method that creates an instance of itself.
	 */
	static Tizen::App::UiApp* CreateInstance(void);

public:
	OGLTemplateApp(void);
	virtual~OGLTemplateApp(void);

public:
	// Called when the UiApp is initializing.
	virtual bool OnAppInitializing(Tizen::App::AppRegistry& appRegistry);

	// Called when the UiApp initializing is finished. 
	virtual bool OnAppInitialized(void); 

	// Called when the UiApp is requested to terminate. 
	virtual bool OnAppWillTerminate(void); 

	// Called when the UiApp is terminating.
	virtual bool OnAppTerminating(Tizen::App::AppRegistry& appRegistry, bool forcedTermination = false);

	// Called when the UiApp's frame moves to the top of the screen.
	virtual void OnForeground(void);

	// Called when this UiApp's frame is moved from top of the screen to the background.
	virtual void OnBackground(void);

	// Called when the system memory is not sufficient to run the UiApp any further.
	virtual void OnLowMemory(void);

	// Called when the battery level changes.
	virtual void OnBatteryLevelChanged(Tizen::System::BatteryLevel batteryLevel);

	// Called when the screen turns on.
	virtual void OnScreenOn(void);

	// Called when the screen turns off.
	virtual void OnScreenOff(void);

	// Called when a key is pressed.
	virtual void OnKeyPressed(const Tizen::Ui::Control& source, Tizen::Ui::KeyCode keyCode);

	// Called when a key is released.
	virtual void OnKeyReleased(const Tizen::Ui::Control& source, Tizen::Ui::KeyCode keyCode);

	// Called when a key is pressed and held down for some time.
	virtual void OnKeyLongPressed(const Tizen::Ui::Control& source, Tizen::Ui::KeyCode keyCode);

private:
	Tizen::Graphics::Opengl::GlPlayer* __player;
	Tizen::Graphics::Opengl::IGlRenderer* __renderer;
};

#endif // _OGLTEMPLATE_H_
