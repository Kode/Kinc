/************************************************************************************

Filename    :   OVR_Log.cpp
Content     :   Logging support
Created     :   September 19, 2012

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

#include "OVR_Log.h"
#include "OVR_Allocator.h"
#include "OVR_String.h"
#include "OVR_DebugHelp.h"
#include "Util/Util_SystemGUI.h"

namespace OVR {

ovrlog::Channel DefaultChannel("Kernel:Default");

static OVRAssertionHandler sOVRAssertionHandler = OVR::DefaultAssertionHandler;
static intptr_t sOVRAssertionHandlerUserParameter = 0;

OVRAssertionHandler GetAssertionHandler(intptr_t* userParameter) {
  if (userParameter)
    *userParameter = sOVRAssertionHandlerUserParameter;
  return sOVRAssertionHandler;
}

void SetAssertionHandler(OVRAssertionHandler assertionHandler, intptr_t userParameter) {
  sOVRAssertionHandler = assertionHandler;
  sOVRAssertionHandlerUserParameter = userParameter;
}

intptr_t
DefaultAssertionHandler(intptr_t /*userParameter*/, const char* title, const char* message) {
  if (OVRIsDebuggerPresent()) {
    OVR_DEBUG_BREAK;
  } else {
    OVR_UNUSED(title);
    OVR_UNUSED(message);

#if defined(OVR_BUILD_DEBUG)
    if (Allocator::GetInstance()) // The code below currently depends on having a valid Allocator.
    {
      // Print a stack trace of all threads.
      OVR::String s;
      OVR::String threadListOutput;
      static OVR::SymbolLookup symbolLookup;

      s = "Failure: ";
      s += message;

      if (symbolLookup.Initialize() &&
          symbolLookup.ReportThreadCallstack(threadListOutput, 4)) // This '4' is there to skip our
      // internal handling and retrieve
      // starting at the assertion
      // location (our caller) only.
      {
        // threadListOutput has newlines that are merely \n, whereas Windows MessageBox wants \r\n
        // newlines. So we insert \r in front of all \n.
        for (size_t i = 0, iEnd = threadListOutput.GetSize(); i < iEnd; i++) {
          if (threadListOutput[i] == '\n') {
            threadListOutput.Insert("\r", i, 1);
            ++i;
            ++iEnd;
          }
        }

        s += "\r\n\r\n";
        s += threadListOutput;
      }

      OVR::Util::DisplayMessageBox(title, s.ToCStr());
    } else {
      // See above.
      OVR::Util::DisplayMessageBox(title, message);
    }
#else
    OVR::Util::DisplayMessageBox(title, message);
#endif
  }

  return 0;
}

bool IsAutomationRunning() {
#if defined(OVR_OS_WIN32)
  // We use the OS GetEnvironmentVariable function as opposed to getenv, as that
  // is process-wide as opposed to being tied to the current C runtime library.
  return GetEnvironmentVariableW(L"OvrAutomationRunning", NULL, 0) > 0;
#else
  return getenv("OvrAutomationRunning") != nullptr;
#endif
}

} // namespace OVR
