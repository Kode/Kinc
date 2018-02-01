/************************************************************************************

Filename    :   Util_SystemInfo.h
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

#ifndef OVR_Util_SystemInfo_h
#define OVR_Util_SystemInfo_h

#include "Kernel/OVR_String.h"
#include "Kernel/OVR_Types.h"
#include "Kernel/OVR_Array.h"

namespace OVR {
namespace Util {
bool Is64BitWindows();
const char* OSAsString();
String OSVersionAsString();
uint64_t GetGuidInt();
String GetGuidString();
const char* GetProcessInfo();
String GetCameraDriverVersion();
void GetGraphicsCardList(OVR::Array<OVR::String>& gpus);
String GetProcessorInfo();

enum WindowsVersion { Windows7_SP1 = 0, Windows8, Windows8_1, Windows10, Windows10_TH2 };

bool IsAtLeastWindowsVersion(WindowsVersion version);
bool IsAtMostWindowsVersion(WindowsVersion version);

// Retrives the root of the Oculus install directory
bool GetOVRRuntimePath(OVR::String& runtimePath);

// This function searches for likely locations of the firmware.zip file
// returns true if the file is found
bool GetFirmwarePath(std::wstring* outPath);

// This function searches for likely locations of the RiftFirmLoad2.exe file
// returns true if the file is found
bool GetRFL2Path(std::wstring* outPath);

// Checks if the computer is currently locked
bool CheckIsComputerLocked();

#ifdef OVR_OS_MS

// Retrives the root of the Oculus install directory
// The returned string is a directory path and does not have a trailing path separator.
// Example return value as of runtime 1.2 (February 2016): L"C:\Program Files
// (x86)\Oculus\Support\oculus-runtime"
// Upon a false return value, the returned runtimePath contents are not defined.
bool GetOVRRuntimePathW(wchar_t runtimePath[MAX_PATH]);

// Returns true if a string-type registry key of the given name is present, else sets out to empty
// and returns false.
// The output string will always be 0-terminated, but may be empty.
// If wow64value is true then KEY_WOW64_32KEY is used in the registry lookup.
// If currentUser is true then HKEY_CURRENT_USER root is used instead of HKEY_LOCAL_MACHINE
bool GetRegistryStringW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    wchar_t out[MAX_PATH],
    bool wow64value = false,
    bool currentUser = false);

// Returns true if a DWORD-type registry key of the given name is present, else sets out to 0 and
// returns false.
// If wow64value is true then KEY_WOW64_32KEY is used in the registry lookup.
// If currentUser is true then HKEY_CURRENT_USER root is used instead of HKEY_LOCAL_MACHINE
bool GetRegistryDwordW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    DWORD& out,
    bool wow64value = false,
    bool currentUser = false);

// Returns true if a string-type registry key of the given name is present and can be read as a
// float, else sets out to 0 and returns false.
// If wow64value is true then KEY_WOW64_32KEY is used in the registry lookup.
// If currentUser is true then HKEY_CURRENT_USER root is used instead of HKEY_LOCAL_MACHINE
bool GetRegistryFloatW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    float& out,
    bool wow64value = false,
    bool currentUser = false);

// Returns true if a string-type registry key of the given name is present and can be read as a
// double, else sets out to 0 and returns false.
// If wow64value is true then KEY_WOW64_32KEY is used in the registry lookup.
// If currentUser is true then HKEY_CURRENT_USER root is used instead of HKEY_LOCAL_MACHINE
bool GetRegistryDoubleW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    double& out,
    bool wow64value = false,
    bool currentUser = false);

// Returns true if a BINARY-type registry key of the given name is present, else sets out to 0 and
// returns false.
// Size must be set to max size of out buffer on way in. Will be set to size actually read into the
// buffer on way out.
// If wow64value is true then KEY_WOW64_32KEY is used in the registry lookup.
// If currentUser is true then HKEY_CURRENT_USER root is used instead of HKEY_LOCAL_MACHINE
bool GetRegistryBinaryW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    LPBYTE out,
    DWORD* size,
    bool wow64value = false,
    bool currentUser = false);

// Returns true if a registry key of the given type is present and can be interpreted as a boolean,
// otherwise
// returns defaultValue. It's not possible to tell from a single call to this function if the given
// registry key
// was present. For Strings, boolean means (atoi(str) != 0). For DWORDs, boolean means (dw != 0).
// If currentUser is true then HKEY_CURRENT_USER root is used instead of HKEY_LOCAL_MACHINE
bool GetRegistryBoolW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    bool defaultValue,
    bool wow64value = false,
    bool currentUser = false);

// Returns true if the value could be successfully written to the registry.
// If wow64value is true then KEY_WOW64_32KEY is used in the registry write.
// If currentUser is true then HKEY_CURRENT_USER root is used instead of HKEY_LOCAL_MACHINE
bool SetRegistryBinaryW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    const LPBYTE value,
    DWORD size,
    bool wow64value = false,
    bool currentUser = false);

// Returns true if the value could be successfully written to the registry.
// If wow64value is true then KEY_WOW64_32KEY is used in the registry write.
// If currentUser is true then HKEY_CURRENT_USER root is used instead of HKEY_LOCAL_MACHINE
bool SetRegistryStringW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    wchar_t in[MAX_PATH],
    size_t length,
    bool wow64value,
    bool currentUser);

// Returns true if the DWORD value could be successfully written to the registry.
// If wow64value is true then KEY_WOW64_32KEY is used in the registry write.
// If currentUser is true then HKEY_CURRENT_USER root is used instead of HKEY_LOCAL_MACHINE
bool SetRegistryDwordW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    DWORD newValue,
    bool wow64value = false,
    bool currentUser = false);

// Returns true if the value could be successfully deleted from the registry.
// If wow64value is true then KEY_WOW64_32KEY is used.
// If currentUser is true then HKEY_CURRENT_USER root is used instead of HKEY_LOCAL_MACHINE
bool DeleteRegistryValue(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    bool wow64value = false,
    bool currentUser = false);

// Returns a string with a comma-separated list of tags for the machine.
// The tags are obtained from the registry.
String GetMachineTags();

String GetFileVersionStringW(const wchar_t filePath[MAX_PATH]);
String GetSystemFileVersionStringW(const wchar_t filePath[MAX_PATH]);

#else
// Mac + Linux equivalants are not implemented.

#endif // OVR_OS_MS

//-----------------------------------------------------------------------------
// Get the path for local app data.

String GetBaseOVRPath(bool create_dir);
String GetOVRPath(const wchar_t* subPath, bool create_dir);

//-----------------------------------------------------------------------------
// Process path information

// Returns the file path for the binary associated with the given runtime process id.
// Returns an empty string if the process id is invalid or could not be read.
// If enableErrorResults is true then failure strings will be written to the returned
// value in parentheses. This is useful for logging.
std::string GetProcessPath(pid_t processId, bool fileNameOnly, bool enableErrorResults);

#if defined(_WIN32)
// Returns the file path for the binary associated with the given runtime process id.
// Returns an empty string if the process handle is invalid or could not be read.
// If enableErrorResults is true then failure strings will be written to the returned
// value in parentheses. This is useful for logging.
std::string GetProcessPath(HANDLE processHandle, bool fileNameOnly, bool enableErrorResults);
#endif

// Returns true if the process was loaded outside of our Oculus store directory.
bool IsProcessSideLoaded(const std::string& processPath);

// Same as GetProcessPath, except scrubs the returned process path string if it's something we have
// deemed
// cannot be reported in our log, usually due to privacy measures we have enacted.
std::string GetLoggableProcessPath(pid_t processId, bool fileNameOnly);

#if defined(_WIN32)
// Same as GetProcessPath, except scrubs the returned process path string if it's something we have
// deemed
// cannot be reported in our log, usually due to privacy measures we have enacted.
std::string GetLoggableProcessPath(HANDLE processHandle, bool fileNameOnly);
#endif

//-----------------------------------------------------------------------------
// Retrieves memory usage info for the current process.
//
// Do not call this function frequently as it takes hundreds of microseconds to
// execute on typical computers.

struct ProcessMemoryInfo {
  uint64_t UsedMemory; // Same as Windows working set size.
  // https://msdn.microsoft.com/en-us/library/windows/desktop/cc441804%28v=vs.85%29.aspx
};

ProcessMemoryInfo GetCurrentProcessMemoryInfo();

//-----------------------------------------------------------------------------
// Retrieves memory info for the system (counterpart of GetCurrentProcessMemoryInfo)
//
// Do not call this function frequently as it takes a long time to
// execute on typical computers.

struct SystemMemoryInfo {
  double PageFault; // System-wide hard page faults per sec
  uint64_t CommittedTotal; // Total number of committed memory (in bytes) on the system
};

SystemMemoryInfo GetSystemMemoryInfo();

enum class CPUInstructionSet {
  Unknown,
  Basic, // Just basic 32 bit 386.
  SEE1,
  SSE2,
  SSE3,
  SSSE3, // Supplementary SSE3
  SSE41,
  SSE42,
  AVX1,
  AVX2
};

//-----------------------------------------------------------------------------
// Indicates the minimum instruction set that the CPU + OS support. Note that an older OS may
// not properly support a given CPU instruction set, usually because it doesn't know how to
// preserve its registers on context switch.

CPUInstructionSet GetSupportedCPUInstructionSet(bool* popcntSupported, bool* lzcntSupported);
} // namespace Util
} // namespace OVR

#endif // OVR_Util_SystemInfo_h
