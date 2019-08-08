#pragma once

#include <Kore/Math/Vector.h>
#include <Kore/Window.h>

namespace Kore {
	enum Orientation { OrientationLandscapeLeft, OrientationLandscapeRight, OrientationPortrait, OrientationPortraitUpsideDown, OrientationUnknown };

	namespace System {
		Window* init(const char* name, int width, int height, WindowOptions* win = nullptr, FramebufferOptions* frame = nullptr);

		const char* name();
		int windowWidth(int id = 0);
		int windowHeight(int id = 0);

		bool handleMessages();
		vec2i mousePos();
		void showKeyboard();
		void hideKeyboard();
		bool showsKeyboard();
		void loadURL(const char* title);
		const char* language();
		void vibrate(int ms);
		const char* systemId();
		const char* savePath();
		const char** videoFormats();
		float safeZone();
		bool automaticSafeZone();
		void setSafeZone(float value);

		typedef u64 ticks;

		double frequency();
		ticks timestamp();
		double time();

		void start();
		bool frame();
		void stop();

 		void setKeepScreenOn(bool on);

		void login();
		void unlockAchievement(int id);

		void setCallback(void (*value)());
		void setForegroundCallback(void (*value)());
		void setResumeCallback(void (*value)());
		void setPauseCallback(void (*value)());
		void setBackgroundCallback(void (*value)());
		void setShutdownCallback(void (*value)());
		void setOrientationCallback(void (*value)(Orientation));
		void setDropFilesCallback(void (*value)(wchar_t*));
		void setCutCallback(char* (*value)());
		void setCopyCallback(char* (*value)());
		void setPasteCallback(void (*value)(char*));
		void setLoginCallback(void (*value)());
		void setLogoutCallback(void (*value)());

		void _shutdown();
		void _callback();
		void _foregroundCallback();
		void _resumeCallback();
		void _pauseCallback();
		void _backgroundCallback();
		void _shutdownCallback();
		void _orientationCallback(Orientation);
		void _dropFilesCallback(wchar_t*);
		char* _cutCallback();
		char* _copyCallback();
		void _pasteCallback(char*);
	}
}
