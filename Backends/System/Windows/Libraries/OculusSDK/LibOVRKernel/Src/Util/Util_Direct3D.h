/************************************************************************************

Filename    :   Util_Direct3D.h
Content     :   Shared code for Direct3D
Created     :   Oct 14, 2014
Authors     :   Chris Taylor

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

#ifndef OVR_Util_Direct3D_h
#define OVR_Util_Direct3D_h

#if defined(_WIN32)

// Include Windows correctly first before implicitly including it below
#include "Kernel/OVR_Win32_IncludeWindows.h"
#include "Kernel/OVR_String.h"

// Direct3D 11
#include <D3D11Shader.h>
#include <d3dcompiler.h>

#if _MSC_VER >= 1800
// Visual Studio 2013+ support newer D3D/DXGI headers.
#define OVR_D3D11_VER 2
#include <d3d11_2.h>
#define OVR_DXGI_VER 3
#include <dxgi1_3.h> // Used in place of 1.2 for IDXGIFactory2 debug version (when available)
#elif _MSC_VER >= 1700
// Visual Studio 2012+ only supports older D3D/DXGI headers.
#define OVR_D3D11_VER 1
#include <d3d11_1.h>
#else
// Visual Studio 2010+ only supports original D3D/DXGI headers.
#define OVR_D3D11_VER 1
#include <d3d11.h>
#endif

namespace OVR {
namespace D3DUtil {

String GetWindowsErrorString(HRESULT hr);

//-----------------------------------------------------------------------------
// Helper macros for verifying HRESULT values from Direct3D API calls
//
// These will assert on failure in debug mode, and in release or debug mode
// these functions will report the file and line where the error occurred,
// and what the error code was to the log at Error-level.

// Assert on HRESULT failure
bool VerifyHRESULT(const char* file, int line, HRESULT hr);

#define OVR_D3D_CHECK(hr) OVR::D3DUtil::VerifyHRESULT(__FILE__, __LINE__, hr)

// Internal implementation of the OVR_D3D_CHECK_RET family of functions.
#define OVR_D3D_CHECK_RET_IMPL(hr, returnExpression) \
  {                                                  \
    if (!OVR_D3D_CHECK(hr)) {                        \
      returnExpression                               \
    }                                                \
  }

// Returns provided value on failure.
// Example usage:
//     size_t Func() {
//         . . .
//         HRESULT hr = Device->QueryInterface(__uuidof(IDXGIDevice), (void
//         **)&pDXGIDevice.GetRawRef());
//         OVR_D3D_CHECK_RET_VAL(hr, 0);
//         . . .
//     }
#define OVR_D3D_CHECK_RET_VAL(hr, failureValue) OVR_D3D_CHECK_RET_IMPL(hr, return failureValue;)

// Returns void on failure
// Example usage:
//     void Func() {
//         . . .
//         HRESULT hr = Device->QueryInterface(__uuidof(IDXGIDevice), (void
//         **)&pDXGIDevice.GetRawRef());
//         OVR_D3D_CHECK_RET(hr);
//         . . .
//     }
#define OVR_D3D_CHECK_RET(hr) OVR_D3D_CHECK_RET_IMPL(hr, return;)

// Returns false on failure
// Example usage:
//     bool Func() {
//         . . .
//         HRESULT hr = Device->QueryInterface(__uuidof(IDXGIDevice), (void
//         **)&pDXGIDevice.GetRawRef());
//         OVR_D3D_CHECK_RET_FALSE(hr);
//         . . .
//     }
#define OVR_D3D_CHECK_RET_FALSE(hr) OVR_D3D_CHECK_RET_IMPL(hr, return false;)

// Returns nullptr on failure
// Example usage:
//     void* Func() {
//         . . .
//         HRESULT hr = Device->QueryInterface(__uuidof(IDXGIDevice), (void
//         **)&pDXGIDevice.GetRawRef());
//         OVR_D3D_CHECK_RET_NULL(hr);
//         . . .
//     }
#define OVR_D3D_CHECK_RET_NULL(hr) OVR_D3D_CHECK_RET_IMPL(hr, return nullptr;)

// If the result is a failure, it will write the exact compile error to the error log
void LogD3DCompileError(HRESULT hr, ID3DBlob* errorBlob);

#if defined(OVR_BUILD_DEBUG)

// Enable this to track down double-tagging object warnings from D3D.
#define OVR_D3D_TRACK_DOUBLE_TAGGING

#if defined(OVR_D3D_TRACK_DOUBLE_TAGGING)
#define OVR_D3D_CHECK_REUSE(child)               \
  char tagReuseBuffer[1024];                     \
  UINT reuseSize = (UINT)sizeof(tagReuseBuffer); \
  OVR_ASSERT(FAILED(child->GetPrivateData(WKPDID_D3DDebugObjectName, &reuseSize, tagReuseBuffer)));
#else
#define OVR_D3D_CHECK_REUSE(child) (void(0))
#endif

#define OVR_D3D_TAG_OBJECT(child)                                                            \
  if (child) {                                                                               \
    OVR_D3D_CHECK_REUSE(child);                                                              \
    const char* tagName = OVR_STRINGIZE(child) " " __FILE__ "(" OVR_STRINGIZE(__LINE__) ")"; \
    UINT tagDataSize = (UINT)strlen(tagName);                                                \
    HRESULT tagHR = child->SetPrivateData(WKPDID_D3DDebugObjectName, tagDataSize, tagName);  \
    OVR_D3D_CHECK(tagHR);                                                                    \
  }

#else // !Debug:

#define OVR_D3D_TAG_OBJECT(child) (void(0))

#endif // !Debug
} // namespace D3DUtil
} // namespace OVR

#endif // _WIN32

#endif // OVR_Util_Direct3D_h
