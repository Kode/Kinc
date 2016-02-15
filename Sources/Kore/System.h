#pragma once

#include <Kore/Math/Vector.h>
#include <Kore/Window.h>

namespace Kore {
	enum Orientation {
		OrientationLandscapeLeft,
		OrientationLandscapeRight,
		OrientationPortrait,
		OrientationPortraitUpsideDown,
		OrientationUnknown
	};

	// TODO (DK) remove windowing stuff from here and put into Kore::Window?
	namespace System {
		void setup();

		int currentDevice();
		void setCurrentDevice(int id);
		int windowWidth(int id);
		int windowHeight(int id);
		int windowCount();
		
		void setName( const char * name );

		int initWindow( WindowOptions options );
		int createWindow( const char * title, int x, int y, int width, int height, int windowMode, int targetDisplay );
		void destroyWindow(int id);
		void* windowHandle(int windowId);

		void changeResolution(int width, int height, bool fullscreen);
		bool handleMessages();
		vec2i mousePos();
		void showKeyboard();
		void hideKeyboard();
		bool showsKeyboard();
		void loadURL(const char* title);
		int screenWidth(); // main window
		int screenHeight(); // main window
		int desktopWidth();
		int desktopHeight();
		const char* systemId();
		void setTitle(const char* title);
		const char* savePath();
		const char** videoFormats();
		void showWindow();
		void swapBuffers( int contextId );

		typedef unsigned long long ticks;

		double frequency();
		ticks timestamp();
		double time();

		// (DK) old application interface
		void start();
		void stop();
		bool isFullscreen();

		void setCallback( void (*value)() );
		void setForegroundCallback( void (*value)() );
		void setResumeCallback( void (*value)() );
		void setPauseCallback( void (*value)() );
		void setBackgroundCallback( void (*value)() );
		void setShutdownCallback( void (*value)() );
		void setOrientationCallback( void (*value)(Orientation) );
	}
}
