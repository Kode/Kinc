/************************************************************************************

Filename    :   OVR_Log.cpp
Content     :   Logging support
Created     :   September 19, 2012

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

#include "OVR_Log.h"
#include "OVR_Allocator.h"
#include "OVR_String.h"
#include "OVR_DebugHelp.h"
#include "Util/Util_SystemGUI.h"
#include <fstream>

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

// This currently has no multi-thread safety.
intptr_t AssertHandlerDiskFile(intptr_t userParameter, const char* title, const char* message) {
  // We don't have a means to keep state, so we need to have a little state machine here to
  // help us direct how this works.

  static std::ofstream file;
  static std::string filePath;
  const char* userFilePath = (userParameter ? reinterpret_cast<const char*>(userParameter) : "");

  // See if we need to change the file state.
  if (filePath != userFilePath) { // If a file change is occurring...
    file.close(); // Possibly close any existing open file. OK if the file wasn't open.

    filePath = userFilePath;

    if (!filePath.empty()) // If there is a file to open...
      file.open(filePath.c_str(), std::ios::out | std::ios::trunc); // If this fails, we won't know.
  }

  // Write the message if there is a file to write to.
  if (file.is_open()) {
    file.write("Assertion failure\n", strlen("Assertion failure\n"));
    file.write(title, strlen(title));
    file.write("\n", 1);
    file.write(message, strlen(message));
    file.write("\n\n", 1);
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
