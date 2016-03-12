#include "pch.h"
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>
#include <Kore/System.h>
#include <Kore/Graphics/Graphics.h>

#include "Display.h"

#include <cstring>

#include <stdio.h>
#include <stdlib.h>

#include <Kore/ogl.h>

#include <bcm_host.h>

//apt-get install mesa-common-dev
//apt-get install libgl-dev

namespace {
    //static int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8, None};
    //static int dblBuf[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8, GLX_DOUBLEBUFFER, None};


    GLboolean doubleBuffer = GL_TRUE;

    void fatalError(const char* message) {
        printf("main: %s\n", message);
        exit(1);
    }
}

namespace Kore { namespace Display {
    void enumDisplayMonitors( DeviceInfo screens[], int & displayCounter ) {
        displayCounter = 1;
    }
}}

void Kore::System::setup() {

}

bool Kore::System::isFullscreen() {
    // TODO (DK)
    return false;
}

// TODO (DK) the whole glx stuff should go into Graphics/OpenGL?
//  -then there would be a better separation between window + context setup
int
createWindow( const char * title, int x, int y, int width, int height, Kore::WindowMode windowMode, int targetDisplay, int depthBufferBits, int stencilBufferBits ) {
    return 0;
}

namespace Kore { namespace System {
    int windowCount() {
        return 1;
    }

    int windowWidth( int id ) {
        return 1920;
    }

    int windowHeight( int id ) {
        return 1080;
    }

    int initWindow( WindowOptions options ) {
        char buffer[1024] = {0};
        strcat(buffer, name());
        if (options.title != nullptr) {
            strcat(buffer, options.title);
        }

        int id = createWindow(buffer, options.x, options.y, options.width, options.height, options.mode, options.targetDisplay, options.rendererOptions.depthBufferBits, options.rendererOptions.stencilBufferBits);
        Graphics::init(id, options.rendererOptions.depthBufferBits, options.rendererOptions.stencilBufferBits);
        return id;
    }

    void* windowHandle( int id ) {
        return nullptr;
    }
}}

namespace Kore { namespace System {
    int currentDeviceId = -1;

    int currentDevice() {
        return currentDeviceId;
    }

    void setCurrentDevice( int id ) {
        currentDeviceId = id;
    }
}}

bool Kore::System::handleMessages() {

	return true;
}

const char* Kore::System::systemId() {
    return "Pi";
}

void Kore::System::makeCurrent( int contextId ) {
	if (currentDeviceId == contextId) {
		return;
	}

#if !defined(NDEBUG)
	log(Info, "Kore/System | context switch from %i to %i", currentDeviceId, contextId);
#endif

    currentDeviceId = contextId;


}

void Kore::Graphics::clearCurrent() {
}

void Kore::System::clearCurrent() {
#if !defined(NDEBUG)
	log(Info, "Kore/System | context clear");
#endif

    currentDeviceId = -1;
    Graphics::clearCurrent();
}

void Kore::System::swapBuffers( int contextId ) {
    // TODO mighty important
}

void Kore::System::destroyWindow( int id ) {
    // TODO (DK) implement me
}

void Kore::System::changeResolution(int width, int height, bool fullscreen) {

}

void Kore::System::setTitle(const char* title) {

}

void Kore::System::showWindow() {

}

void Kore::System::showKeyboard() {

}

void Kore::System::hideKeyboard() {

}

void Kore::System::loadURL(const char* url) {

}

namespace {
    char save[2000];
    bool saveInitialized = false;
}

const char* Kore::System::savePath() {
    if (!saveInitialized) {
        strcpy(save, "Ķ~/.");
        strcat(save, name());
        strcat(save, "/");
        saveInitialized = true;
    }
	return save;
}

namespace {
	const char* videoFormats[] = { "ogv", nullptr };

}

const char** Kore::System::videoFormats() {
	return ::videoFormats;
}

#include <sys/time.h>
#include <time.h>

double Kore::System::frequency() {
	return 1000000.0;
}

Kore::System::ticks Kore::System::timestamp() {
	timeval now;
	gettimeofday(&now, NULL);
	return static_cast<ticks>(now.tv_sec) * 1000000 + static_cast<ticks>(now.tv_usec);
}

extern int kore(int argc, char** argv);

int main(int argc, char** argv) {
    kore(argc, argv);
}
