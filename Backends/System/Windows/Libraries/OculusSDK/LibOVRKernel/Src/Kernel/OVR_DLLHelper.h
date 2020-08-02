/************************************************************************************
Filename    :   OVR_DLLHelper.h
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

#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <type_traits>

namespace OVR {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Example usage
//
// Write one of these for each of the modules (DLLs) you want to use functions from.
// The following class can be safely declared at global scope.
//    class Kernel32API {
//    public:
//      decltype(GetModuleFileNameA)* getModuleFileNameA = dllHelper.Load("GetModuleFileNameA");
//
//    protected:
//      DllHelper dllHelper{"Kernel32.dll"};
//    };
//
// Use the class declared above:
//    void main() {
//      Kernel32API kernel32Api;
//
//      if(kernel32Api.getModuleFileNameA)
//        kernel32Api.getModuleFileNameA(NULL, path, sizeof(path));
//    }
///////////////////////////////////////////////////////////////////////////////////////////////////

// DllHelper instances can be declared at global scope in most cases.
class DllHelper {
 public:
  // Example moduleFileName: "kernel32.dll"
  explicit DllHelper(const char* moduleFileName) : moduleHandle(::LoadLibraryA(moduleFileName)) {}

  ~DllHelper() {
    ::FreeLibrary(moduleHandle);
  }

  class ProcPtr {
   public:
    explicit ProcPtr(FARPROC ptr) : procPtr(ptr) {}

    template <typename T, typename = std::enable_if_t<std::is_function_v<T>>>
    operator T*() const {
      return reinterpret_cast<T*>(procPtr);
    }

   private:
    FARPROC procPtr;
  };

  // Example proc name: "GetModuleFileName"
  ProcPtr Load(const char* procName) const {
    return ProcPtr(GetProcAddress(moduleHandle, procName));
  }

 protected:
  HMODULE moduleHandle;
};

} // namespace OVR

#endif // _WIN32
