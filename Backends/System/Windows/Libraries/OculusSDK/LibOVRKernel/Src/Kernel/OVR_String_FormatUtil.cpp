/************************************************************************************

Filename    :   OVR_String_FormatUtil.cpp
Content     :   String format functions.
Created     :   February 27, 2013
Notes       :

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

#include "OVR_String.h"
#include "OVR_Log.h"
#include <stdarg.h>

namespace OVR {

void StringBuffer::AppendFormatV(const char* format, va_list argList) {
  char buffer[512];
  char* bufferUsed = buffer;
  char* bufferAllocated = NULL;

  va_list argListSaved;
  va_copy(argListSaved, argList);

  int requiredStrlen = vsnprintf(
      bufferUsed,
      OVR_ARRAY_COUNT(buffer),
      format,
      argListSaved); // The large majority of the time this will succeed.

  if (requiredStrlen >= (int)sizeof(buffer)) // If the initial capacity wasn't enough...
  {
    bufferAllocated = (char*)OVR_ALLOC(sizeof(char) * (requiredStrlen + 1));
    bufferUsed = bufferAllocated;
    if (bufferAllocated) {
      va_end(argListSaved);
      va_copy(argListSaved, argList);
      requiredStrlen = vsnprintf(bufferAllocated, (requiredStrlen + 1), format, argListSaved);
    }
  }

  if (requiredStrlen < 0) // If there was a printf format error...
  {
    bufferUsed = NULL;
  }

  va_end(argListSaved);

  if (bufferUsed)
    AppendString(bufferUsed);

  if (bufferAllocated)
    OVR_FREE(bufferAllocated);
}

void StringBuffer::AppendFormat(const char* format, ...) {
  va_list argList;
  va_start(argList, format);
  AppendFormatV(format, argList);
  va_end(argList);
}
} // namespace OVR
