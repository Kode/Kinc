/************************************************************************************

Filename    :   Util_GL_Blitter.cpp
Content     :   GL implementation for blitting, supporting scaling & rotation
Created     :   February 24, 2015
Authors     :   Reza Nourai

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

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

namespace OVR {
namespace GLUtil {

#define AssertOnGLError()             \
  {                                   \
    GLuint err = glGetError();        \
    OVR_ASSERT_AND_UNUSED(!err, err); \
  }

//-------------------------------------------------------------------------------------
// ***** CAPI::Blitter

Blitter::Blitter() : ReadFBO(0) {}

Blitter::~Blitter() {
  glDeleteFramebuffers(1, &ReadFBO);
  ReadFBO = 0;
}

bool Blitter::Initialize() {
  glGenFramebuffers(1, &ReadFBO);
  AssertOnGLError();

  return true;
}

bool Blitter::Blt(GLuint sourceTexId) {
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
  glBlitFramebuffer(
      0,
      sourceHeight,
      sourceWidth,
      0,
      0,
      0,
      sourceWidth,
      sourceHeight,
      GL_COLOR_BUFFER_BIT,
      GL_NEAREST);
  AssertOnGLError();

  // Restore the previous FBOs
  glBindFramebuffer(GL_READ_FRAMEBUFFER, currentReadFB);
  AssertOnGLError();

  return true;
}

bool Blitter::Blt(
    GLuint sourceTexId,
    uint32_t topLeftX,
    uint32_t topLeftY,
    uint32_t width,
    uint32_t height) {
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

  // Get the default framebuffer size :
  GLint dims[4] = {0};
  glGetIntegerv(GL_VIEWPORT, dims);
  GLint fbHeight = dims[3];

  // setup read buffer
  glBindFramebuffer(GL_READ_FRAMEBUFFER, ReadFBO);
  AssertOnGLError();

  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sourceTexId, 0);
  glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
  AssertOnGLError();
  status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
  OVR_ASSERT_AND_UNUSED(status == GL_FRAMEBUFFER_COMPLETE, status);

  // Do the blt
  glBlitFramebuffer(
      0,
      sourceHeight,
      sourceWidth,
      0,
      (GLint)topLeftX,
      fbHeight - (GLint)topLeftY - (GLint)height,
      (GLint)topLeftX + (GLint)width,
      fbHeight - (GLint)topLeftY,
      GL_COLOR_BUFFER_BIT,
      GL_NEAREST);
  AssertOnGLError();

  // Restore the previous FBOs
  glBindFramebuffer(GL_READ_FRAMEBUFFER, currentReadFB);
  AssertOnGLError();

  return true;
}

bool Blitter::BltCubemap(GLuint sourceTexId, GLuint tempTexId, uint32_t cubeMapSize) {
  GLenum status = 0;
  GLint currentTex2D = 0;
  GLint size = (GLint)cubeMapSize;
  const int numFaces = 6;

  // Store off currently selected tex2d
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTex2D);

  // Save off the current FBOs
  GLint currentReadFB;
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &currentReadFB);
  AssertOnGLError();

  for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex) {
    // Create framebuffers
    GLuint drawCubeFBO;
    glGenFramebuffers(1, &drawCubeFBO);
    AssertOnGLError();

    GLuint readCubeFBO;
    glGenFramebuffers(1, &readCubeFBO);
    AssertOnGLError();

    GLuint drawFinalCubeFBO;
    glGenFramebuffers(1, &drawFinalCubeFBO);
    AssertOnGLError();

    // Setup draw buffer with temp texture to hold flipped face
    // tempTexId holds a Texture2D the size of one cubemap face (cubeMapSize)
    // It is used to temporarily hold a flipped face before blitting it to the original cubemap
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawCubeFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempTexId, 0);

    // Setup read buffer with original cubemap texture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readCubeFBO);
    AssertOnGLError();

    glFramebufferTexture2D(
        GL_READ_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex,
        sourceTexId,
        0);
    AssertOnGLError();
    status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
    OVR_ASSERT_AND_UNUSED(status == GL_FRAMEBUFFER_COMPLETE, status);

    // Do the blt
    glBlitFramebuffer(0, size, size, 0, 0, size, size, 0, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    AssertOnGLError();

    // Setup final draw buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFinalCubeFBO);
    glFramebufferTexture2D(
        GL_DRAW_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex,
        sourceTexId,
        0);

    // Setup read buffer to be the previous draw buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, drawCubeFBO); // flipped FBO
    AssertOnGLError();

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempTexId, 0);
    AssertOnGLError();
    status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
    OVR_ASSERT_AND_UNUSED(status == GL_FRAMEBUFFER_COMPLETE, status);

    // Do the blt again
    glBlitFramebuffer(0, size, size, 0, 0, 0, size, size, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    AssertOnGLError();

    // Delete framebuffers
    glDeleteFramebuffers(1, &drawCubeFBO);
    drawCubeFBO = 0;
    glDeleteFramebuffers(1, &readCubeFBO);
    readCubeFBO = 0;
    glDeleteFramebuffers(1, &drawFinalCubeFBO);
    drawFinalCubeFBO = 0;
  }

  // Restore the previous FBOs
  glBindFramebuffer(GL_READ_FRAMEBUFFER, currentReadFB);
  AssertOnGLError();

  return true;
}
} // namespace GLUtil
} // namespace OVR
