#pragma once

namespace Kore {
	enum Orientation {
		OrientationLandscapeLeft,
		OrientationLandscapeRight,
		OrientationPortrait,
		OrientationPortraitUpsideDown,
		OrientationUnknown
	};
	
	class Application {
	public:
		Application(int argc, char** argv, int width = 800, int height = 600, int antialiasing = 0, bool fullscreen = false, const char* name = "Kore", bool showWindow = true);
		~Application();
		void start();
		void stop();
		const char* name();
		int width();
		int height();
		int antialiasing();
		bool fullscreen();
		bool showWindow();
		void setWidth(int width);
		void setHeight(int height);
		void setFullscreen(bool fullscreen);
		void setCallback(void (*callback)());
		static Application* the();

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
