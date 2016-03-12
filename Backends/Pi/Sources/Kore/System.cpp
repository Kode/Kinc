#include "pch.h"
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>
#include <Kore/System.h>
#include <Kore/Graphics/Graphics.h>

#include "Display.h"

#include <assert.h>
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

    uint32_t screen_width;
    uint32_t screen_height;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

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
    bcm_host_init();

    int32_t success = 0;
    EGLBoolean result;
    EGLint num_config;

    static EGL_DISPMANX_WINDOW_T nativewindow;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    static const EGLint attribute_list[] =
    {
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_NONE
    };

    static const EGLint context_attributes[] =
    {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
    };
    EGLConfig config;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(display!=EGL_NO_DISPLAY);
    glCheckErrors();

    result = eglInitialize(display, NULL, NULL);
    assert(EGL_FALSE != result);
    glCheckErrors();

    result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
    assert(EGL_FALSE != result);
    glCheckErrors();

    result = eglBindAPI(EGL_OPENGL_ES_API);
    assert(EGL_FALSE != result);
    glCheckErrors();

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
    assert(context!=EGL_NO_CONTEXT);
    glCheckErrors();

    success = graphics_get_display_size(0 /* LCD */, &screen_width, &screen_height);
    assert( success >= 0 );

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = screen_width;
    dst_rect.height = screen_height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = screen_width << 16;
    src_rect.height = screen_height << 16;

    dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
    dispman_update = vc_dispmanx_update_start( 0 );

    dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
      0/*layer*/, &dst_rect, 0/*src*/,
      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);

    nativewindow.element = dispman_element;
    nativewindow.width = screen_width;
    nativewindow.height = screen_height;
    vc_dispmanx_update_submit_sync( dispman_update );

    glCheckErrors();

    surface = eglCreateWindowSurface( display, config, &nativewindow, NULL );
    assert(surface != EGL_NO_SURFACE);
    glCheckErrors();

    result = eglMakeCurrent(display, surface, surface, context);
    assert(EGL_FALSE != result);
    glCheckErrors();

    return 0;
}

namespace Kore { namespace System {
    int windowCount() {
        return 1;
    }

    int windowWidth( int id ) {
        return screen_width;
    }

    int windowHeight( int id ) {
        return screen_height;
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
    eglSwapBuffers(display, surface);
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
