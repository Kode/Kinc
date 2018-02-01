/************************************************************************************

Filename    :   Util_SystemInfo.cpp
Content     :   Various operations to get information about the system
Created     :   September 26, 2014
Author      :   Kevin Jenkins

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

#include "Util_SystemInfo.h"
#include "Kernel/OVR_Timer.h"
#include "Kernel/OVR_Threads.h"
#include "Kernel/OVR_Log.h"
#include "Kernel/OVR_Array.h"
#include "Kernel/OVR_System.h"
#include "Kernel/OVR_Error.h"

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

static const PWSTR OculusRegKey = L"Software\\Oculus";
static const PWSTR MachineTagRegValueName = L"MachineTags";

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
        if (!VerQueryValueW(pVersionInfo.get(), L"\\", (LPVOID*)&pFileInfo, &pLenFileInfo)) {
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
    OVR::GetSysErrorCodeString(dwLastError, false, strError);

    char buffer[256];
    snprintf(
        buffer,
        sizeof(buffer),
        "(QueryFullProcessImageNameW failure for process handle %llx, pid %u. Error code: %u)",
        (uint64_t)processHandle,
        dwProcessId,
        dwLastError);
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

String GetOVRPath(const wchar_t* subPath, bool create_dir) {
#if defined(_WIN32)
  wchar_t fullPath[MAX_PATH];
  SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, NULL, 0, fullPath);
  PathAppendW(fullPath, L"\\Oculus");
  PathAppendW(fullPath, subPath);

  if (create_dir) {
    DWORD attrib = ::GetFileAttributesW(fullPath);
    bool exists = attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY);
    if (!exists) {
      ::CreateDirectoryW(fullPath, NULL);
    }
  }

  return String(fullPath);
#else
  (void)subPath;
  (void)create_dir;
  OVR_FAIL_M("GetOVRPath: needs to be implemented for this platform.");
  return String("/"); // Need to implement this properly.
#endif
}

//-----------------------------------------------------------------------------
// Get the path for local app data.

String GetBaseOVRPath(bool create_dir) {
#if defined(OVR_OS_WIN32)

  wchar_t path[MAX_PATH];
  ::SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, NULL, 0, path);

  OVR_wcscat(path, MAX_PATH, L"\\Oculus");

  if (create_dir) { // Create the Oculus directory if it doesn't exist
    DWORD attrib = ::GetFileAttributesW(path);
    bool exists = attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY);
    if (!exists) {
      ::CreateDirectoryW(path, NULL);
    }
  }

#elif defined(OVR_OS_MAC)

  const char* home = getenv("HOME");
  String path = home;
  path += "/Library/Preferences/Oculus";

  if (create_dir) { // Create the Oculus directory if it doesn't exist
    DIR* dir = opendir(path);
    if (dir == NULL) {
      mkdir(path.ToCStr(), S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
      closedir(dir);
    }
  }

#else

  const char* home = getenv("HOME");
  String path = home;
  path += "/.config/Oculus";

  if (create_dir) { // Create the Oculus directory if it doesn't exist
    DIR* dir = opendir(path);
    if (dir == NULL) {
      mkdir(path.ToCStr(), S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
      closedir(dir);
    }
  }

#endif

  return String(path);
}

#ifdef OVR_OS_MS
// widechar functions are Windows only for now
bool GetRegistryStringW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    wchar_t out[MAX_PATH],
    bool wow64value,
    bool currentUser) {
  HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  DWORD dwType = REG_SZ;
  HKEY hKey = 0;
  wchar_t value[MAX_PATH + 1]; // +1 because RegQueryValueEx doesn't necessarily 0-terminate.
  DWORD value_length = MAX_PATH;

  if ((RegOpenKeyExW(
           root,
           pSubKey,
           0,
           KEY_QUERY_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
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

bool GetRegistryDwordW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    DWORD& out,
    bool wow64value,
    bool currentUser) {
  HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  DWORD dwType = REG_DWORD;
  HKEY hKey = 0;
  DWORD value_length = sizeof(DWORD);

  if ((RegOpenKeyExW(
           root,
           pSubKey,
           0,
           KEY_QUERY_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
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

bool GetRegistryFloatW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    float& out,
    bool wow64value,
    bool currentUser) {
  out = 0.f;

  wchar_t stringValue[MAX_PATH];
  bool result = GetRegistryStringW(pSubKey, stringName, stringValue, wow64value, currentUser);

  if (result) {
    out = wcstof(stringValue, nullptr);

    if (errno == ERANGE) {
      out = 0.f;
      result = false;
    }
  }

  return result;
}

bool GetRegistryDoubleW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    double& out,
    bool wow64value,
    bool currentUser) {
  out = 0.0;

  wchar_t stringValue[MAX_PATH];
  bool result = GetRegistryStringW(pSubKey, stringName, stringValue, wow64value, currentUser);

  if (result) {
    out = wcstod(stringValue, nullptr);

    if (errno == ERANGE) {
      out = 0.0;
      result = false;
    }
  }

  return result;
}

bool GetRegistryBinaryW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    LPBYTE out,
    DWORD* size,
    bool wow64value,
    bool currentUser) {
  HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  DWORD dwType = REG_BINARY;
  HKEY hKey = 0;

  if ((RegOpenKeyExW(
           root,
           pSubKey,
           0,
           KEY_QUERY_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
           &hKey) != ERROR_SUCCESS) ||
      (RegQueryValueExW(hKey, stringName, NULL, &dwType, out, size) != ERROR_SUCCESS) ||
      (dwType != REG_BINARY)) {
    *out = 0;
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
    bool wow64value,
    bool currentUser) {
  wchar_t out[MAX_PATH];
  if (GetRegistryStringW(pSubKey, stringName, out, wow64value, currentUser)) {
    return (_wtoi64(out) != 0);
  }

  DWORD dw;
  if (GetRegistryDwordW(pSubKey, stringName, dw, wow64value, currentUser)) {
    return (dw != 0);
  }

  return defaultValue;
}

bool SetRegistryBinaryW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    LPBYTE value,
    DWORD size,
    bool wow64value,
    bool currentUser) {
  HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  HKEY hKey = 0;

  if ((RegCreateKeyExW(
           root,
           pSubKey,
           0,
           nullptr,
           0,
           KEY_CREATE_SUB_KEY | KEY_SET_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
           nullptr,
           &hKey,
           nullptr) != ERROR_SUCCESS) ||
      (RegSetValueExW(hKey, stringName, 0, REG_BINARY, value, size) != ERROR_SUCCESS)) {
    RegCloseKey(hKey);
    return false;
  }
  RegCloseKey(hKey);
  return true;
}

bool SetRegistryStringW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    wchar_t in[MAX_PATH],
    size_t length,
    bool wow64value,
    bool currentUser) {
  HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  HKEY hKey = 0;

  if ((RegCreateKeyExW(
           root,
           pSubKey,
           0,
           nullptr,
           0,
           KEY_CREATE_SUB_KEY | KEY_SET_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
           nullptr,
           &hKey,
           nullptr) != ERROR_SUCCESS) ||
      (RegSetValueExW(
           hKey, stringName, 0, REG_SZ, reinterpret_cast<BYTE*>(&in[0]), (DWORD)length) !=
       ERROR_SUCCESS)) {
    RegCloseKey(hKey);
    return false;
  }
  RegCloseKey(hKey);
  return true;
}

bool SetRegistryDwordW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    DWORD newValue,
    bool wow64value,
    bool currentUser) {
  HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  HKEY hKey = 0;

  if ((RegCreateKeyExW(
           root,
           pSubKey,
           0,
           nullptr,
           0,
           KEY_CREATE_SUB_KEY | KEY_SET_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
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
    bool wow64value,
    bool currentUser) {
  HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
  HKEY hKey = 0;

  if (RegOpenKeyExW(
          root,
          pSubKey,
          0,
          KEY_ALL_ACCESS | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY),
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

  wchar_t stringValue[MAX_PATH];

  // HKEY_LOCAL_MACHINE\Software\Oculus\MachineTags SZ <tags>
  bool result = OVR::Util::GetRegistryStringW(
      OculusRegKey, MachineTagRegValueName, stringValue, false, false);
  if (result) {
    allTags = stringValue;
  }

  // HKEY_CURRENT_USER\Software\Oculus\MachineTags SZ <tags>
  result =
      OVR::Util::GetRegistryStringW(OculusRegKey, MachineTagRegValueName, stringValue, false, true);
  if (result) {
    if (!allTags.empty()) {
      allTags += L",";
    }

    allTags += stringValue;
  }

  return String(OVR::UCSStringToUTF8String(allTags).c_str());
}

// Usually this returns C:\Program Files (x86)\Oculus\Support\oculus-runtime
bool GetOVRRuntimePathW(wchar_t runtimePath[MAX_PATH]) {
  if (GetRegistryStringW(
          L"Software\\Oculus VR, LLC\\Oculus", L"Base", runtimePath, Is64BitWindows(), false)) {
    // At this point, runtimePath is usually "C:\Program Files (x86)\Oculus\", so append what we
    // need to it.
    return (OVR_strlcat(runtimePath, L"Support\\oculus-runtime", MAX_PATH) < MAX_PATH);
  }

  runtimePath[0] = 0;
  return false;
}

#endif // OVR_OS_MS

bool GetOVRRuntimePath(String& runtimePath) {
  runtimePath.Clear();

#ifdef OVR_OS_MS
  wchar_t path[MAX_PATH];
  if (GetOVRRuntimePathW(path)) {
    runtimePath = String(path);
    return true;
  }
#else
// mac/linux uses environment variables
#endif

  return false;
}

bool GetDefaultFirmwarePath(String& firmwarePath) {
  if (!GetOVRRuntimePath(firmwarePath)) {
    return false;
  } else {
    firmwarePath + "\\Tools\\FirmwareBundle.json";
    return true;
  }
}

//-----------------------------------------------------------------------------
// GetFirmwarePath
//
// This function searches for likely locations of the firmware.zip file when
// the file path is not specified by the user.

#ifdef OVR_OS_MS

// We search the following paths relative to the executable:
static const wchar_t* FWRelativePaths[] = {L"firmware.zip",
                                           L"../Tools/Firmware/firmware.zip",
                                           L"Firmware/firmware.zip",
                                           L"../Firmware/firmware.zip",
                                           L"../../Firmware/firmware.zip",
                                           L"../../../Firmware/firmware.zip",
                                           L"../../../../Firmware/firmware.zip",
                                           L"../../../../../Firmware/firmware.zip",
                                           L"../../../../../../Firmware/firmware.zip",
                                           L"../../../../../../../Firmware/firmware.zip",
                                           L"../../../../../../../../Firmware/firmware.zip",
                                           L"../oculus-tools/firmware/Firmware.zip"};
static const int RelativePathsCount = OVR_ARRAY_COUNT(FWRelativePaths);

#endif // OVR_OS_MS

#ifdef OVR_OS_MS
static std::wstring GetModulePath() {
  wchar_t wpath[MAX_PATH];
  DWORD len = ::GetModuleFileNameW(nullptr, wpath, MAX_PATH);

  if (len <= 0 || len >= MAX_PATH) {
    return std::wstring();
  }

  return wpath;
}

#endif // OVR_OS_MS
bool GetFirmwarePath(std::wstring* outPath) {
#ifdef OVR_OS_MS

  std::wstring basePath = GetModulePath();

  // Remove trailing slash.
  std::wstring::size_type lastSlashOffset = basePath.find_last_of(L"/\\", std::wstring::npos, 2);
  if (lastSlashOffset == std::string::npos) {
    // Abort if no trailing slash.
    return false;
  }
  basePath = basePath.substr(0, lastSlashOffset + 1);

  // For each relative path to test,
  for (int i = 0; i < RelativePathsCount; ++i) {
    // Concatenate the relative path on the base path (base path contains a trailing slash)
    std::wstring candidatePath = basePath + FWRelativePaths[i];

    // If a file exists at this location,
    if (::PathFileExistsW(candidatePath.c_str())) {
      // Return the path if requested.
      if (outPath) {
        *outPath = candidatePath;
      }

      return true;
    }
  }
#else
  OVR_UNUSED(outPath);
#endif // OVR_OS_MS

  return false;
}

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

#if defined(_WIN32)

static const wchar_t* RFL2RelativePaths[] = {
    L"..\\oculus-tools\\RiftFirmLoad2.exe",
    L"RiftFirmLoad2.exe",
    L"Tools\\RiftFirmLoad2.exe",
    L"..\\..\\..\\..\\..\\..\\.."
    L"\\Tools\\RiftFirmLoad2\\Bin\\RiftFirmLoad2\\Windows\\"
#ifdef OVR_OS_WIN64
    L"x64\\"
#else
    L"Win32\\"
#endif
#ifdef OVR_DEBUG_CODE
    L"Debug\\"
#else
    L"Release\\"
#endif
#if (_MSC_VER == 1900) //(Visual Studio 2015)
    L"VS2015\\"
#elif (_MSC_VER == 1800) //(Visual Studio 2013)
    L"VS2013\\"
#endif
    L"RiftFirmLoad2.exe"};

static const int RFL2RelativePathsCount = OVR_ARRAY_COUNT(RFL2RelativePaths);

#endif // WIN32

bool GetRFL2Path(std::wstring* outPath) {
#ifdef OVR_OS_MS

  std::wstring basePath = GetModulePath();

  // Remove trailing slash.
  std::wstring::size_type lastSlashOffset = basePath.find_last_of(L"/\\", std::wstring::npos, 2);
  if (lastSlashOffset == std::string::npos) {
    // Abort if no trailing slash.
    return false;
  }
  basePath = basePath.substr(0, lastSlashOffset + 1);

  // For each relative path to test,
  for (int i = 0; i < RFL2RelativePathsCount; ++i) {
    // Concatenate the relative path on the base path (base path contains a trailing slash)
    std::wstring candidatePath = basePath + RFL2RelativePaths[i];

    // If a file exists at this location,
    if (::PathFileExistsW(candidatePath.c_str())) {
      // Return the path if requested.
      if (outPath) {
        *outPath = candidatePath;
      }

      return true;
    }
  }
#else
  OVR_UNUSED(outPath);
//#error "FIXME"
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
  PROCESS_MEMORY_COUNTERS_EX pmce = {sizeof(PROCESS_MEMORY_COUNTERS_EX), 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

    PERFORMANCE_INFORMATION pi = {sizeof(pi)};
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

} // namespace Util

} // namespace OVR

#if defined(_WIN32)
OVR_DEFINE_SINGLETON(OVR::Util::PdhHelper);
#endif
