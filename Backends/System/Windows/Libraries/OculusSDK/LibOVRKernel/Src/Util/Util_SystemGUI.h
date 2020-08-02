/************************************************************************************

Filename    :   Util_SystemGUI.h
Content     :   OS GUI access, usually for diagnostics.
Created     :   October 20, 2014
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

*************************************************************************************/

#ifndef OVR_Util_SystemGUI_h
#define OVR_Util_SystemGUI_h

#include <stdarg.h>

namespace OVR {
namespace Util {

// Displays a modal message box on the default GUI display (not on a VR device).
// The message box interface (e.g. OK button) is not localized.
bool DisplayMessageBox(const char* pTitle, const char* pText);

bool DisplayMessageBoxF(const char* pTitle, const char* pFormat, ...);

bool DisplayMessageBoxVaList(const char* pTitle, const char* pFormat, va_list argList);
} // namespace Util
} // namespace OVR

#endif // OVR_Util_SystemGUI_h
