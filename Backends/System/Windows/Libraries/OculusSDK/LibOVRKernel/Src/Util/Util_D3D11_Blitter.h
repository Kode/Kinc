/************************************************************************************

Filename    :   Util_D3D11_Blitter.h
Content     :   D3D11 implementation for blitting, supporting scaling
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

#ifndef OVR_Util_D3D11_Blitter_h
#define OVR_Util_D3D11_Blitter_h

#include "Kernel/OVR_RefCount.h"

#ifdef OVR_OS_MS

#include <d3d11_1.h>

namespace OVR { namespace D3DUtil {

//-------------------------------------------------------------------------------------
// ***** CAPI::Blitter

// D3D11 implementation of blitter

class Blitter : public RefCountBase<Blitter>
{
public:
    Blitter(const Ptr<ID3D11Device>& device);
    ~Blitter();

    bool Initialize();

    bool Blt(ID3D11RenderTargetView* dest, ID3D11ShaderResourceView* source);

private:
    Ptr<ID3D11Device1>          Device;
    Ptr<ID3D11DeviceContext1>   Context1;
    Ptr<ID3DDeviceContextState> BltState;
    Ptr<ID3D11InputLayout>      IL;
    Ptr<ID3D11Buffer>           VB;
    Ptr<ID3D11VertexShader>     VS;
    Ptr<ID3D11PixelShader>      PS;
    Ptr<ID3D11SamplerState>     Sampler;
    bool                        AlreadyInitialized;

    struct BltVertex
    {
        float x, y;
        float u, v;
    };
};

}} // namespace OVR::D3DUtil

#endif // OVR_OS_MS
#endif // OVR_Util_D3D11_Blitter_h
