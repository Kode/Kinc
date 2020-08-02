/************************************************************************************

Filename    :   Util_SystemInfo.h
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

#ifndef OVR_Util_SystemInfo_h
#define OVR_Util_SystemInfo_h

#include "Kernel/OVR_String.h"
#include "Kernel/OVR_Types.h"
#include "Kernel/OVR_Array.h"
#include "Kernel/OVR_DebugHelp.h"
#include <vector>
#include <set>
#include <string>

#if (__cplusplus >= 201703L) || \
    (defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L) && (_MSC_VER >= 1913))
#define OVR_UTIL_STD_FILESYSTEM_AVAILABLE
#include <filesystem>
#endif // (__cplusplus >= 201703L) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L) && (_MSC_VER
       // >= 1913))

namespace OVR {
namespace Util {

//-----------------------------------------------------------------------------
// Utility functions
//

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

#ifdef OVR_UTIL_STD_FILESYSTEM_AVAILABLE
// Returns the current module (e.g. exe or dll) path. If executed within a DLL, it returns the
// DLL path.
std::filesystem::path GetCurrentModuleDirectoryPath();
#endif // OVR_UTIL_STD_FILESYSTEM_AVAILABLE

// Checks if the computer is currently locked
bool CheckIsComputerLocked();

//-----------------------------------------------------------------------------
// SettingsManager
//
// This is a portable interface replacement for Windows-specific registry key functions.
//
// For all path-based functions:
//   - Paths are UTF8-encoded.
//   - Paths are case-insensitive.
//   - Path follows Unix path conventions (e.g. /a/b/c/d).
//
// Windows platform:
//   Applications may use Registry setting paths to read the Windows registry.
//   An example Windows path is:
//      "/HKEY_LOCAL_MACHINE/Software/Oculus/ForceGPUDriverVersionAcceptance"
//   Windows registry path reading for Win32 apps on Win64 OSs does not change
//   the path to insert Wow64 paths, though users of this functionality may explicitly
//   use "WoW64" in paths.
//
// Example Windows usage with default paths:
//   mgr.SetDefaultLocation("/HKEY_LOCAL_MACHINE/Software/Oculus/");
//   ...
//   mgr.ReadValue("EnableTimeouts", timeoutsEnabled);
//   mgr.ReadValue("FileName", test.tga);
//   mgr.ReadValue("Graphics/ATWEnabled", aswEnabled);
//
// Example Windows usage with full paths:
//   bool result = mgr.ReadValue("/HKEY_LOCAL_MACHINE/Software/Oculus/EnablePTW", ptwEnabled);
//   bool result = mgr.WriteValue("/HKEY_LOCAL_MACHINE/Software/Oculus/FileName", "test.tga");
//
class SettingsManager {
 public:
  SettingsManager() = default;
  ~SettingsManager() = default;

  enum Options {
    OptionNone = 0x00,
    OptionUse32BitDatabase = 0x01, // Windows: This means to use KEY_WOW64_32KEY
    OptionIgnoreAlternativeLocations = 0x02
  };

  // ReadValue:
  //   - Return true if value exists and could be read, false if doesn't exist or can't be read.
  //   - The value is written to only if the setting could be read (true return value).
  //   - If the value was not originally written as the requested type,
  //     it is interpreted as the type.
  //   - If the path is relative path (no initial '/' char) then the default location
  //     is prefixed to path.
  bool ReadValue(const char* path, bool& value, int options = OptionNone);
  bool ReadValue(const char* path, int32_t& value, int options = OptionNone);
  bool ReadValue(const char* path, uint32_t& value, int options = OptionNone);
  bool ReadValue(const char* path, unsigned long& value, int options = OptionNone); // a.k.a. DWORD
  bool ReadValue(const char* path, int64_t& value, int options = OptionNone);
  bool ReadValue(const char* path, uint64_t& value, int options = OptionNone);
  bool ReadValue(const char* path, float& value, int options = OptionNone);
  bool ReadValue(const char* path, double& value, int options = OptionNone);
  bool ReadValue(const char* path, std::string& value, int options = OptionNone);
  bool ReadValue(const char* path, std::wstring& value, int options = OptionNone);
  bool ReadValue(const char* path, std::vector<uint8_t>& value, int options = OptionNone);
  bool ReadValue(const char* path, uint8_t* data, size_t& dataSize, int options = OptionNone);

  // WriteValue:
  //    - Creates the setting if not already present
  //    - Return true if the setting could be created (if needed) and written.
  //    - Writes only the given path and not alternative locations.
  //    - A value of a given type can be overwritten with a value of a different type.
  bool WriteValue(const char* path, bool value, int options = OptionNone);
  bool WriteValue(const char* path, int32_t value, int options = OptionNone);
  bool WriteValue(const char* path, uint32_t value, int options = OptionNone);
  bool WriteValue(const char* path, int64_t value, int options = OptionNone);
  bool WriteValue(const char* path, uint64_t value, int options = OptionNone);
  bool WriteValue(const char* path, float value, int options = OptionNone);
  bool WriteValue(const char* path, double value, int options = OptionNone);
  bool WriteValue(const char* path, const char* value, size_t valStrlen, int options = OptionNone);
  bool WriteValue(const char* path, const std::string& str, int options = OptionNone);
  bool WriteValue(const char* path, const wchar_t* val, size_t valStrlen, int options = OptionNone);
  bool WriteValue(const char* path, const std::wstring& str, int options = OptionNone);
  bool WriteValue(const char* path, const uint8_t* data, size_t dataSize, int options = OptionNone);

  // Returns true if the value can be read and exists.
  bool ValueExists(const char* path, int options = OptionNone) const;

  // Returns true if the value could be deleted. Returns false if the value was present but deletion
  // failed. wasPresent indicates if the value was present before deletion was attempted. Deletion
  // affects the path and any registered alternative locations, unless
  // OptionIgnoreAlternativeLocations is specified.
  bool DeleteValue(const char* path, bool* wasPresent = nullptr, int options = OptionNone);

  // Sets the default location for read and written value paths.
  // Path must be a "full path", which begins with a path separator.
  // Path must end with trailing path separtor.
  // Defaults to empty.
  // Example usage:
  //    settingsMgr.SetDefaultLocation("/HKEY_LOCAL_MACHINE/Software/Oculus/");
  void SetDefaultLocation(const char* parentPath);

#define OVR_DEFAULT_SETTINGS_LOCATION "/HKEY_LOCAL_MACHINE/Software/Oculus/"

  // Returns the default location, which can be set by SetDefaultLocation.
  std::string GetDefaultLocation() const;

  // Adds an alternative location for value reads.
  // Path must end with trailing path separtor.
  // This function exists for the purpose of allowing migration of old Oculus settings
  // paths to a single unified path.
  // This function is not thread safe. It can be called from only a single thread at a time.
  // Example usage:
  //    settingsMgr.AddAlternativeLocation("/HKEY_LOCAL_MACHINE/Software/Oculus2/");
  void AddAlternativeLocation(const char* parentPath);

  // This function is not thread safe. It can be called from only a single thread at a time.
  // Example usage:
  //    settingsMgr.RemoveAlternativeLocation("/HKEY_LOCAL_MACHINE/Software/Oculus2/");
  void RemoveAlternativeLocation(const char* parentPath);

  struct InsensitiveCompare {
    bool operator()(const std::string& a, const std::string& b) const {
      return OVR_stricmp(a.c_str(), b.c_str()) < 0;
    }
  };

  typedef std::set<std::string, InsensitiveCompare> StringSet;

  // Gets a set of all currently registered alternative locations.
  StringSet GetAlternativeLocations() const;

  // Sets Oculus-specific alternative locations.
  // This function is not thread safe. It can be called from only a single thread at a time.
  void SetDefaultAlternativeLocations();

  // Returns the values present at the given parentPath.
  // Writes a single line per enumerated value, with each line followed by a newline,
  // including the last line.
  // The parentPath must begin with '/' and end with '/' or be NULL to indicate usage of the
  // default path.
  // Example usage:
  //    str = settingsMgr.EnumerateValues("/HKEY_LOCAL_MACHINE/Software/Oculus/");
  // Note:
  //    When this data is backed by the Windows registry, the enumerated types won't be the
  //    same as the written types. For example, WriteDouble writes to the Windows registry as
  //    a binary value, and will be enumerated by EnumerateValues as binary instead of float.
  std::string EnumerateValues(const char* parentPath = nullptr, int options = OptionNone) const;

 protected:
  // This is a parent settings directory which acts as the default if value
  // reads and writes do not include a parent path.
  // Example Windows value: "/HKEY_LOCAL_MACHINE/Software/Oculus2/"
  std::string defaultLocation;

#ifdef _WIN32
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724884(v=vs.85).aspx
  struct Win32RegistryAnyValue {
    uint32_t type; // Which REG_XXX type is used.
    std::vector<uint8_t> binaryData; // REG_BINARY
    uint32_t dwordData; // REG_DWORD
    uint64_t qwordData; // REG_QWORD
    std::string stringData; // REG_SZ or REG_EXPAND_SZ
    std::vector<std::string> stringArrayData; // REG_MULTI_SZ

    Win32RegistryAnyValue();
  };

  // Reads the Windows registry at the given path to a Win32RegistryAnyValue.
  bool Win32ReadAnyRegistryValue(const char* path, Win32RegistryAnyValue& any, int options) const;

  // Reads the Windows registry value (sourceData/sourceDataSize) to a Win32RegistryAnyValue.
  bool Win32ReadAnyRegistryValue(
      DWORD dwType,
      uint8_t* sourceData,
      size_t sourceDataSize,
      Win32RegistryAnyValue& anyValue) const;

  // Oculus unfortunately has a history of divergent Windows registry key storage locations.
  // As part of the effort to converge on a single location, we provide backward-compatible
  // old locations to check when reading values, so existing users that have registry keys at
  // the old locations can still have their settings work. The locations are the parent
  // registry key "directory". So for the "/HKEY_LOCAL_MACHINE/Software/Oculus LLC/Test" key,
  // the location would be stored here as "/HKEY_LOCAL_MACHINE/Software/Oculus LLC/"
  StringSet RegistryAlternativeLocations;
#endif
};

extern SettingsManager DefaultSettingsManager;

#ifdef OVR_OS_MS

//--------------------------------------------------------------------------------------------------
// Windows registry functions
//
// Use these functions only for reading non-Oculus registry data. For Oculus settings, use the
// SettingsManager instead, which supplants direct Windows registry usage.
//--------------------------------------------------------------------------------------------------

enum class RegistryDB {
  normal, // KEY_WOW64_64KEY. 64 bit apps use 64 bit database, 32 bit apps use 32 bit database.
  wow64_32 // KEY_WOW64_32KEY. Force looking at the 32 bit database even if this is a 64 bit app.
};

enum class RegistryBase {
  localMachine, // HKEY_LOCAL_MACHINE
  currentUser // HKEY_CURRENT_USER
};

// Returns true if a registry key of the given type is present and can be interpreted as a boolean,
// otherwise
// returns defaultValue. It's not possible to tell from a single call to this function if the given
// registry key
// was present. For Strings, boolean means (atoi(str) != 0). For DWORDs, boolean means (dw != 0).
// If registryDB is RegistryDB::normal then KEY_WOW64_32KEY is used in the registry lookup.
// If registryBase is RegistryBase::currentUser then the HKEY_CURRENT_USER root is used
// instead of HKEY_LOCAL_MACHINE
bool GetRegistryBoolW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    bool defaultValue,
    RegistryDB registryDB = RegistryDB::normal,
    RegistryBase registryBase = RegistryBase::localMachine);

// Returns true if a DWORD-type registry key of the given name is present, else sets out to 0 and
// returns false.
// If registryDB is RegistryDB::normal then KEY_WOW64_32KEY is used in the registry lookup.
// If registryBase is RegistryBase::currentUser then the HKEY_CURRENT_USER root is used
// instead of HKEY_LOCAL_MACHINE
bool GetRegistryDwordW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    DWORD& out,
    RegistryDB registryDB = RegistryDB::normal,
    RegistryBase registryBase = RegistryBase::localMachine);

// Returns true if a string-type registry key of the given name is present, else sets out to empty
// and returns false.
// The output string will always be 0-terminated, but may be empty.
// If registryDB is RegistryDB::normal then KEY_WOW64_32KEY is used in the registry lookup.
// If registryBase is RegistryBase::currentUser then the HKEY_CURRENT_USER root is used
// instead of HKEY_LOCAL_MACHINE
bool GetRegistryStringW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    wchar_t out[MAX_PATH],
    RegistryDB registryDB = RegistryDB::normal,
    RegistryBase registryBase = RegistryBase::localMachine);

// Returns true if the DWORD value could be successfully written to the registry.
// If registryDB is RegistryDB::normal then KEY_WOW64_32KEY is used in the registry lookup.
// If registryBase is RegistryBase::currentUser then the HKEY_CURRENT_USER root is used
// instead of HKEY_LOCAL_MACHINE
bool SetRegistryDwordW(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    DWORD newValue,
    RegistryDB registryDB = RegistryDB::normal,
    RegistryBase registryBase = RegistryBase::localMachine);

// Returns true if the value could be successfully deleted from the registry.
// If registryDB is RegistryDB::normal then KEY_WOW64_32KEY is used in the registry lookup.
// If registryBase is RegistryBase::currentUser then the HKEY_CURRENT_USER root is used
// instead of HKEY_LOCAL_MACHINE
bool DeleteRegistryValue(
    const wchar_t* pSubKey,
    const wchar_t* stringName,
    RegistryDB registryDB = RegistryDB::normal,
    RegistryBase registryBase = RegistryBase::localMachine);

#endif // OVR_OS_MS

//-----------------------------------------------------------------------------
// Get the path where the Oculus runtime output is written.
//
// GetBaseOVRPath() returns the top level Oculus output path.
// On Windows, this is %LOCALAPPDATA%\Oculus.
//
// GetBaseOVRPathW() has the identical behavior as GetBaseOVRPath(), but
// returns its result in a std::wstring.
//
// GetOVRPath() returns a path that represents a subdirectory from the top-level
// Oculus output path. (The top-level path is what GetBaseOVRPath() returns.)
//
// GetOVRPathW() has the identical behavior as GetOVRPath(), but returns its
// result in a std::wstring.

String GetBaseOVRPath(bool create_dir);
std::wstring GetBaseOVRPathW(bool create_dir);
String GetOVRPath(const wchar_t* subPath, bool create_dir);
std::wstring GetOVRPathW(const wchar_t* subPath, bool create_dir);

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

// Retrives the root of the Oculus install directory
// The returned string is a directory path and does not have a trailing path separator.
// Example return value: L"C:\Program Files// (x86)\Oculus\Support\oculus-runtime"
std::wstring GetOVRRuntimePathW();
#endif

// Returns a string with a comma-separated list of tags for the machine.
// The tags are obtained from the registry.
String GetMachineTags();

#if defined(_WIN32)
String GetFileVersionStringW(const wchar_t filePath[MAX_PATH]);

String GetSystemFileVersionStringW(const wchar_t filePath[MAX_PATH]);
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

//-----------------------------------------------------------------------------
// Implements ModuleInfo lookup functionalitity.
// This class is separate from the OVR::SymbolLookup class, which has some
// similar functionality, because that class has extra wiring to support (heavy)
// symbol lookups, which we don't want here and which would put a lot more weight
// on the usage of this functionality (which we want to be able to call frequently
// at runtime).
//
class ModuleInfoLookup {
 public:
  ModuleInfoLookup() = default;
  ~ModuleInfoLookup() = default;

  const OVR::ModuleInfo* GetModuleInfoForAddress(uint64_t address);
  const OVR::ModuleInfo& GetModuleInfoForCurrentModule();
  bool GetAddressIsFromCurrentModule(uint64_t address);

  // All process ModuleInfo, sorted by address.
  std::vector<OVR::ModuleInfo> ModuleInfoArray;

  // ModuleInfo for the current module. Not necessarily the main executable module.
  OVR::ModuleInfo CurrentModuleInfo;
};

extern ModuleInfoLookup DefaultModuleInfoLookup;

} // namespace Util
} // namespace OVR

#endif // OVR_Util_SystemInfo_h
