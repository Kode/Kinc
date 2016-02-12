#pragma once

#include "Window.h"

namespace Kore {
	enum Orientation {
		OrientationLandscapeLeft,
		OrientationLandscapeRight,
		OrientationPortrait,
		OrientationPortraitUpsideDown,
		OrientationUnknown
	};
	
	// TODO (DK) this whole class seems pretty useless and could be refactored?
	class Application {
	public:
		Application();//int argc, char** argv, int width = 800, int height = 600, int antialiasing = 0, int windowMode = 0, const char* name = "Kore", bool showWindow = true, int x = -1, int y = -1);
		~Application();
		void start();
		void stop();
		const char* name();
		int x();
		int y();
		int width();
		int height();
		int antialiasing();
		bool fullscreen();
		int windowMode();
		bool showWindow();
		void setWidth(int width);
		void setHeight(int height);
		void setFullscreen(bool fullscreen);
		void setCallback(void (*callback)());
		static Application* the();

		static Application * initDefault(int argc, char** argv, int width = 800, int height = 600, int antialiasing = 0, int windowMode = 0, const char* name = "Kore", bool showWindow = true, int x = -1, int y = -1 );
		static Application * initEx();
		static int initWindow( WindowOptions options );

		void (*callback)();
		void (*orientationCallback)(Orientation);
		void (*foregroundCallback)();
		void (*resumeCallback)();
		void (*pauseCallback)();
		void (*backgroundCallback)();
		void (*shutdownCallback)();
		
		bool running;
	};
}

int kore(int argc, char** argv);
