/************************************************************************************

Filename    :   Util_D3D11_Blitter.cpp
Content     :   D3D11 implementation for blitting, supporting scaling & rotation
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

#include "Util_D3D11_Blitter.h"

#ifdef OVR_OS_MS

#include "Util/Util_Direct3D.h"
#include "Shaders/Blt_vs.h"
#include "Shaders/Blt_ps.h"

namespace OVR { namespace D3DUtil {

//-------------------------------------------------------------------------------------
// ***** CAPI::Blitter

Blitter::Blitter(const Ptr<ID3D11Device>& device) :
    Device(),
    Context1(),
    BltState(),
    IL(),
    VB(),
    VS(),
    PS(),
    Sampler(),
    AlreadyInitialized(false)
{
    device->QueryInterface(IID_PPV_ARGS(&Device.GetRawRef()));
    OVR_ASSERT(Device);

    Device->GetImmediateContext1(&Context1.GetRawRef());
    OVR_ASSERT(Context1);
}

Blitter::~Blitter()
{
}

bool Blitter::Initialize()
{
    if (!Device)
    {
        OVR_ASSERT(false);
        return false;
    }

    OVR_ASSERT(!AlreadyInitialized);
    if (AlreadyInitialized)
    {
        return false;
    }

    UINT deviceFlags = Device->GetCreationFlags();
    D3D_FEATURE_LEVEL featureLevel = Device->GetFeatureLevel();

    // If the device is single threaded, the context state must be too
    UINT stateFlags = 0;
    if (deviceFlags & D3D11_CREATE_DEVICE_SINGLETHREADED)
    {
        stateFlags |= D3D11_1_CREATE_DEVICE_CONTEXT_STATE_SINGLETHREADED;
    }

    // TODO: Clean this up with OVR_D3D_CREATE() when we move OVRError to kernel.

    OVR_ASSERT(!BltState); // Expected to be null on the way in.
    BltState = nullptr; // Prevents a potential leak on the next line.
    HRESULT hr = Device->CreateDeviceContextState(stateFlags, &featureLevel, 1, D3D11_SDK_VERSION, __uuidof(ID3D11Device1), nullptr, &BltState.GetRawRef());
    OVR_D3D_CHECK_RET_FALSE(hr);
    OVR_D3D_TAG_OBJECT(BltState);

    OVR_ASSERT(!VS); // Expected to be null on the way in.
    VS = nullptr; // Prevents a potential leak on the next line.
    hr = Device->CreateVertexShader(Blt_vs, sizeof(Blt_vs), nullptr, &VS.GetRawRef());
    OVR_D3D_CHECK_RET_FALSE(hr);
    OVR_D3D_TAG_OBJECT(VS);

    OVR_ASSERT(!PS); // Expected to be null on the way in.
    PS = nullptr; // Prevents a potential leak on the next line.
    hr = Device->CreatePixelShader(Blt_ps, sizeof(Blt_ps), nullptr, &PS.GetRawRef());
    OVR_D3D_CHECK_RET_FALSE(hr);
    OVR_D3D_TAG_OBJECT(PS);

    D3D11_INPUT_ELEMENT_DESC elems[2] = {};
    elems[0].Format = DXGI_FORMAT_R32G32_FLOAT;
    elems[0].SemanticName = "POSITION";
    elems[1].AlignedByteOffset = sizeof(float)* 2;
    elems[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    elems[1].SemanticName = "TEXCOORD";

    OVR_ASSERT(!IL); // Expected to be null on the way in.
    IL = nullptr; // Prevents a potential leak on the next line.
    hr = Device->CreateInputLayout(elems, _countof(elems), Blt_vs, sizeof(Blt_vs), &IL.GetRawRef());
    OVR_D3D_CHECK_RET_FALSE(hr);
    OVR_D3D_TAG_OBJECT(IL);

    // Quad with texcoords designed to rotate the source 90deg clockwise
    BltVertex vertices[] =
    {
        { -1, 1, 0, 0 },
        { 1, 1, 1, 0 },
        { 1, -1, 1, 1 },
        { -1, 1, 0, 0 },
        { 1, -1, 1, 1 },
        { -1, -1, 0, 1 }
    };

    D3D11_BUFFER_DESC bd = {};
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = sizeof(vertices);
    bd.StructureByteStride = sizeof(BltVertex);
    bd.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA init = {};
    init.pSysMem = vertices;
    init.SysMemPitch = sizeof(vertices);
    init.SysMemSlicePitch = init.SysMemPitch;

    OVR_ASSERT(!VB); // Expected to be null on the way in.
    VB = nullptr; // Prevents a potential leak on the next line.
    hr = Device->CreateBuffer(&bd, &init, &VB.GetRawRef());
    OVR_D3D_CHECK_RET_FALSE(hr);
    OVR_D3D_TAG_OBJECT(VB);

    D3D11_SAMPLER_DESC ss = {};
    ss.AddressU = ss.AddressV = ss.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    ss.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    ss.MaxLOD = 15;

    OVR_ASSERT(!Sampler); // Expected to be null on the way in.
    Sampler = nullptr; // Prevents a potential leak on the next line.
    hr = Device->CreateSamplerState(&ss, &Sampler.GetRawRef());
    OVR_D3D_CHECK_RET_FALSE(hr);
    // OVR_D3D_TAG_OBJECT();  Seems to already have a name.

    // Swap to our blt state to set it up
    Ptr<ID3DDeviceContextState> existingState;
    Context1->SwapDeviceContextState(BltState, &existingState.GetRawRef());

    Context1->IASetInputLayout(IL);
    Context1->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context1->VSSetShader(VS, nullptr, 0);
    Context1->PSSetShader(PS, nullptr, 0);
    Context1->PSSetSamplers(0, 1, &Sampler.GetRawRef());

    // Swap back
    Context1->SwapDeviceContextState(existingState, nullptr);

    AlreadyInitialized = true;
    return true;
}

bool Blitter::Blt(ID3D11RenderTargetView* dest, ID3D11ShaderResourceView* source)
{
    OVR_ASSERT(AlreadyInitialized);
    if (!AlreadyInitialized)
    {
        return false;
    }

    // Switch to our state
    Ptr<ID3DDeviceContextState> existingState;
    Context1->SwapDeviceContextState(BltState, &existingState.GetRawRef());

    ID3D11RenderTargetView* nullRTVs[] = { nullptr, nullptr, nullptr, nullptr };
    ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr, nullptr };

    Context1->OMSetRenderTargets(_countof(nullRTVs), nullRTVs, nullptr);
    Context1->PSSetShaderResources(0, _countof(nullSRVs), nullSRVs);

    // Set the mirror as the render target
    Context1->OMSetRenderTargets(1, &dest, nullptr);

    Ptr<ID3D11Resource> resource;
    dest->GetResource(&resource.GetRawRef());
    Ptr<ID3D11Texture2D> texture;
    HRESULT hr = resource->QueryInterface(IID_PPV_ARGS(&texture.GetRawRef()));
    OVR_D3D_CHECK_RET_FALSE(hr);

    D3D11_TEXTURE2D_DESC desc = {};
    texture->GetDesc(&desc);
    D3D11_VIEWPORT vp = {};
    vp.Width = (float)desc.Width;
    vp.Height = (float)desc.Height;
    vp.MaxDepth = 1.0f;
    Context1->RSSetViewports(1, &vp);

    Context1->PSSetShaderResources(0, 1, &source);

    static const uint32_t stride = sizeof(BltVertex);
    static const uint32_t offset = 0;
    Context1->IASetVertexBuffers(0, 1, &VB.GetRawRef(), &stride, &offset);

    Context1->Draw(6, 0);

    Context1->OMSetRenderTargets(_countof(nullRTVs), nullRTVs, nullptr);
    Context1->PSSetShaderResources(0, _countof(nullSRVs), nullSRVs);

    // Switch back to app state
    Context1->SwapDeviceContextState(existingState, nullptr);

    return true;
}

}} // namespace OVR::D3DUtil

#endif // OVR_OS_MS
