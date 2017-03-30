/************************************************************************************

Filename    :   Util_GL_Blitter.cpp
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

#include "Util_GL_Blitter.h"

namespace OVR { namespace GLUtil {

#define AssertOnGLError() { GLuint err = glGetError(); OVR_ASSERT_AND_UNUSED(!err,err); }

//-------------------------------------------------------------------------------------
// ***** CAPI::Blitter

Blitter::Blitter()
    : ReadFBO(0)
{
}

Blitter::~Blitter()
{
    glDeleteFramebuffers(1, &ReadFBO);
    ReadFBO = 0;
}

bool Blitter::Initialize()
{
    glGenFramebuffers(1, &ReadFBO);
    AssertOnGLError();

    return true;
}

bool Blitter::Blt(GLuint sourceTexId)
{
    GLenum status = 0;

    GLint currentTex2D = 0;
    GLint sourceWidth = 0, sourceHeight = 0;

    // Store off currently selected tex2d
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTex2D);

    // Get source texture dimensions
    glBindTexture(GL_TEXTURE_2D, sourceTexId);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &sourceWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &sourceHeight);

    // Put old texture back
    glBindTexture(GL_TEXTURE_2D, currentTex2D);

    // Save off the current FBOs
    GLint currentReadFB;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &currentReadFB);
    AssertOnGLError();

    // setup draw buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);

    // setup read buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, ReadFBO);
    AssertOnGLError();

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sourceTexId, 0);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    AssertOnGLError();
    status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
    OVR_ASSERT_AND_UNUSED(status == GL_FRAMEBUFFER_COMPLETE, status);

    // Do the blt
    glBlitFramebuffer(0, sourceHeight, sourceWidth, 0,
                      0, 0, sourceWidth, sourceHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    AssertOnGLError();

    // Restore the previous FBOs
    glBindFramebuffer(GL_READ_FRAMEBUFFER, currentReadFB);
    AssertOnGLError();

    return true;
}

}} // namespace OVR::GLUtil
