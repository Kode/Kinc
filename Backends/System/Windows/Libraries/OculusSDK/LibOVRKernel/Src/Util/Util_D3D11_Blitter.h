/************************************************************************************

Filename    :   Util_D3D11_Blitter.h
Content     :   D3D11 implementation for blitting, supporting scaling
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

#ifndef OVR_Util_D3D11_Blitter_h
#define OVR_Util_D3D11_Blitter_h

#include "Kernel/OVR_RefCount.h"
#include "Kernel/OVR_Log.h"

#ifdef OVR_OS_MS

#include <d3d11_1.h>

#include "OVR_CAPI.h"

namespace OVR {
namespace D3DUtil {

//-------------------------------------------------------------------------------------
// ***** CAPI::Blitter

// D3D11 implementation of blitter

class Blitter : public RefCountBase<Blitter> {
 public:
  Blitter(const Ptr<ID3D11Device>& device);
  ~Blitter();

  bool Initialize(bool single_channel = false);

  bool Blt(ID3D11RenderTargetView* dest, ID3D11ShaderResourceView* source);
  bool Blt(
      ID3D11RenderTargetView* dest,
      ID3D11ShaderResourceView* source,
      uint32_t topLeftX,
      uint32_t topLeftY,
      uint32_t width,
      uint32_t height);

 private:
  enum PixelShaders { OneMSAA = 0, TwoOrMoreMSAA = 1, Grayscale = 2, ShaderCount = 3 };

  Ptr<ID3D11Device1> Device;
  Ptr<ID3D11DeviceContext1> Context1;
  Ptr<ID3DDeviceContextState> BltState;
  Ptr<ID3D11InputLayout> IL;
  Ptr<ID3D11Buffer> VB;
  Ptr<ID3D11VertexShader> VS;

  std::array<Ptr<ID3D11PixelShader>, PixelShaders::ShaderCount> PS;
  Ptr<ID3D11SamplerState> Sampler;
  Ptr<ID3D11DepthStencilState> DepthState;
  bool AlreadyInitialized;
  bool SingleChannel;

  struct BltVertex {
    float x, y;
    float u, v;
  };
};

// Writes a D3D texture to a file path.
class D3DTextureWriter {
 public:
  D3DTextureWriter(ID3D11Device* deviceNew = nullptr);

  void Shutdown();

  void SetDevice(ID3D11Device* deviceNew);

  enum class Result {
    SUCCESS,
    NULL_SURFACE,
    NULL_DEVICE,
    TEXTURE_CREATION_FAILURE,
    TEXTURE_MAP_FAILURE,
    FILE_CREATION_FAILURE,
  };

  // Beware that if the texture being saved is one that is a render target then the rendering to
  // that
  // texture will need to be complete for this to work properly. You may need to flush the device or
  // command buffer to achieve this.
  // If copyTexture is true then we make a copy of the input texture before writing it to disk.
  // If texture is mapped for writing then you may want to use copyTexture because reading from it
  // will be slow.
  Result GrabPixels(
      ID3D11Texture2D* texture,
      UINT subresource,
      bool copyTexture,
      const ovrTimewarpProjectionDesc* depthProj,
      const float* linearDepthScale);

  Result SavePixelsToBMP(const wchar_t* path);

  // Simple composition of GrabPixels() and SavePixelsToBMP() functions
  Result SaveTexture(
      ID3D11Texture2D* texture,
      UINT subresource,
      bool copyTexture,
      const wchar_t* path,
      const ovrTimewarpProjectionDesc* depthProj,
      const float* linearDepthScale);

  static uint32_t* ConvertRGBA2BGRA(const uint32_t* src, uint32_t* dst, unsigned pixelCount);
  static char* ConvertBGRA2RGB(const uint32_t* src, char* dst, unsigned pixelCount);

 protected:
  Ptr<ID3D11Device> device; // D3D11Device we use. Must match the textures we work with.
  Ptr<ID3D11Texture2D> textureCopy; // The last texture we used. Cached for future use.
  D3D11_TEXTURE2D_DESC textureCopyDesc; // This is a D3D11_TEXTURE2D_DESC. The description of
  // textureCopy, which allows us to know if we need to free
  // it and reallocate it anew.
  std::pair<UINT, UINT> pixelsDimentions = {0, 0};
  std::unique_ptr<uint32_t[]> pixels; // Windows RGB .bmp files are actually in BGRA or BGR format.
};
} // namespace D3DUtil
} // namespace OVR

#endif // OVR_OS_MS
#endif // OVR_Util_D3D11_Blitter_h
