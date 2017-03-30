/************************************************************************************

Filename    :   Util_Direct3D.cpp
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

#include "Util_Direct3D.h"

#include "Kernel/OVR_Log.h"

namespace OVR { namespace D3DUtil {


bool VerifyHRESULT(const char* file, int line, HRESULT hr)
{
    if (FAILED(hr))
    {
        LogError("D3D function returned fail HRESULT at %s on line %d : %s",
                 file, line, D3DUtil::GetWindowsErrorString(hr).ToCStr());
        OVR_ASSERT(false);
        return false;
    }

    return true;
}

String GetWindowsErrorString(HRESULT hr)
{
    wchar_t* errorText = nullptr;

    DWORD slen = FormatMessageW(
        // use system message tables to retrieve error text
        FORMAT_MESSAGE_FROM_SYSTEM
        // allocate buffer on local heap for error text
        | FORMAT_MESSAGE_ALLOCATE_BUFFER
        // Important! will fail otherwise, since we're not 
        // (and CANNOT) pass insertion parameters
        | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&errorText,  // output, allocated via LocalAlloc (free with LocalFree) 
        256, // minimum size for output buffer
        nullptr);   // arguments - see note 

    char formatStr[512];
    snprintf(formatStr, sizeof(formatStr), "[Code=%x = %d]", hr, hr);

    String retStr = formatStr;

    if (slen > 0 && errorText)
    {
        retStr += " ";
        retStr += errorText;

        // release memory allocated by FormatMessage()
        LocalFree(errorText);
    }

    return retStr;
}

void LogD3DCompileError(HRESULT hr, ID3DBlob* blob)
{
    if (FAILED(hr))
    {
        char* errStr = (char*)blob->GetBufferPointer();
        SIZE_T len = blob->GetBufferSize();

        if (errStr && len > 0)
        {
            LogError("Error compiling shader: %s", errStr);
        }
    }
}


}} // namespace OVR::D3DUtil
