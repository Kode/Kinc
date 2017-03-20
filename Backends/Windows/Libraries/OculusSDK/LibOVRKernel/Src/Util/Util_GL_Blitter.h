/************************************************************************************

Filename    :   Util_GL_Blitter.h
Content     :   GL implementation for blitting, supporting scaling & rotation
Created     :   February 24, 2015
Authors     :   Reza Nourai

Copyright   :   Copyright 2014-2016 Oculus VR, LLC All Rights reserved.

Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License");
you may not use the Oculus VR Rift SDK except in compliance with the License,
which is provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-3.3

Unless required by applicable law or agreed to in writing, the Oculus VR SDK
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#ifndef OVR_Util_GL_Blitter_h
#define OVR_Util_GL_Blitter_h

#include "Kernel/OVR_RefCount.h"
#include "Kernel/OVR_Win32_IncludeWindows.h"

#include "GL/CAPI_GLE.h"

#if defined(OVR_OS_WIN32)
#include <gl/GL.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace OVR { namespace GLUtil {

//-------------------------------------------------------------------------------------
// ***** CAPI::Blitter

// D3D11 implementation of blitter

class Blitter : public RefCountBase<Blitter>
{
public:
    Blitter();
    ~Blitter();

    bool Initialize();

    // Blit sourceTexture to the active frame buffer
    bool Blt(GLuint sourceTexId);

private:
    GLuint ReadFBO;
};

}} // namespace OVR::GLUtil

#endif // OVR_Util_GL_Blitter_h
