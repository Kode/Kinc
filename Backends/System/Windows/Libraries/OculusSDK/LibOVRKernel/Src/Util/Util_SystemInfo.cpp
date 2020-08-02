/************************************************************************************

Filename    :   Util_SystemInfo.cpp
Content     :   Various operations to get information about the system
Created     :   September 26, 2014
Author      :   Kevin Jenkins

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

#include "Util_SystemInfo.h"
#include "Kernel/OVR_Timer.h"
#include "Kernel/OVR_Threads.h"
#include "Kernel/OVR_Log.h"
#include "Kernel/OVR_Array.h"
#include "Kernel/OVR_System.h"
#include "Kernel/OVR_Error.h"
#include <locale>
#include <codecvt>

#if defined(OVR_OS_LINUX)
#include <sys/utsname.h>
#endif

// Includes used for GetBaseOVRPath()
#ifdef OVR_OS_WIN32
#include "Kernel/OVR_Win32_IncludeWindows.h"
#include <Shlobj.h>
#include <Shlwapi.h>
#include <wtsapi32.h>
#include <Psapi.h>
#include <pdh.h>
#include <pdhMsg.h>
#include <perflib.h>
#include <intrin.h>

#pragma comment(lib, "Shlwapi") // PathFileExistsW
#pragma comment(lib, "Wtsapi32.lib") // WTSQuerySessionInformation
#pragma comment(lib, "pdh.lib") // PDH
#elif defined(OVR_OS_MS) // Other Microsoft OSs
// Nothing, thanks.
#else
#include <dirent.h>
#include <sys/stat.h>

#ifdef OVR_OS_LINUX
#include <unistd.h>
#include <pwd.h>
#elif defined(OVR_OS_MAC)
#include <libproc.h>
#endif

#endif

namespace OVR {
namespace Util {

// From http://blogs.msdn.com/b/oldnewthing/archive/2005/02/01/364563.aspx
#if defined(OVR_OS_WIN64) || defined(OVR_OS_WIN32)

#pragma comment(lib, "version.lib")

typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

bool Is64BitWindows() {
#if defined(_WIN64)
  return TRUE; // 64-bit programs run only on Win64
#elif defined(_WIN32)
  // 32-bit programs run on both 32-bit and 64-bit Windows
  // so must sniff
  BOOL f64 = FALSE;
  LPFN_ISWOW64PROCESS fnIsWow64Process;

  fnIsWow64Process =
      (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process");
  if (NULL != fnIsWow64Process) {
    return fnIsWow64Process(GetCurrentProcess(), &f64) && f64;
  }
  return FALSE;
#else
  return FALSE; // Win64 does not support Win16
#endif
}
#endif

const char* OSAsString() {
#if defined(OVR_OS_IPHONE)
  return "IPhone";
#elif defined(OVR_OS_DARWIN)
  return "Darwin";
#elif defined(OVR_OS_MAC)
  return "Mac";
#elif defined(OVR_OS_BSD)
  return "BSD";
#elif defined(OVR_OS_WIN64) || defined(OVR_OS_WIN32)
  if (Is64BitWindows())
    return "Win64";
  else
    return "Win32";
#elif defined(OVR_OS_ANDROID)
  return "Android";
#elif defined(OVR_OS_LINUX)
  return "Linux";
#elif defined(OVR_OS_BSD)
  return "BSD";
#else
  return "Other";
#endif
}

uint64_t GetGuidInt() {
  uint64_t g = Timer::GetTicksNanos();

  uint64_t lastTime, thisTime;
  int j;
  // Sleep a small random time, then use the last 4 bits as a source of randomness
  for (j = 0; j < 8; j++) {
    lastTime = Timer::GetTicksNanos();
    Thread::MSleep(1);
    // Note this does not actually sleep for "only" 1 millisecond
    // necessarily.  Since we do not call timeBeginPeriod(1) explicitly
    // before invoking this function it may be sleeping for 10+ milliseconds.
    thisTime = Timer::GetTicksNanos();
    uint64_t diff = thisTime - lastTime;
    unsigned int diff4Bits = (unsigned int)(diff & 15);
    diff4Bits <<= 32 - 4;
    diff4Bits >>= j * 4;
    ((char*)&g)[j] ^= diff4Bits;
  }

  return g;
}

String GetGuidString() {
  uint64_t guid = GetGuidInt();

  char buff[64];
#if defined(OVR_CC_MSVC)
  snprintf(buff, sizeof(buff), "%I64u", guid);
#else
  snprintf(buff, sizeof(buff), "%llu", (unsigned long long)guid);
#endif
  return String(buff);
}

const char* GetProcessInfo() {
#if defined(OVR_CPU_X86_64)
  return "64 bit";
#elif defined(OVR_CPU_X86)
  return "32 bit";
#else
  return "TODO";
#endif
}

#ifdef OVR_OS_WIN32

String OSVersionAsString() {
  return GetSystemFileVersionStringW(L"\\kernel32.dll");
}
String GetSystemFileVersionStringW(const wchar_t filePath[MAX_PATH]) {
  wchar_t strFilePath[MAX_PATH]; // Local variable
  UINT sysDirLen = GetSystemDirectoryW(strFilePath, ARRAYSIZE(strFilePath));
  if (sysDirLen != 0) {
    OVR_wcscat(strFilePath, MAX_PATH, filePath);
    return GetFileVersionStringW(strFilePath);
  } else {
    return "GetSystemDirectoryW failed";
  }
}

// See
// http://stackoverflow.com/questions/940707/how-do-i-programatically-get-the-version-of-a-dll-or-exe-file
String GetFileVersionStringW(const wchar_t filePath[MAX_PATH]) {
  String result;

  DWORD dwSize = GetFileVersionInfoSizeW(filePath, NULL);
  if (dwSize == 0) {
    OVR_DEBUG_LOG(
        ("Error in GetFileVersionInfoSizeW: %d (for %s)",
         GetLastError(),
         String(filePath).ToCStr()));
    result = String(filePath) + " not found";
  } else {
    auto pVersionInfo = std::make_unique<BYTE[]>(dwSize);
    if (!pVersionInfo) {
      OVR_DEBUG_LOG(("Out of memory allocating %d bytes (for %s)", dwSize, filePath));
      result = "Out of memory";
    } else {
      if (!GetFileVersionInfoW(filePath, 0, dwSize, pVersionInfo.get())) {
        OVR_DEBUG_LOG((
            "Error in GetFileVersionInfo: %d (for %s)", GetLastError(), String(filePath).ToCStr()));
        result = "Cannot get version info";
      } else {
        VS_FIXEDFILEINFO* pFileInfo = NULL;
        UINT pLenFileInfo = 0;

        static HMODULE library;
        static std::once_flag once;
        std::call_once(once, [&]() { library = LoadLibraryW(L"version.dll"); });

        decltype(&::VerQueryValueW) const call = reinterpret_cast<decltype(&::VerQueryValueW)>(
            GetProcAddress(library, "VerQueryValueW"));

        if (!call(pVersionInfo.get(), L"\\", (LPVOID*)&pFileInfo, &pLenFileInfo)) {
          OVR_DEBUG_LOG(
              ("Error in VerQueryValueW: %d (for %s)", GetLastError(), String(filePath).ToCStr()));
          result = "File has no version info";
        } else {
          int major = (pFileInfo->dwFileVersionMS >> 16) & 0xffff;
          int minor = (pFileInfo->dwFileVersionMS) & 0xffff;
          int hotfix = (pFileInfo->dwFileVersionLS >> 16) & 0xffff;
          int other = (pFileInfo->dwFileVersionLS) & 0xffff;

          char str[128];
          snprintf(str, 128, "%d.%d.%d.%d", major, minor, hotfix, other);

          result = str;
        }
      }
    }
  }

  return result;
}

String GetCameraDriverVersion() {
  return GetSystemFileVersionStringW(L"\\drivers\\OCUSBVID.sys");
}

// From http://stackoverflow.com/questions/9524309/enumdisplaydevices-function-not-working-for-me
void GetGraphicsCardList(Array<String>& gpus) {
  gpus.Clear();
  DISPLAY_DEVICEW dd;
  dd.cb = sizeof(dd);

  DWORD deviceNum = 0;
  while (EnumDisplayDevicesW(NULL, deviceNum, &dd, 0)) {
    if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
      gpus.PushBack(String(dd.DeviceString));
    deviceNum++;
  }
}

String GetProcessorInfo() {
  char brand[0x40] = {};
  int cpui[4] = {-1};

  __cpuidex(cpui, 0x80000002, 0);

  // unsigned int blocks = cpui[0];
  for (int i = 0; i <= 2; ++i) {
    __cpuidex(cpui, 0x80000002 + i, 0);
    *reinterpret_cast<int*>(brand + i * 16) = cpui[0];
    *reinterpret_cast<int*>(brand + 4 + i * 16) = cpui[1];
    *reinterpret_cast<int*>(brand + 8 + i * 16) = cpui[2];
    *reinterpret_cast<int*>(brand + 12 + i * 16) = cpui[3];
  }
  return String(brand, 0x40);
}

#else

#ifdef OVR_OS_MAC
// use objective c source

// used for driver files
String GetFileVersionString(String /*filePath*/) {
  return String();
}

String GetSystemFileVersionString(String /*filePath*/) {
  return String();
}

String GetDisplayDriverVersion() {
  return String();
}

String GetCameraDriverVersion() {
  return String();
}

#else

String GetDisplayDriverVersion() {
  char info[256] = {0};
  FILE* file = popen("/usr/bin/glxinfo", "r");
  if (file) {
    int status = 0;
    while (status == 0) {
      status = fscanf(file, "%*[^\n]\n"); // Read up till the end of the current line, leaving the
      // file pointer at the beginning of the next line
      // (skipping any leading whitespace).
      OVR_UNUSED(status); // Prevent GCC compiler warning: "ignoring return value of ‘int
      // fscanf(FILE*, const char*, ...)"

      status = fscanf(file, "OpenGL version string: %255[^\n]", info);
    }
    pclose(file);
    if (status == 1) {
      return String(info);
    }
  }
  return String("No graphics driver details found.");
}

String GetCameraDriverVersion() {
  struct utsname kver;
  if (uname(&kver)) {
    return String();
  }
  return String(kver.release);
}

void GetGraphicsCardList(OVR::Array<OVR::String>& gpus) {
  gpus.Clear();

  char info[256] = {0};
  FILE* file = popen("/usr/bin/lspci", "r");
  if (file) {
    int status = 0;
    while (status >= 0) {
      status = fscanf(file, "%*[^\n]\n"); // Read up till the end of the current line, leaving the
      // file pointer at the beginning of the next line
      // (skipping any leading whitespace).
      OVR_UNUSED(status); // Prevent GCC compiler warning: "ignoring return value of ‘int
      // fscanf(FILE*, const char*, ...)"

      status = fscanf(file, "%*[^ ] VGA compatible controller: %255[^\n]", info);
      if (status == 1) {
        gpus.PushBack(String(info));
      }
    }
    pclose(file);
  }
  if (gpus.GetSizeI() <= 0) {
    gpus.PushBack(String("No video card details found."));
  }
}

String OSVersionAsString() {
  char info[256] = {0};
  FILE* file = fopen("/etc/issue", "r");
  if (file) {
    int status = fscanf(file, "%255[^\n\\]", info);
    fclose(file);
    if (status == 1) {
      return String(info);
    }
  }
  return String("No OS version details found.");
}

String GetProcessorInfo() {
  char info[256] = {0};
  FILE* file = fopen("/proc/cpuinfo", "r");
  if (file) {
    int status = 0;
    while (status == 0) {
      status = fscanf(file, "%*[^\n]\n"); // Read up till the end of the current line, leaving the
      // file pointer at the beginning of the next line
      // (skipping any leading whitespace).
      OVR_UNUSED(status); // Prevent GCC compiler warning: "ignoring return value of ‘int
      // fscanf(FILE*, const char*, ...)"

      status = fscanf(file, "model name : %255[^\n]", info);
    }
    fclose(file);
    if (status == 1) {
      return String(info);
    }
  }
  return String("No processor details found.");
}
#endif // OVR_OS_MAC
#endif // WIN32

std::string GetProcessPath(pid_t processId, bool fileNameOnly, bool enableErrorResults) {
  std::string result;

  if (processId == OVR_INVALID_PID) {
    if (enableErrorResults) {
      result = "(invalid PID)";
    }
  }

#if defined(OVR_OS_WIN32)
  ScopedProcessHANDLE processHandle(
      OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId));

  if (processHandle.IsValid()) {
    result = GetProcessPath(processHandle.Get(), fileNameOnly, enableErrorResults);
  } else if (enableErrorResults) {
    result = "(OpenProcess failure)";
  }

#elif defined(OVR_OS_LINUX)
  char procPath[1024];
  char linkPath[64];
  snprintf(linkPath, OVR_ARRAY_COUNT(linkPath), "/proc/%d/exe", processId);

  int readResult = readlink(linkPath, procPath, OVR_ARRAY_COUNT(procPath));

  if ((readResult != -1) || (readResult >= int(OVR_ARRAY_COUNT(procPath)))) {
    // If the file was deleted, its name will have "(deleted)" after it, which we may want to deal
    // with.
    procPath[result] = '\0';
    OVR_UNUSED(fileNameOnly); // To do.
  } else
    procPath[0] = '\0';
  result = procPath;

#elif defined(OVR_OS_MAC)
  char procPath[PROC_PIDPATHINFO_MAXSIZE];
  int ret = proc_pidpath(processId, procPath, sizeof(procPath));
  if (ret <= 0) {
    OVR_DEBUG_LOG(("Unable to lookup PID: %d -- %s", processId, strerror(errno)));
    procPath[0] = '\0';
    OVR_UNUSED(fileNameOnly); // To do.
  }
  result = procPath;
#endif

  return result;
}

#if defined(_WIN32)
std::string GetProcessPath(HANDLE processHandle, bool fileNameOnly, bool enableErrorResults) {
  std::string result;
  WCHAR procPathW[MAX_PATH];
  DWORD processNameLength = OVR_ARRAY_COUNT(procPathW);

  if (QueryFullProcessImageNameW(processHandle, 0, procPathW, &processNameLength) != 0) {
    result = OVR::UCSStringToUTF8String(procPathW);

    if (fileNameOnly) {
      char fileName[_MAX_FNAME];
      fileName[0] = '\0';
      char fileExtension[_MAX_EXT];
      fileExtension[0] = '\0';

      if (_splitpath_s(
              result.c_str(),
              nullptr,
              0,
              nullptr,
              0,
              fileName,
              _MAX_FNAME,
              fileExtension,
              _MAX_EXT) == 0) {
        result = fileName;
        result += fileExtension;
      } // Else leave it as its full path, though that will in practice rarely or never occur.
    }
  } else if (enableErrorResults) {
    DWORD dwProcessId = GetProcessId(processHandle);
    DWORD dwLastError = GetLastError();
    OVR::String strError;
    if (dwLastError == 31) // Windows reports "A device attached to the system is not functioning."
      strError = "Process has previously exited.";
    else
      OVR::GetSysErrorCodeString(ovrSysErrorCodeType::OS, dwLastError, false, strError);

    char buffer[256];
    snprintf(
        buffer,
        sizeof(buffer),
        "(QueryFullProcessImageNameW failure for process handle 0x%llx, pid %u. Error code: %u: %s)",
        (uint64_t)processHandle,
        static_cast<int>(dwProcessId),
        static_cast<int>(dwLastError),
        strError.ToCStr());
    result = buffer;
  }

  return result;
}
#endif

// Same as GetProcessPath, except scrubs the returned process path string if it's something we have
// deemed
// cannot be reported in our log, usually due to privacy measures we have enacted.
std::string GetLoggableProcessPath(pid_t processId, bool fileNameOnly) {
  std::string processPath = GetProcessPath(processId, fileNameOnly, true);

  // The following is currently disabled, as we decided to not redact side-loaded file names from
  // the server log, and added a ReadMe.txt to the log folder which has a disclaimer about this.
  //
  //#ifdef OVR_INTERNAL_CODE
  //    // For internal builds, we do no scrubbing.
  //#else
  //    // For public builds we scrub the process path if it's side-loaded.
  //    if (IsProcessSideLoaded(GetProcessPath(processId, false)))
  //        processPath = "(side-loaded app)";
  //#endif

  return processPath;
}

#if defined(_WIN32)
// Same as GetProcessPath, except scrubs the returned process path string if it's something we have
// deemed
// cannot be reported in our log, usually due to privacy measures we have enacted.
std::string GetLoggableProcessPath(HANDLE processHandle, bool fileNameOnly) {
  std::string processPath = GetProcessPath(processHandle, fileNameOnly, true);

  // The current design, which may change, is that we allow logging of process file paths in public
  // builds.
  //#ifdef OVR_INTERNAL_CODE
  //    // For internal builds, we do no scrubbing.
  //#else
  //    // For public builds we scrub the process path if it's side-loaded.
  //    if (IsProcessSideLoaded(processPath))
  //        processPath = "(side-loaded app)";
  //#endif

  return processPath;
}
#endif

//-----------------------------------------------------------------------------
// Get a path under BaseOVRPath
// This version returns a std::wstring object.
std::wstring GetOVRPathW(const wchar_t* subPath, bool create_dir) {
#if defined(_WIN32)
  wchar_t fullPath[MAX_PATH];
  SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, NULL, 0, fullPath);
  PathAppendW(fullPath, L"Oculus");
  if (subPath != nullptr) {
    OVR_ASSERT(subPath[0] != '\0' && subPath[0] != L'/' && subPath[0] != L'\\');
    PathAppendW(fullPath, subPath);
  }

  if (create_dir) {
    DWORD attrib = ::GetFileAttributesW(fullPath);
    bool exists = attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY);
    if (!exists) {
      ::CreateDirectoryW(fullPath, NULL);
    }
  }

  return std::wstring(fullPath);
#else
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

  const char* home = getenv("HOME");
  std::string path = home;

#if defined(OVR_OS_MAC)
  path += "/Library/Preferences/Oculus";
#else
  path += "/.config/Oculus";
#endif

  if (subPath != nullptr) {
    OVR_ASSERT(subPath[0] != '\0' && subPath[0] != L'/' && subPath[0] != L'\\');
    std::string narrow = converter.to_bytes(subPath);
    path += "/";
    path += narrow;
  }

  // Create the Oculus directory if it doesn't exist
  if (create_dir) {
    DIR* dir = opendir(path.c_str());
    if (dir == NULL) {
      mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
      closedir(dir);
    }
  }

  return converter.from_bytes(path);
#endif
}
//-----------------------------------------------------------------------------
// Get a path under BaseOVRPath
// This version returns an OVR::String.
String GetOVRPath(const wchar_t* subPath, bool create_dir) {
  return OVR::UCSStringToOVRString(GetOVRPathW(subPath, create_dir));
}
//-----------------------------------------------------------------------------
// Get the (wide) path for local app data.
std::wstring GetBaseOVRPathW(bool create_dir) {
  return GetOVRPathW(nullptr, create_dir);
}
//-----------------------------------------------------------------------------
// Get the path for local app data.
// This version returns an OVR::String.
String GetBaseOVRPath(bool create_dir) {
  return GetOVRPath(nullptr, create_dir);
}

#ifdef _WIN32

// subKey and stringName must point to 256 or greater buffers.
static bool
ParseRegistryPath(const char* path, HKEY& rootKey, wchar_t* subKey, wchar_t* stringName) {
  std::wstring pathW = OVR::UTF8StringToUCSString(path);

  // We need to convert the path to a Windows registry path.
  // "/HKEY_LOCAL_MACHINE/Software/Oculus/ForceGPUDriverVersionAcceptance" needs to convert
  // to HKEY_LOCAL_MACHINE, "Software\\Oculus", "ForceGPUDriverVersionAcceptance".
  std::replace(pathW.begin(), pathW.end(), L'/', L'\\'); // Convert / -> \

  // Find the root key
  if (pathW.find(L"\\HKEY_LOCAL_MACHINE\\") == 0) {
    pathW.erase(0, wcslen(L"\\HKEY_LOCAL_MACHINE\\"));
    rootKey = HKEY_LOCAL_MACHINE;
  } else if (pathW.find(L"\\HKEY_CLASSES_ROOT\\") == 0) {
    pathW.erase(0, wcslen(L"\\HKEY_CLASSES_ROOT\\"));
    rootKey = HKEY_CLASSES_ROOT;
  } else if (pathW.find(L"\\HKEY_CURRENT_CONFIG\\") == 0) {
    pathW.erase(0, wcslen(L"\\HKEY_CURRENT_CONFIG\\"));
    rootKey = HKEY_CURRENT_CONFIG;
  } else if (pathW.find(L"\\HKEY_CURRENT_USER\\") == 0) {
    pathW.erase(0, wcslen(L"\\HKEY_CURRENT_USER\\"));
    rootKey = HKEY_CURRENT_USER;
  } else if (pathW.find(L"\\HKEY_USERS\\") == 0) {
    pathW.erase(0, wcslen(L"\\HKEY_USERS\\"));
    rootKey = HKEY_USERS;
  } else {
    rootKey = 0;
    subKey[0] = L'\0';
    stringName[0] = L'\0';
    return false;
  }

  // pathW now looks like: "Software\\Oculus\\ForceGPUDriverVersionAcceptance"

  size_t lastSeparator = pathW.rfind('\\');
  if ((lastSeparator == std::wstring::npos) || ((pathW.length() - lastSeparator) > 256))
    return false;

  OVR_strlcpy(stringName, &pathW[lastSeparator + 1], 256);
  pathW.erase(lastSeparator, pathW.length() - lastSeparator);

  // stringName now looks like: "ForceGPUDriverVersionAcceptance"
  // pathW now looks like: "Software\\Oculus"

  if ((pathW.length()) >= 256)
    return false;

  OVR_strlcpy(subKey, pathW.c_str(), 256);

  return true;
}

#endif // _WIN32

////////////////////////////////////////////////////////////////////////////////////////////////////
// SettingsManager
////////////////////////////////////////////////////////////////////////////////////////////////////

#if (__cplusplus >= 201103L) || (defined(_MSC_VER) && (_MSC_VER >= 1800)) // C++11 required

SettingsManager DefaultSettingsManager;

#ifdef _WIN32
SettingsManager::Win32RegistryAnyValue::Win32RegistryAnyValue()
    : type(REG_NONE), binaryData(), dwordData(), qwordData(), stringData(), stringArrayData() {}
#endif

void SettingsManager::AddAlternativeLocation(const char* path) {
#ifdef _WIN32
  auto it =
      std::find(RegistryAlternativeLocations.begin(), RegistryAlternativeLocations.end(), path);
  if (it == RegistryAlternativeLocations.end())
    RegistryAlternativeLocations.insert(path);
#else
  (void)path;
#endif
}

void SettingsManager::RemoveAlternativeLocation(const char* path) {
#ifdef _WIN32
  auto it = RegistryAlternativeLocations.find(path);
  if (it != RegistryAlternativeLocations.end())
    RegistryAlternativeLocations.erase(it);
#else
  (void)path;
#endif
}

SettingsManager::StringSet SettingsManager::GetAlternativeLocations() const {
#ifdef _WIN32
  return RegistryAlternativeLocations;
#else
  return StringSet();
#endif
}

void SettingsManager::SetDefaultAlternativeLocations() {
#ifdef _WIN32
  // The following is disabled because we are using this are our default.
  // RegistryAlternativeLocations.push_back("/HKEY_LOCAL_MACHINE/SOFTWARE/Oculus/");
  RegistryAlternativeLocations.insert("/HKEY_CURRENT_USER/SOFTWARE/Oculus/");
  RegistryAlternativeLocations.insert("/HKEY_LOCAL_MACHINE/SOFTWARE/Oculus VR, LLC/Oculus/");
  RegistryAlternativeLocations.insert("/HKEY_LOCAL_MACHINE/SOFTWARE/Oculus VR, LLC/LibOVR/");
  RegistryAlternativeLocations.insert(
      "/HKEY_LOCAL_MACHINE/SOFTWARE/Wow6432Node/Oculus VR, LLC/Oculus/");
  RegistryAlternativeLocations.insert(
      "/HKEY_LOCAL_MACHINE/SOFTWARE/Wow6432Node/Oculus VR, LLC/Oculus/Config/");
  RegistryAlternativeLocations.insert("/HKEY_CURRENT_USER/SOFTWARE/Oculus/Dash/");
#endif
}

#ifdef _WIN32

bool SettingsManager::Win32ReadAnyRegistryValue(
    DWORD dwType,
    uint8_t* data,
    size_t dataSize,
    Win32RegistryAnyValue& anyValue) const {
  switch (dwType) {
    case REG_BINARY:
      anyValue.type = dwType;
      anyValue.binaryData.assign(data, data + dataSize);
      break;

    case REG_DWORD:
      anyValue.type = dwType;
      anyValue.dwordData = *reinterpret_cast<uint32_t*>(data);
      break;

    case REG_QWORD:
      anyValue.type = dwType;
      anyValue.qwordData = *reinterpret_cast<uint64_t*>(data);
      break;

    case REG_SZ: {
      case REG_EXPAND_SZ:
        anyValue.type = dwType;
        std::wstring stringW(reinterpret_cast<wchar_t*>(data), (dataSize / sizeof(wchar_t)) - 1);
        anyValue.stringData =
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(stringW);
        break;
    }

    case REG_MULTI_SZ: {
      anyValue.type = dwType;

      for (wchar_t* p = reinterpret_cast<wchar_t*>(data); *p; p += (wcslen(p) + 1)) {
        std::wstring stringW(p);
        std::string string8 =
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(stringW);
        anyValue.stringArrayData.emplace_back(string8);
      }
      break;
    }

    default:
      // Unsupported type. Leave anyValue.type as REG_NONE.
      return false;
  }

  return true;
}

bool SettingsManager::Win32ReadAnyRegistryValue(
    const char* path,
    Win32RegistryAnyValue& anyValue,
    int options) const {
  bool success = false;
  HKEY rootKey; // e.g. HKEY_CURRENT_USER/
  wchar_t subKey[256]; // e.g. Software/Oculus
  wchar_t stringName[256]; // e.g. SomeValue
  std::string fullPath; // Used in case path is relative.

  anyValue.type = REG_NONE;

  if (path[0] != '/') { // If this is a path relative to the default... make a full path.
    fullPath = defaultLocation + path;
    path = fullPath.c_str();
  }

  if (!ParseRegistryPath(path, rootKey, subKey, stringName))
    return false;

  int64_t iEnd =
      ((options & OptionIgnoreAlternativeLocations) ? 0
                                                    : (int64_t)RegistryAlternativeLocations.size());

  // We do a loop starting with -1 because we are reading from two locations: the input path and
  // an array of alternative locations. -1 is for the input path and 0+ is the alternative
  // locations.
  for (int64_t i = -1; !success && (i < iEnd); ++i) {
    if (i >= 0) { // If the input path didn't find it...
      // try reading from the alternative locations.
      wchar_t stringNameIgnore[256];
      auto it = RegistryAlternativeLocations.begin();
      std::advance(it, (size_t)i); // This is a little slow, but not commonly called.
      ParseRegistryPath(it->c_str(), rootKey, subKey, stringNameIgnore);
    } // Else i == -1 and use the input path as-is.

    HKEY hKey = 0; // Always use the 64 bit registry view. All Oculus runtime software is 64 bit.
    DWORD keyOptions =
        KEY_QUERY_VALUE | ((options & OptionUse32BitDatabase) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
    LONG openResult = RegOpenKeyExW(rootKey, subKey, 0, keyOptions, &hKey);

    if (openResult == ERROR_SUCCESS) {
      DWORD dwType;
      uint8_t data[2048]; // We don't currently have handling for accepting data larger than this.
      DWORD dataSize(2048); // We could add extra logic to support this if needed.

      // RegGetValue is like RegQueryValueEx except that Windows guarantees
      // that string data is properly 0-terminated. RegGetValue returns ERROR_FILE_NOT_FOUND if
      // the given value doesn't exist.
      const DWORD flags = RRF_RT_REG_BINARY | RRF_RT_REG_DWORD | RRF_RT_REG_QWORD | RRF_RT_REG_SZ |
          RRF_RT_REG_MULTI_SZ | RRF_RT_REG_EXPAND_SZ |
          RRF_NOEXPAND; // NOEXPAND needed for RRF_RT_REG_EXPAND_SZ specifically on Win7
      LONG queryResult = RegGetValueW(hKey, nullptr, stringName, flags, &dwType, data, &dataSize);

      if (queryResult == ERROR_SUCCESS) // We don't currently support handling ERROR_MORE_DATA.
        success = Win32ReadAnyRegistryValue(dwType, data, dataSize, anyValue);

      RegCloseKey(hKey);
    }
  }

  return success;
}

#endif // _WIN32

bool SettingsManager::ReadValue(const char* path, std::vector<uint8_t>& value, int options) {
#ifdef _WIN32
  Win32RegistryAnyValue anyValue;

  if (!Win32ReadAnyRegistryValue(path, anyValue, options))
    return false;

  switch (anyValue.type) {
    case REG_BINARY:
      value = anyValue.binaryData;
      break;

    case REG_QWORD:
      value.assign(
          reinterpret_cast<uint8_t*>(&anyValue.qwordData),
          reinterpret_cast<uint8_t*>(&anyValue.qwordData) + sizeof(anyValue.qwordData));
      break;

    case REG_DWORD:
      value.assign(
          reinterpret_cast<uint8_t*>(&anyValue.dwordData),
          reinterpret_cast<uint8_t*>(&anyValue.dwordData) + sizeof(anyValue.dwordData));
      break;

    case REG_SZ:
    case REG_EXPAND_SZ:
      // Copy the trailing '\0'.
      value.assign(
          anyValue.stringData.c_str(),
          anyValue.stringData.c_str() + anyValue.stringData.size() + 1);
      break;

    case REG_MULTI_SZ:
      if (!anyValue.stringArrayData.empty() && !anyValue.stringArrayData[0].empty()) {
        // Copy the trailing '\0'.
        value.assign(
            anyValue.stringArrayData[0].c_str(),
            anyValue.stringArrayData[0].c_str() + anyValue.stringArrayData[0].size() + 1);
      }
      break;
  }

  return true;
#else
  return false;
#endif
}

bool SettingsManager::ReadValue(const char* path, uint8_t* data, size_t& dataSize, int options) {
  std::vector<uint8_t> value;

  if (ReadValue(path, value, options)) {
    if (value.size() <= dataSize) {
      dataSize = std::min(value.size(), dataSize);
      memcpy(data, value.data(), dataSize);
      return true;
    }
  }

  return false;
}

bool SettingsManager::ReadValue(const char* path, double& value, int options) {
#ifdef _WIN32
  Win32RegistryAnyValue anyValue;

  if (!Win32ReadAnyRegistryValue(path, anyValue, options))
    return false;

  try { // Some of the std C++ calls below can throw exceptions.
    switch (anyValue.type) {
      case REG_BINARY:
        // reinterpreting binary as a floating point value is probably not something to rely on.
        if (anyValue.binaryData.size() >= 8)
          value = *reinterpret_cast<double*>(anyValue.binaryData.data());
        else if (anyValue.binaryData.size() >= 4)
          value = *reinterpret_cast<float*>(anyValue.binaryData.data());
        break;

      case REG_QWORD:
        value = (double)anyValue.qwordData;
        break;

      case REG_DWORD:
        value = (double)anyValue.dwordData;
        break;

      case REG_SZ:
      case REG_EXPAND_SZ:
        value = std::stod(anyValue.stringData); // may throw an execption.
        break;

      case REG_MULTI_SZ:
        if (!anyValue.stringArrayData.empty() && !anyValue.stringArrayData[0].empty())
          value = std::stod(anyValue.stringArrayData[0]); // may throw an execption.
        break;
    }
  } catch (...) {
    // Leave value as-is.
  }

  return true;
#else
  return false;
#endif
}

bool SettingsManager::ReadValue(const char* path, float& value, int options) {
  double valueDouble;

  if (!ReadValue(path, valueDouble, options))
    return false;

  value = (float)valueDouble;
  return true;
}

bool SettingsManager::ReadValue(const char* path, uint64_t& value, int options) {
#ifdef _WIN32
  Win32RegistryAnyValue anyValue;

  if (!Win32ReadAnyRegistryValue(path, anyValue, options))
    return false;

  try { // Some of the std C++ calls below can throw exceptions.
    switch (anyValue.type) {
      case REG_BINARY:
        if (anyValue.binaryData.size() >= 8)
          value = *reinterpret_cast<uint64_t*>(anyValue.binaryData.data());
        else if (anyValue.binaryData.size() >= 4)
          value = *reinterpret_cast<uint32_t*>(anyValue.binaryData.data());
        else if (anyValue.binaryData.size() >= 2)
          value = *reinterpret_cast<uint16_t*>(anyValue.binaryData.data());
        else if (anyValue.binaryData.size() >= 1)
          value = *reinterpret_cast<uint8_t*>(anyValue.binaryData.data());
        break;

      case REG_QWORD:
        value = anyValue.qwordData;
        break;

      case REG_DWORD:
        value = anyValue.dwordData;
        break;

      case REG_SZ:
      case REG_EXPAND_SZ:
        value = std::stoull(anyValue.stringData, 0, 0); // may throw an execption.
        break;

      case REG_MULTI_SZ:
        if (!anyValue.stringArrayData.empty() && !anyValue.stringArrayData[0].empty())
          value = std::stoull(anyValue.stringArrayData[0], 0, 0); // may throw an execption.
        break;
    }
  } catch (...) {
    // Leave value as-is.
  }

  return true;
#else
  return false;
#endif
}

bool SettingsManager::ReadValue(const char* path, uint32_t& value, int options) {
  uint64_t value64;

  if (!ReadValue(path, value64, options))
    return false;

  value = (uint32_t)value64;
  return true;
}

bool SettingsManager::ReadValue(const char* path, unsigned long& value, int options) {
#if defined(_WIN32) || (OVR_PTR_SIZE == 4) // unsigned long is always 32 bit for Microsoft.
  return ReadValue(path, (uint32_t&)value, options);
#else
  return ReadValue(path, (uint32_t&)value, options);
#endif
}

bool SettingsManager::ReadValue(const char* path, int64_t& value, int options) {
#ifdef _WIN32
  Win32RegistryAnyValue anyValue;

  if (!Win32ReadAnyRegistryValue(path, anyValue, options))
    return false;

  try { // Some of the std C++ calls below can throw exceptions.
    switch (anyValue.type) {
      case REG_BINARY:
        if (anyValue.binaryData.size() >= 8)
          value = *reinterpret_cast<int64_t*>(anyValue.binaryData.data());
        else if (anyValue.binaryData.size() >= 4)
          value = *reinterpret_cast<int32_t*>(anyValue.binaryData.data());
        else if (anyValue.binaryData.size() >= 2)
          value = *reinterpret_cast<int16_t*>(anyValue.binaryData.data());
        else if (anyValue.binaryData.size() >= 1)
          value = *reinterpret_cast<int8_t*>(anyValue.binaryData.data());
        break;

      case REG_QWORD:
        value = (int64_t)anyValue.qwordData;
        break;

      case REG_DWORD:
        value = (int64_t)((int32_t)anyValue.dwordData);
        break;

      case REG_SZ:
      case REG_EXPAND_SZ:
        value = std::stoll(anyValue.stringData, 0, 0); // may throw an execption.
        break;

      case REG_MULTI_SZ:
        if (!anyValue.stringArrayData.empty() && !anyValue.stringArrayData[0].empty())
          value = std::stoll(anyValue.stringArrayData[0], 0, 0); // may throw an execption.
        break;
    }
  } catch (...) {
    // Leave value as-is.
  }

  return true;
#else
  return false;
#endif
}

bool SettingsManager::ReadValue(const char* path, int32_t& value, int options) {
  int64_t value64;

  if (!ReadValue(path, value64, options))
    return false;

  value = (int32_t)value64;
  return true;
}

bool SettingsManager::ReadValue(const char* path, std::string& value, int options) {
#ifdef _WIN32
  Win32RegistryAnyValue anyValue;

  if (!Win32ReadAnyRegistryValue(path, anyValue, options))
    return false;

  // std::to_string usage below may throw std::bad_alloc, but we don't handle bad_alloc here,
  // but rather we catch it at a higher level, as it's an indicator not of a data format problem,
  // but of a system problem. In practice std::to_string will virtually never throw.
  switch (anyValue.type) {
    case REG_BINARY:
      // When interpreting binary data as a string, how do we interpret the binary data?
      // We write binary under two conditions: if the type is floating point and if the type
      // is natively binary. Between floating point and binary, the only one which we can
      // Possibly convert to a string with any consistency is floating point.
      if (anyValue.binaryData.size() == sizeof(double)) {
        const double* d = reinterpret_cast<double*>(anyValue.binaryData.data());
        value = std::to_string(*d);
      } else {
        // Currently we have no translation for this. We could potentially interpret it as
        // string data, but the problem with that is that we would be inconsistent because
        // we interpret the binary data as floating point if the length is 8 but a string
        // for all other sizes.
        return false;
      }
      break;

    case REG_QWORD:
      value = std::to_string(anyValue.qwordData);
      break;

    case REG_DWORD:
      value = std::to_string(anyValue.dwordData);
      break;

    case REG_SZ:
    case REG_EXPAND_SZ:
      value = anyValue.stringData;
      break;

    case REG_MULTI_SZ:
      if (!anyValue.stringArrayData.empty() && !anyValue.stringArrayData[0].empty())
        value = anyValue.stringArrayData[0];
      break;
  }

  return true;
#else
  return false;
#endif
}

bool SettingsManager::ReadValue(const char* path, std::wstring& value, int options) {
  std::string value8;

  if (!ReadValue(path, value8, options)) // Read as UTF8
    return false;

  value = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(value8);
  return true;
}

bool SettingsManager::ReadValue(const char* path, bool& value, int options) {
  uint64_t result64;

  if (!ReadValue(path, result64, options))
    return false;

  value = (result64 != 0);
  return true;
}

bool SettingsManager::WriteValue(const char* path, bool value, int options) {
#ifdef _WIN32
  // There's no Windows registry bool type, but the defacto standard for
  // supporting it is to use the DWORD type as 0 or 1.
  return WriteValue(path, value ? (uint32_t)1 : (uint32_t)0, options);
#else
  return false;
#endif
}

bool SettingsManager::WriteValue(const char* path, const std::string& str, int options) {
  return WriteValue(path, str.data(), str.size(), options);
}

bool SettingsManager::WriteValue(const char* path, const std::wstring& str, int options) {
  return WriteValue(path, str.data(), str.size(), options);
}

bool SettingsManager::WriteValue(
    const char* path,
    const wchar_t* value,
    size_t valueStrlen,
    int options) {
  // Caller is required to provide a 0-terminated string.
  if (value[valueStrlen] != '\0') {
    OVR_FAIL();
    return false;
  }

#ifdef _WIN32
  bool result = false;
  HKEY rootKey; // e.g. HKEY_CURRENT_USER/
  wchar_t subKey[256]; // e.g. Software/Oculus
  wchar_t stringName[256]; // e.g. SomeValue

  if (!ParseRegistryPath(path, rootKey, subKey, stringName))
    return false;

  // RegCreateKeyExW returns success if the key already exists.
  HKEY hKey = 0; // Always use the 64 bit registry view. All Oculus runtime software is 64 bit.
  DWORD keyOptions = KEY_CREATE_SUB_KEY | KEY_SET_VALUE |
      ((options & OptionUse32BitDatabase) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
  LONG regResult =
      RegCreateKeyExW(rootKey, subKey, 0, nullptr, 0, keyOptions, nullptr, &hKey, nullptr);
  // If regResult fails with a value of 5 (ERROR_ACCESS_DENIED), it's often because the process
  // is trying to write to HLEY_LOCAL_MACHINE and doesn't have admin privileges.

  if (regResult == ERROR_SUCCESS) {
    regResult = RegSetValueExW(
        hKey,
        stringName,
        0,
        REG_SZ,
        reinterpret_cast<BYTE*>(const_cast<wchar_t*>(value)),
        (DWORD)(((valueStrlen + 1) * sizeof(wchar_t))));
    result = (regResult == ERROR_SUCCESS);
    // Note that if the RegSetValueExW failed then we make no attempt to uncreate
    // the key that may have been created above via RegCreateKeyExW.
    RegCloseKey(hKey);
  }

  return result;
#else
  return false;
#endif
}

bool SettingsManager::WriteValue(
    const char* path,
    const char* value,
    size_t valueStrlen,
    int options) {
#ifdef _WIN32
  std::wstring stringW = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(
      value, value + valueStrlen);
  return SettingsManager::WriteValue(path, stringW.c_str(), stringW.size(), options);
#else
  return false;
#endif
}

bool SettingsManager::WriteValue(const char* path, uint32_t value, int options) {
#ifdef _WIN32
  bool result = false;
  HKEY rootKey; // e.g. HKEY_CURRENT_USER/
  wchar_t subKey[256]; // e.g. Software/Oculus
  wchar_t stringName[256]; // e.g. SomeValue
  std::string fullPath; // Used in case path is relative.

  if (path[0] != '/') { // If this is a path relative to the default... make a full path.
    fullPath = defaultLocation + path;
    path = fullPath.c_str();
  }

  if (!ParseRegistryPath(path, rootKey, subKey, stringName))
    return false;

  // RegCreateKeyExW returns success if the key already exists.
  HKEY hKey = 0; // Always use the 64 bit registry view. All Oculus runtime software is 64 bit.
  DWORD keyOptions = KEY_CREATE_SUB_KEY | KEY_SET_VALUE |
      ((options & OptionUse32BitDatabase) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
  LONG regResult =
      RegCreateKeyExW(rootKey, subKey, 0, nullptr, 0, keyOptions, nullptr, &hKey, nullptr);
  // If regResult fails with a value of 5 (ERROR_ACCESS_DENIED), it's often because the process
  // is trying to write to HLEY_LOCAL_MACHINE and doesn't have admin privileges.

  if (regResult == ERROR_SUCCESS) {
    regResult = RegSetValueExW(
        hKey, stringName, 0, REG_DWORD, reinterpret_cast<BYTE*>(&value), sizeof(value));
    result = (regResult == ERROR_SUCCESS);
    // Note that if the RegSetValueExW failed then we make no attempt to uncreate
    // the key that may have been created above via RegCreateKeyExW.
    RegCloseKey(hKey);
  }

  return result;
#else
  return false;
#endif
}

bool SettingsManager::WriteValue(const char* path, int32_t value, int options) {
#ifdef _WIN32
  // If we write it as uint32_t bits, when we read it back as int32_t bits it will be as expected.
  return SettingsManager::WriteValue(path, (uint32_t)value, options);
#else
  return false;
#endif
}

bool SettingsManager::WriteValue(const char* path, uint64_t value, int options) {
#ifdef _WIN32
  bool result = false;
  HKEY rootKey; // e.g. HKEY_CURRENT_USER/
  wchar_t subKey[256]; // e.g. Software/Oculus
  wchar_t stringName[256]; // e.g. SomeValue
  std::string fullPath; // Used in case path is relative.

  if (path[0] != '/') { // If this is a path relative to the default... make a full path.
    fullPath = defaultLocation + path;
    path = fullPath.c_str();
  }

  if (!ParseRegistryPath(path, rootKey, subKey, stringName))
    return false;

  // RegCreateKeyExW returns success if the key already exists.
  HKEY hKey = 0; // Always use the 64 bit registry view. All Oculus runtime software is 64 bit.
  DWORD keyOptions = KEY_CREATE_SUB_KEY | KEY_SET_VALUE |
      ((options & OptionUse32BitDatabase) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
  LONG regResult =
      RegCreateKeyExW(rootKey, subKey, 0, nullptr, 0, keyOptions, nullptr, &hKey, nullptr);
  // If regResult fails with a value of 5 (ERROR_ACCESS_DENIED), it's often because the process
  // is trying to write to HLEY_LOCAL_MACHINE and doesn't have admin privileges.

  if (regResult == ERROR_SUCCESS) {
    regResult = RegSetValueExW(
        hKey, stringName, 0, REG_QWORD, reinterpret_cast<BYTE*>(&value), sizeof(value));
    result = (regResult == ERROR_SUCCESS);
    // Note that if the RegSetValueExW failed then we make no attempt to uncreate
    // the key that may have been created above via RegCreateKeyExW.
    RegCloseKey(hKey);
  }

  return result;
#else
  return false;
#endif
}

bool SettingsManager::WriteValue(const char* path, int64_t value, int options) {
#ifdef _WIN32
  // If we write it as uint32_t bits, when we read it back as int32_t bits it will be as expected.
  return SettingsManager::WriteValue(path, (uint64_t)value, options);
#else
  return false;
#endif
}

bool SettingsManager::WriteValue(const char* path, double value, int options) {
#ifdef _WIN32
  // There's no explicit Windows registry support for writing floating point data,
  // but the only lossless way to do is is to use REG_BINARY.
  return WriteValue(path, reinterpret_cast<const uint8_t*>(&value), sizeof(value), options);
#else
  return false;
#endif
}

bool SettingsManager::WriteValue(const char* path, float value, int options) {
#ifdef _WIN32
  // There's no explicit Windows registry support for writing floating point data,
  // but the only lossless way to do is is to use REG_BINARY.
  return WriteValue(path, reinterpret_cast<const uint8_t*>(&value), sizeof(value), options);
#else
  return false;
#endif
}

bool SettingsManager::WriteValue(
    const char* path,
    const uint8_t* value,
    size_t valueSize,
    int options) {
#ifdef _WIN32
  bool result = false;
  HKEY rootKey; // e.g. HKEY_CURRENT_USER/
  wchar_t subKey[256]; // e.g. Software/Oculus
  wchar_t stringName[256]; // e.g. SomeValue
  std::string fullPath; // Used in case path is relative.

  if (path[0] != '/') { // If this is a path relative to the default... make a full path.
    fullPath = defaultLocation + path;
    path = fullPath.c_str();
  }

  if (!ParseRegistryPath(path, rootKey, subKey, stringName))
    return false;

  // RegCreateKeyExW returns success if the key already exists.
  HKEY hKey = 0; // Always use the 64 bit registry view. All Oculus runtime software is 64 bit.
  DWORD keyOptions = KEY_CREATE_SUB_KEY | KEY_SET_VALUE |
      ((options & OptionUse32BitDatabase) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);
  LONG regResult =
      RegCreateKeyExW(rootKey, subKey, 0, nullptr, 0, keyOptions, nullptr, &hKey, nullptr);
  // If regResult fails with a value of 5 (ERROR_ACCESS_DENIED), it's often because the process
  // is trying to write to HLEY_LOCAL_MACHINE and doesn't have admin privileges.

  if (regResult == ERROR_SUCCESS) {
    regResult = RegSetValueExW(
        hKey, stringName, 0, REG_BINARY, const_cast<uint8_t*>(value), (DWORD)valueSize);
    result = (regResult == ERROR_SUCCESS);
    // Note that if the RegSetValueExW failed then we make no attempt to uncreate
    // the key that may have been created above via RegCreateKeyExW.
    RegCloseKey(hKey);
  }

  return result;
#else
  return false;
#endif
}

bool SettingsManager::ValueExists(const char* path, int options) const {
#ifdef _WIN32
  Win32RegistryAnyValue anyValue;
  return Win32ReadAnyRegistryValue(path, anyValue, options);
#else
  return false;
#endif
}

bool SettingsManager::DeleteValue(const char* path, bool* wasPresent, int options) {
#ifdef _WIN32
  bool result = true; // True until proven false below.

  if (wasPresent)
    *wasPresent = false;

  std::vector<std::string> registryLocations;

  if ((options & OptionIgnoreAlternativeLocations) == 0)
    registryLocations.insert(
        registryLocations.begin(),
        RegistryAlternativeLocations.begin(),
        RegistryAlternativeLocations.end());

  registryLocations.emplace_back(std::string(path));

  for (const auto& pathStr : registryLocations) {
    HKEY rootKey; // e.g. HKEY_CURRENT_USER/
    wchar_t subKey[256]; // e.g. Software/Oculus
    wchar_t stringName[256]; // e.g. SomeValue
    std::string fullPath; // Used in case path is relative.

    path = pathStr.c_str();

    if (path[0] != '/') { // If this is a path relative to the default... make a full path.
      fullPath = defaultLocation + path;
      path = fullPath.c_str();
    }

    Win32RegistryAnyValue anyValue;
    anyValue.type = REG_NONE;

    if (ParseRegistryPath(path, rootKey, subKey, stringName)) {
      HKEY hKey;
      DWORD keyOptions = KEY_QUERY_VALUE | KEY_SET_VALUE |
          ((options & OptionUse32BitDatabase) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY);

      if (RegOpenKeyExW(rootKey, subKey, 0, keyOptions, &hKey) == ERROR_SUCCESS) {
        // Unfortunately, Windows provides no way to properly tell if a value deletion
        // attempt failed due to the value not existing. The error code from RegDeleteValue
        // can't tell you this. You can call RegQueryValue before calling RegDeleteValue to
        // tell if it pre-existed, but between your call to RegQueryValue and RegDeleteValue
        // another thread may have deleted it and give you a false error return from
        // RegDeleteValue. Our best response to this is to check the value again and if it's
        // mising then we know somebody else deleted it.
        DWORD dwType;

        if (RegGetValueW(hKey, nullptr, stringName, RRF_RT_ANY, &dwType, nullptr, nullptr) ==
            ERROR_SUCCESS) {
          if (wasPresent)
            *wasPresent = true;

          if (RegDeleteValueW(hKey, stringName) != ERROR_SUCCESS) // If failed...
            result =
                (RegGetValueW(hKey, nullptr, stringName, RRF_RT_ANY, &dwType, nullptr, nullptr) !=
                 ERROR_SUCCESS);
          // If result fails with a value of 5 (ERROR_ACCESS_DENIED), it's often because the process
          // is trying to write to HLEY_LOCAL_MACHINE and doesn't have admin privileges.
        }

        RegCloseKey(hKey);
      }
    }
  }

  return result;
#else
  if (wasPresent)
    *wasPresent = false;
  return true;
#endif
}

void SettingsManager::SetDefaultLocation(const char* parentPath) {
  OVR_ASSERT(parentPath && (parentPath[0] == '/' || parentPath[0] == '\0'));
  defaultLocation = parentPath;
}

std::string SettingsManager::GetDefaultLocation() const {
  return defaultLocation;
}

std::string SettingsManager::EnumerateValues(const char* parentPath, int options) const {
  std::string result;

#ifdef _WIN32
  HKEY rootKey; // e.g. HKEY_CURRENT_USER/
  wchar_t subKey[256]; // e.g. Software/Oculus
  wchar_t stringName[256]; // e.g. SomeValue

  if (!parentPath)
    parentPath = defaultLocation.c_str();

  int64_t kEnd =
      ((options & OptionIgnoreAlternativeLocations) ? 0
                                                    : (int64_t)RegistryAlternativeLocations.size());

  // We do a loop starting with -1 because we are reading from two locations: the input path and
  // an array of alternative locations. -1 is for the input path and 0+ is the alternative
  // locations.
  for (int64_t k = -1; k < kEnd; ++k) {
    if (k >= 0) {
      auto it = RegistryAlternativeLocations.begin();
      std::advance(it, (size_t)k); // This is a little slow, but not commonly called.
      parentPath = it->c_str();
    }
    // Assert that parentPath begins and ends with /.
    OVR_ASSERT(parentPath && (parentPath[0] == '/') && (parentPath[strlen(parentPath) - 1] == '/'));

    if (!ParseRegistryPath(parentPath, rootKey, subKey, stringName)) {
      return "";
    }

    HKEY hKey = 0; // Always use the 64 bit registry view. All Oculus runtime software is 64 bit.
    LONG openResult = RegOpenKeyExW(rootKey, subKey, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey);

    if (openResult == ERROR_SUCCESS) {
      LONG regResult = ERROR_SUCCESS;

      for (DWORD j = 0; regResult != ERROR_NO_MORE_ITEMS; ++j) {
        DWORD dwType;
        DWORD stringNameSize = OVR_ARRAY_COUNT(stringName);
        uint8_t data[2048];
        DWORD dataSize(2048);

        regResult =
            RegEnumValueW(hKey, j, stringName, &stringNameSize, nullptr, &dwType, data, &dataSize);

        if (regResult == ERROR_SUCCESS) { // There's also ERROR_MORE_DATA.
          Win32RegistryAnyValue anyValue;

          if (Win32ReadAnyRegistryValue(dwType, data, dataSize, anyValue)) {
            char buffer[256];

            snprintf(buffer, sizeof(buffer), "%s%ls: ", parentPath, stringName);
            result += buffer;

            switch (dwType) {
              case REG_BINARY:
                result += "binary: ";

                for (size_t i = 0, iEnd = std::min<size_t>(64, anyValue.binaryData.size());
                     i < iEnd;
                     ++i) {
                  snprintf(buffer, sizeof(buffer), "%02x ", anyValue.binaryData[i]);
                  result += buffer;
                }

                if (anyValue.binaryData.size() > 64) // If we had to truncate it...
                  result += "...";

                result += "\n";
                break;

              case REG_QWORD:
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "uint64: %llu (%#llx)\n",
                    anyValue.qwordData,
                    anyValue.qwordData);
                result += buffer;
                break;

              case REG_DWORD:
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "uint32: %u (%#x)\n",
                    anyValue.dwordData,
                    anyValue.dwordData);
                result += buffer;
                break;

              case REG_SZ:
              case REG_EXPAND_SZ:
                static_assert(sizeof(buffer) >= 256, "buffer size failure");
                snprintf(
                    buffer,
                    sizeof(buffer),
                    "string: %.250s%s\n",
                    anyValue.stringData.c_str(),
                    anyValue.stringData.size() >= 256 ? "..." : "");
                result += buffer;
                break;

              case REG_MULTI_SZ:
                result += "string array: ";
                for (size_t i = 0; i < std::max<size_t>(8, anyValue.stringArrayData.size()); ++i) {
                  snprintf(
                      buffer,
                      sizeof(buffer),
                      "%s%.250s%s\n",
                      i > 0 ? "              " : "",
                      anyValue.stringArrayData[i].c_str(),
                      anyValue.stringData.size() >= 256 ? "..." : "");
                  result += buffer;
                }
                if (anyValue.stringArrayData.size() >= 256) // If we had to truncate it...
                  result += "...\n";
                break;

              default: {
                OVR_FAIL();
              }
            }
          }
        }
      }
    }
  }
#else
  OVR_FAIL();
#endif

  return result;
}

#endif //#if (__cplusplus >= 201103L) // C++11 required

#ifdef _WIN32

bool GetRegistryDwordW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    DWORD& out,
    RegistryDB registryDB,
    RegistryBase registryBase) {
  HKEY root = (registryBase == RegistryBase::currentUser) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  DWORD dwType = REG_DWORD;
  HKEY hKey = 0;
  DWORD value_length = sizeof(DWORD);

  if ((RegOpenKeyExW(
           root,
           pSubKey,
           0,
           KEY_QUERY_VALUE |
               ((registryDB == RegistryDB::wow64_32) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
           &hKey) != ERROR_SUCCESS) ||
      (RegQueryValueExW(hKey, stringName, NULL, &dwType, (LPBYTE)&out, &value_length) !=
       ERROR_SUCCESS) ||
      (dwType != REG_DWORD)) {
    out = 0;
    RegCloseKey(hKey);
    return false;
  }
  RegCloseKey(hKey);

  return true;
}

// When reading Oculus registry keys, we recognize that the user may have inconsistently
// used a DWORD 1 vs. a string "1", and so we support either when checking booleans.
bool GetRegistryBoolW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    bool defaultValue,
    RegistryDB registryDB,
    RegistryBase registryBase) {
  wchar_t out[MAX_PATH];
  if (GetRegistryStringW(pSubKey, stringName, out, registryDB, registryBase)) {
    return (_wtoi64(out) != 0);
  }

  DWORD dw;
  if (GetRegistryDwordW(pSubKey, stringName, dw, registryDB, registryBase)) {
    return (dw != 0);
  }

  return defaultValue;
}

bool GetRegistryStringW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    wchar_t out[MAX_PATH],
    RegistryDB registryDB,
    RegistryBase registryBase) {
  HKEY root = (registryBase == RegistryBase::currentUser) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  DWORD dwType = REG_SZ;
  HKEY hKey = 0;
  wchar_t value[MAX_PATH + 1]; // +1 because RegQueryValueEx doesn't necessarily 0-terminate.
  DWORD value_length = MAX_PATH;

  if ((RegOpenKeyExW(
           root,
           pSubKey,
           0,
           KEY_QUERY_VALUE |
               ((registryDB == RegistryDB::wow64_32) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
           &hKey) != ERROR_SUCCESS) ||
      (RegQueryValueExW(hKey, stringName, NULL, &dwType, (LPBYTE)&value, &value_length) !=
       ERROR_SUCCESS) ||
      (dwType != REG_SZ)) {
    out[0] = L'\0';
    RegCloseKey(hKey);
    return false;
  }
  RegCloseKey(hKey);

  value[value_length] = L'\0';
  wcscpy_s(out, MAX_PATH, value);
  return true;
}

bool SetRegistryDwordW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    DWORD newValue,
    RegistryDB registryDB,
    RegistryBase registryBase) {
  HKEY root = (registryBase == RegistryBase::currentUser) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  HKEY hKey = 0;

  if ((RegCreateKeyExW(
           root,
           pSubKey,
           0,
           nullptr,
           0,
           KEY_CREATE_SUB_KEY | KEY_SET_VALUE |
               ((registryDB == RegistryDB::wow64_32) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
           nullptr,
           &hKey,
           nullptr) != ERROR_SUCCESS) ||
      (RegSetValueExW(
           hKey, stringName, 0, REG_DWORD, reinterpret_cast<BYTE*>(&newValue), sizeof(newValue)) !=
       ERROR_SUCCESS)) {
    RegCloseKey(hKey);
    return false;
  }
  RegCloseKey(hKey);
  return true;
}

bool DeleteRegistryValue(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    RegistryDB registryDB,
    RegistryBase registryBase) {
  HKEY root = (registryBase == RegistryBase::currentUser) ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  HKEY hKey = 0;

  if (RegOpenKeyExW(
          root,
          pSubKey,
          0,
          KEY_ALL_ACCESS |
              ((registryDB == RegistryDB::wow64_32) ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
          &hKey) != ERROR_SUCCESS) {
    RegCloseKey(hKey);
    return false;
  }
  bool result = (RegDeleteValueW(hKey, stringName) == ERROR_SUCCESS);

  RegCloseKey(hKey);
  return result;
}

String GetMachineTags() {
  // We check both HKLM and HKCU for machine tags and append HKCU tags onto
  // HKLM tags. The tag list is comma separated.
  std::wstring allTags;
  std::wstring tempStr;

  if (DefaultSettingsManager.ReadValue(
          "/HKEY_LOCAL_MACHINE/Software/Oculus/MachineTags",
          tempStr,
          SettingsManager::OptionIgnoreAlternativeLocations))
    allTags = tempStr;

  if (DefaultSettingsManager.ReadValue(
          "/HKEY_CURRENT_USER/Software/Oculus/MachineTags",
          tempStr,
          SettingsManager::OptionIgnoreAlternativeLocations)) {
    if (!allTags.empty())
      allTags += L",";
    allTags += tempStr;
  }

  return OVR::String(OVR::UCSStringToUTF8String(allTags).c_str());
}

std::wstring GetOVRRuntimePathW() {
  std::wstring result;

  // HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Oculus VR, LLC\Oculus\Base
  if (DefaultSettingsManager.ReadValue(
          "/HKEY_LOCAL_MACHINE/SOFTWARE/Oculus VR, LLC/Oculus/Base",
          result,
          SettingsManager::OptionUse32BitDatabase)) {
    // Default case
  } else {
    // If our registry value gets corrupted we should attempt to use
    // a sensible value.
    result = L"C:\\Program Files\\Oculus\\";
  }

  // At this point, runtimePath is usually "C:\Program Files\Oculus\", so append what we need to
  // it.
  result += L"Support\\oculus-runtime";

  return result;
}
#endif // _WIN32

#ifdef OVR_UTIL_STD_FILESYSTEM_AVAILABLE

std::filesystem::path GetCurrentModuleDirectoryPath() {
  namespace fs = std::filesystem;

#ifdef _WIN32
  // Get the module handle to the currently executing code. Can't use NULL or GetModuleHandle(NULL),
  // as that will return the parent exe and not the current module (dll or exe).
  HMODULE hModule = NULL;
  BOOL result = GetModuleHandleExW(
      GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPWSTR)GetCurrentModuleDirectoryPath, &hModule);

  if (result) { // Should always succeed.
    std::array<wchar_t, MAX_PATH> path{};

    if (GetModuleFileNameW(hModule, path.data(), (DWORD)path.size())) { // Should always succeed.
      auto fsPath = fs::path(path.data());
      return fsPath.parent_path();
    }
  }
#endif

  return fs::path();
}
#endif // OVR_UTIL_STD_FILESYSTEM_AVAILABLE

// Returns if the computer is currently locked
bool CheckIsComputerLocked() {
#ifdef OVR_OS_MS
  LPWSTR pBuf = nullptr;
  DWORD bytesReturned = 0;

  if (::WTSQuerySessionInformationW(
          WTS_CURRENT_SERVER_HANDLE,
          WTS_CURRENT_SESSION,
          WTSSessionInfoEx,
          &pBuf,
          &bytesReturned)) {
    if (pBuf && bytesReturned >= sizeof(WTSINFOEX)) {
      WTSINFOEXW* info = (WTSINFOEXW*)pBuf;

      WTSINFOEX_LEVEL1_W* level1 = (WTSINFOEX_LEVEL1_W*)&info->Data;

      bool isLocked = false;

      if (level1->SessionFlags == WTS_SESSIONSTATE_LOCK) {
        isLocked = true;
      } else if (level1->SessionFlags != WTS_SESSIONSTATE_UNLOCK) // if not unlocked, we expect
      // locked
      {
        LogError("Unknown Lock State = %d", (int)level1->SessionFlags);
      }

      // Note: On Windows 7, the locked and unlocked flags are reversed!
      // See: https://msdn.microsoft.com/en-us/library/windows/desktop/ee621019(v=vs.85).aspx
      if (IsAtMostWindowsVersion(WindowsVersion::Windows7_SP1)) {
        isLocked = !isLocked;
      }

      return isLocked;
    } else {
      LogError("Wrong return size from WTSQuerySessionInformation %u", bytesReturned);
    }
    if (pBuf) {
      WTSFreeMemory(pBuf);
    }
  }
#endif // OVR_OS_MS
  return false;
}

#ifdef OVR_OS_MS

static INIT_ONCE OSVersionInitOnce = INIT_ONCE_STATIC_INIT;
static uint32_t OSVersion;
static uint32_t OSBuildNumber;

BOOL CALLBACK VersionCheckInitOnceCallback(PINIT_ONCE, PVOID, PVOID*) {
  typedef NTSTATUS(WINAPI * pfnRtlGetVersion)(PRTL_OSVERSIONINFOEXW lpVersionInformation);

  NTSTATUS status = STATUS_DLL_NOT_FOUND;

  HMODULE hNTDll = LoadLibraryW(L"ntdll.dll");
  OVR_ASSERT(hNTDll);

  if (hNTDll) {
    status = STATUS_ENTRYPOINT_NOT_FOUND;

    pfnRtlGetVersion pRtlGetVersion = (pfnRtlGetVersion)GetProcAddress(hNTDll, "RtlGetVersion");
    OVR_ASSERT(pRtlGetVersion);

    if (pRtlGetVersion) {
      RTL_OSVERSIONINFOEXW OSVersionInfoEx;
      OSVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVersionInfoEx);
      status = pRtlGetVersion(&OSVersionInfoEx);
      OVR_ASSERT(status == 0);

      if (status == 0) {
        OSVersion = OSVersionInfoEx.dwMajorVersion * 100 + OSVersionInfoEx.dwMinorVersion;
        OSBuildNumber = OSVersionInfoEx.dwBuildNumber;
      }
    }

    FreeLibrary(hNTDll);
  }

  if (status != 0) {
    LogError(
        "[VersionCheckInitOnceCallback] Failed to obtain OS version information. 0x%08x\n", status);
  }

  return (status == 0);
}

#endif // OVR_OS_MS

bool IsAtLeastWindowsVersion(WindowsVersion version) {
#ifdef OVR_OS_MS
  if (!InitOnceExecuteOnce(&OSVersionInitOnce, VersionCheckInitOnceCallback, nullptr, nullptr)) {
    OVR_ASSERT(false);
    return false;
  }

  switch (version) {
    case WindowsVersion::Windows10_TH2:
      return (OSVersion > 1000) || (OSVersion == 1000 && OSBuildNumber >= 10586);

    case WindowsVersion::Windows10:
      return (OSVersion >= 1000);

    case WindowsVersion::Windows8_1:
      return (OSVersion >= 603);

    case WindowsVersion::Windows8:
      return (OSVersion >= 602);

    case WindowsVersion::Windows7_SP1:
      return (OSVersion >= 601) && (OSBuildNumber >= 7601);

    default:
      OVR_ASSERT(false); // Forget to add a case for a new OS?
      return false;
  }
#else // OVR_OS_MS
  OVR_UNUSED(version);
  return false;
#endif // OVR_OS_MS
}

bool IsAtMostWindowsVersion(WindowsVersion version) {
#ifdef OVR_OS_MS
  if (!InitOnceExecuteOnce(&OSVersionInitOnce, VersionCheckInitOnceCallback, nullptr, nullptr)) {
    OVR_ASSERT(false);
    return false;
  }

  switch (version) {
    case WindowsVersion::Windows10_TH2:
      return (OSVersion < 1000) || (OSVersion == 1000 && OSBuildNumber <= 10586);

    case WindowsVersion::Windows10:
      return (OSVersion < 1000) || (OSVersion == 1000 && OSBuildNumber == 10000);

    case WindowsVersion::Windows8_1:
      return (OSVersion <= 603);

    case WindowsVersion::Windows8:
      return (OSVersion <= 602);

    case WindowsVersion::Windows7_SP1:
      return (OSVersion < 601) || (OSVersion == 601 && OSBuildNumber <= 7601);

    default:
      OVR_ASSERT(false); // Forget to add a case for a new OS?
      return false;
  }
#else // OVR_OS_MS
  OVR_UNUSED(version);
  return false;
#endif // OVR_OS_MS
}

ProcessMemoryInfo GetCurrentProcessMemoryInfo() {
  ProcessMemoryInfo pmi = {};

#if defined(_WIN32)
  PROCESS_MEMORY_COUNTERS_EX pmce = {};
  pmce.cb = sizeof(pmce);

  // This should always succeed for the current process.
  // We need PROCESS_QUERY_LIMITED_INFORMATION rights if we want to read the info from a different
  // process.
  // This call used nearly 1ms on a sampled computer, and so should be called infrequently.
  if (::GetProcessMemoryInfo(
          GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmce), sizeof(pmce))) {
    pmi.UsedMemory = (uint64_t)pmce.WorkingSetSize; // The set of pages in the virtual address space
    // of the process that are currently resident in
    // physical memory.
  }
#else
// To do: Implement this.
#endif

  return pmi;
}

#if defined(_WIN32)

//-----------------------------------------------------------------------------
// PdhHelper is a helper class for querying perf counters using PDH.
// It is implemented as a singleton and initialized upon the first use.
// Currently, it is only used by GetSystemMemoryInfo() to obtain
// page fault information. There is one RateCounter object within
// PdhHelper for that.
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa373083(v=vs.85).aspx
//
class PdhHelper : public SystemSingletonBase<PdhHelper> {
  OVR_DECLARE_SINGLETON(PdhHelper);

 public:
  // RateCounter encapsulates a single counter that represents rate information.
  struct RateCounter {
    // Requires a query object and counter path.
    RateCounter() : Valid(false), HasLast(false), CounterHandle(0) {}

    ~RateCounter() {
      if (CounterHandle)
        PdhRemoveCounter(CounterHandle);
    }

    // Note this call may take some time to complete (180ms from testing) due to
    // PdhAddCounterW, so be mindful if adding a bunch of counter objects in synchronous
    // or blocking manner.
    void Init(PDH_HQUERY hQuery, LPCWSTR counterPath) {
      if (!hQuery)
        return;

      PDH_STATUS status = PdhAddCounterW(hQuery, counterPath, 0, &CounterHandle);
      if (ERROR_SUCCESS != status)
        return;

      Valid = true;
    }

    // Call this to obtain the latest counter value.
    // Note that PdhHelper::Collect() must be called prior to this call.
    double Query() {
      if (!Valid)
        return 0.0;

      PDH_RAW_COUNTER now;
      DWORD counterType;
      PDH_STATUS status = PdhGetRawCounterValue(CounterHandle, &counterType, &now);
      if (ERROR_SUCCESS == status && PDH_CSTATUS_VALID_DATA == now.CStatus) {
        // Calculate the rate (since this is a rate counter).
        // FirstValue is the instance count. SecondValue is the timestamp.
        double result = (HasLast && (now.SecondValue - Last.SecondValue))
            ? double(now.FirstValue - Last.FirstValue) /
                (double(now.SecondValue - Last.SecondValue) * Timer::GetPerfFrequencyInverse())
            : 0.0;

        Last = now;
        HasLast = true;

        return result;
      }

      return 0.0;
    }

    bool Valid; // Whether this counter object was initialized properly
    bool HasLast; // Whether we have a previous snapshot. Need it to compute rate.
    PDH_RAW_COUNTER Last; // Previous counter values
    PDH_HCOUNTER CounterHandle; // PDH handle
  };

 public:
  virtual void OnThreadDestroy() override {
    CleanUp();
  }

  void CleanUp() {
    if (PdhQuery)
      PdhCloseQuery(PdhQuery);
  }

  // Collects raw counter values for all counters in this query.
  bool Collect() {
    return ERROR_SUCCESS == PdhCollectQueryData(PdhQuery);
  }

 public:
  // Here we define the counters that we are interested in.
  RateCounter CounterPageFault;

 private:
  PDH_HQUERY PdhQuery;
};

PdhHelper::PdhHelper() {
  PDH_STATUS status = PdhOpenQueryW(NULL, 0, &PdhQuery);
  if (ERROR_SUCCESS != status) {
    // If this fails, PdhQuery will be null, and subsequent query will
    // check for this and return default data if we have no query object.
    CleanUp();
  }

  // Initialize the counters
  CounterPageFault.Init(PdhQuery, L"\\Memory\\Page Reads/sec");
}

PdhHelper::~PdhHelper() {}

void PdhHelper::OnSystemDestroy() {
  delete this;
}

#endif // _WIN32

//-----------------------------------------------------------------------------

SystemMemoryInfo GetSystemMemoryInfo() {
  SystemMemoryInfo smi = {};

#if defined(_WIN32)
  PdhHelper* ph = PdhHelper::GetInstance();

  if (ph) {
    // A single Collect call is required for the query object regardless of how many counters
    // we are interested in.
    if (ph->Collect()) {
      smi.PageFault = ph->CounterPageFault.Query();
    } else {
      // Can't get the counter for some reason. Use default.
      smi.PageFault = 0.0;
    }

    PERFORMANCE_INFORMATION pi{};
    pi.cb = sizeof(pi);
    if (GetPerformanceInfo(&pi, sizeof(pi)))
      smi.CommittedTotal = pi.CommitTotal;
  }
#endif

  return smi;
}

static void cpuid(int output[4], int functionNumber) {
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
  __cpuidex(output, functionNumber, 0);
#elif defined(__GNUC__) || defined(__clang__)
  int a, b, c, d;
  __asm("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(functionNumber), "c"(0) :);
  output[0] = a;
  output[1] = b;
  output[2] = c;
  output[3] = d;
#endif
}

CPUInstructionSet GetSupportedCPUInstructionSet(bool* popcntSupported, bool* lzcntSupported) {
  static CPUInstructionSet cpuIS = CPUInstructionSet::Unknown;

  if (cpuIS != CPUInstructionSet::Unknown)
    return cpuIS;

  cpuIS = CPUInstructionSet::Basic;

  int features[4] = {};
  cpuid(features, 0);
  if (features[0] == 0) // If there are no features...
    return cpuIS;

  // Check for lzcnt
  if (lzcntSupported) {
    cpuid(features, 0x80000001);
    *lzcntSupported = ((features[2] & (1 << 5)) == 0);
  }

  // Check for popcnt
  if (popcntSupported) {
    cpuid(features, 1);
    *popcntSupported = ((features[2] & (1 << 23)) == 0);
  }

  cpuid(features, 1);

  if ((features[3] & (1 << 0)) == 0 || // If floating point is not supported...
      (features[3] & (1 << 23)) == 0 || // If MMX is not supported...
      (features[3] & (1 << 15)) == 0 || // If conditional move is not supported...
      (features[3] & (1 << 24)) == 0 || // If FXSAVE is not supported...
      (features[3] & (1 << 25)) == 0) // If SSE is not supported...
  {
    return cpuIS;
  }
  cpuIS = CPUInstructionSet::SEE1;

  if ((features[3] & (1 << 26)) == 0) // If SSE2 is not supported...
    return cpuIS;
  cpuIS = CPUInstructionSet::SSE2;

  if ((features[2] & (1 << 0)) == 0) // If SSE3 is not supported...
    return cpuIS;
  cpuIS = CPUInstructionSet::SSE3;

  if ((features[2] & (1 << 9)) == 0) // If SSSE3 is not supported...
    return cpuIS;
  cpuIS = CPUInstructionSet::SSSE3;

  if ((features[2] & (1 << 19)) == 0) // If SSE4.1 is not supported...
    return cpuIS;
  cpuIS = CPUInstructionSet::SSE41;

  if ((features[2] & (1 << 20)) == 0) // If SSE42 is not supproted...
    return cpuIS;
  cpuIS = CPUInstructionSet::SSE42;

#if defined(_MSC_VER)
  if ((features[2] & (1 << 27)) == 0 || // If OSXSAVE is not supported...
      (_xgetbv(0) & 6) != 6 || // If AVX is not recognized by the OS...
      (features[2] & (1 << 28)) == 0) // If AVX is not supported by the CPU...
  {
    return cpuIS;
  }
#else
// To do: Implement this.
#endif

  cpuIS = CPUInstructionSet::AVX1;

  cpuid(features, 7);
  if ((features[1] & (1 << 5)) == 0) // If AVX2 is not supported...
    return cpuIS;

  cpuIS = CPUInstructionSet::AVX2;
  return cpuIS;
}

//-----------------------------------------------------------------------------
// ModuleInfoLookup
//

OVR::Util::ModuleInfoLookup DefaultModuleInfoLookup;

const OVR::ModuleInfo* ModuleInfoLookup::GetModuleInfoForAddress(uint64_t address) {
  if (ModuleInfoArray.empty()) {
    ModuleInfoArray.resize(256);

    size_t requiredCount = OVR::SymbolLookup::GetModuleInfoArray(
        &ModuleInfoArray[0], ModuleInfoArray.size(), OVR::SymbolLookup::ModuleSortByAddress);
    OVR_ASSERT_AND_UNUSED(requiredCount <= ModuleInfoArray.size(), requiredCount);
  }

  OVR::ModuleInfo
      moduleInfo; // Construct a temporary so std::lower_bound can binary-search find it.
  moduleInfo.baseAddress = address;
  moduleInfo.size = 0;

  auto mi = std::lower_bound(
      ModuleInfoArray.begin(),
      ModuleInfoArray.end(),
      moduleInfo,
      [](const OVR::ModuleInfo& mi1, const OVR::ModuleInfo& mi2) -> bool {
        return ((mi1.baseAddress + mi1.size) < (mi2.baseAddress + mi2.size));
      });

  if (mi != ModuleInfoArray.end()) {
    if ((mi->baseAddress <= address) && (address <= (mi->baseAddress + mi->size)))
      return &*mi;
  }

  return nullptr;
}

const OVR::ModuleInfo& ModuleInfoLookup::GetModuleInfoForCurrentModule() {
  if (CurrentModuleInfo.baseAddress == kMIBaseAddressInvalid) {
    // Use GetInstructionAddress because it happens to be a function we forcibly non-inline.
    const OVR::ModuleInfo* mi = GetModuleInfoForAddress((uintptr_t)GetInstructionAddress);

    if (mi)
      CurrentModuleInfo = *mi;
  }

  return CurrentModuleInfo;
}

bool ModuleInfoLookup::GetAddressIsFromCurrentModule(uint64_t address) {
  const OVR::ModuleInfo* mi = GetModuleInfoForAddress(address);
  const OVR::ModuleInfo& miCurrent = GetModuleInfoForCurrentModule();

  return (mi && (mi->baseAddress == miCurrent.baseAddress));
}

} // namespace Util

} // namespace OVR

#if defined(_WIN32)
OVR_DEFINE_SINGLETON(OVR::Util::PdhHelper);
#endif
