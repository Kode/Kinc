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
    #endif
#endif


namespace OVR { namespace Util {

// From http://blogs.msdn.com/b/oldnewthing/archive/2005/02/01/364563.aspx
#if defined (OVR_OS_WIN64) || defined (OVR_OS_WIN32)

#pragma comment(lib, "version.lib")

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
bool Is64BitWindows()
{
#if defined(_WIN64)
    return TRUE;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
    // 32-bit programs run on both 32-bit and 64-bit Windows
    // so must sniff
    BOOL f64 = FALSE;
    LPFN_ISWOW64PROCESS fnIsWow64Process;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process");
    if (NULL != fnIsWow64Process)
    {
        return fnIsWow64Process(GetCurrentProcess(), &f64) && f64;
    }
    return FALSE;
#else
    return FALSE; // Win64 does not support Win16
#endif
}
#endif

const char * OSAsString()
{
#if defined (OVR_OS_IPHONE)
    return "IPhone";
#elif defined (OVR_OS_DARWIN)
    return "Darwin";
#elif defined (OVR_OS_MAC)
    return "Mac";
#elif defined (OVR_OS_BSD)
    return "BSD";
#elif defined (OVR_OS_WIN64) || defined (OVR_OS_WIN32)
    if (Is64BitWindows())
        return "Win64";
    else
        return "Win32";
#elif defined (OVR_OS_ANDROID)
    return "Android";
#elif defined (OVR_OS_LINUX)
    return "Linux";
#elif defined (OVR_OS_BSD)
    return "BSD";
#else
    return "Other";
#endif
}

uint64_t GetGuidInt()
{
    uint64_t g = Timer::GetTicksNanos();

    uint64_t lastTime, thisTime;
    int j;
    // Sleep a small random time, then use the last 4 bits as a source of randomness
    for (j = 0; j < 8; j++)
    {
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

String GetGuidString()
{
    uint64_t guid = GetGuidInt();

    char buff[64];
#if defined(OVR_CC_MSVC)
    snprintf(buff, sizeof(buff), "%I64u", guid);
#else
    snprintf(buff, sizeof(buff), "%llu", (unsigned long long) guid);
#endif
    return String(buff);
}

const char * GetProcessInfo()
{
#if defined (OVR_CPU_X86_64    )
    return "64 bit";
#elif defined (OVR_CPU_X86)
    return "32 bit";
#else
    return "TODO";
#endif
}
#ifdef OVR_OS_WIN32

String OSVersionAsString()
{
    return GetSystemFileVersionStringW(L"\\kernel32.dll");
}
String GetSystemFileVersionStringW(wchar_t filePath[MAX_PATH])
{
    wchar_t strFilePath[MAX_PATH]; // Local variable
    UINT sysDirLen = GetSystemDirectoryW(strFilePath, ARRAYSIZE(strFilePath));
    if (sysDirLen != 0)
    {
        OVR_wcscat(strFilePath, MAX_PATH, filePath);
        return GetFileVersionStringW(strFilePath);
    }
    else
    {
        return "GetSystemDirectoryW failed";
    }
}

// See http://stackoverflow.com/questions/940707/how-do-i-programatically-get-the-version-of-a-dll-or-exe-file
String GetFileVersionStringW(wchar_t filePath[MAX_PATH])
{
    String result;

    DWORD dwSize = GetFileVersionInfoSizeW(filePath, NULL);
    if (dwSize == 0)
    {
        OVR_DEBUG_LOG(("Error in GetFileVersionInfoSizeW: %d (for %s)", GetLastError(), String(filePath).ToCStr()));
        result = String(filePath) + " not found";
    }
    else
    {
        auto pVersionInfo = std::make_unique<BYTE[]>(dwSize);
        if (!pVersionInfo)
        {
            OVR_DEBUG_LOG(("Out of memory allocating %d bytes (for %s)", dwSize, filePath));
            result = "Out of memory";
        }
        else
        {
            if (!GetFileVersionInfoW(filePath, 0, dwSize, pVersionInfo.get()))
            {
                OVR_DEBUG_LOG(("Error in GetFileVersionInfo: %d (for %s)", GetLastError(), String(filePath).ToCStr()));
                result = "Cannot get version info";
            }
            else
            {
                VS_FIXEDFILEINFO* pFileInfo = NULL;
                UINT              pLenFileInfo = 0;
                if (!VerQueryValueW(pVersionInfo.get(), L"\\", (LPVOID*)&pFileInfo, &pLenFileInfo))
                {
                    OVR_DEBUG_LOG(("Error in VerQueryValueW: %d (for %s)", GetLastError(), String(filePath).ToCStr()));
                    result = "File has no version info";
                }
                else
                {
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

String GetCameraDriverVersion()
{
    return GetSystemFileVersionStringW(L"\\drivers\\OCUSBVID.sys");
}

// From http://stackoverflow.com/questions/9524309/enumdisplaydevices-function-not-working-for-me
void GetGraphicsCardList( Array< String > &gpus)
{
    gpus.Clear();
    DISPLAY_DEVICEW dd;
    dd.cb = sizeof(dd);

    DWORD deviceNum = 0;
    while( EnumDisplayDevicesW(NULL, deviceNum, &dd, 0) )
    {
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
            gpus.PushBack(String(dd.DeviceString));
        deviceNum++;
    }
}

String GetProcessorInfo()
{
    char brand[0x40] = {};
    int cpui[4] = { -1 };

    __cpuidex(cpui, 0x80000002, 0);

    //unsigned int blocks = cpui[0];
    for (int i = 0; i <= 2; ++i)
    {
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
//use objective c source

// used for driver files
String GetFileVersionString(String /*filePath*/)
{
    return String();
}

String GetSystemFileVersionString(String /*filePath*/)
{
    return String();
}

String GetDisplayDriverVersion()
{
    return String();
}

String GetCameraDriverVersion()
{
    return String();
}

#else

String GetDisplayDriverVersion()
{
    char info[256] = { 0 };
    FILE *file = popen("/usr/bin/glxinfo", "r");
    if (file)
    {
        int status = 0;
        while (status == 0)
        {
            status = fscanf(file, "%*[^\n]\n"); // Read up till the end of the current line, leaving the file pointer at the beginning of the next line (skipping any leading whitespace).
            OVR_UNUSED(status);                 // Prevent GCC compiler warning: "ignoring return value of ‘int fscanf(FILE*, const char*, ...)"

            status = fscanf(file, "OpenGL version string: %255[^\n]", info);
        }
        pclose(file);
        if (status == 1)
        {
            return String(info);
        }
    }
    return String("No graphics driver details found.");
}

String GetCameraDriverVersion()
{
    struct utsname kver;
    if (uname(&kver))
    {
        return String();
    }
    return String(kver.release);
}

void GetGraphicsCardList(OVR::Array< OVR::String > &gpus)
{
    gpus.Clear();

    char info[256] = { 0 };
    FILE *file = popen("/usr/bin/lspci", "r");
    if (file)
    {
        int status = 0;
        while (status >= 0)
        {
            status = fscanf(file, "%*[^\n]\n"); // Read up till the end of the current line, leaving the file pointer at the beginning of the next line (skipping any leading whitespace).
            OVR_UNUSED(status);                 // Prevent GCC compiler warning: "ignoring return value of ‘int fscanf(FILE*, const char*, ...)"

            status = fscanf(file, "%*[^ ] VGA compatible controller: %255[^\n]", info);
            if (status == 1)
            {
                gpus.PushBack(String(info));
            }
        }
        pclose(file);
    }
    if (gpus.GetSizeI() <= 0)
    {
        gpus.PushBack(String("No video card details found."));
    }
}

String OSVersionAsString()
{
    char info[256] = { 0 };
    FILE *file = fopen("/etc/issue", "r");
    if (file)
    {
        int status = fscanf(file, "%255[^\n\\]", info);
        fclose(file);
        if (status == 1)
        {
            return String(info);
        }
    }
    return String("No OS version details found.");
}

String GetProcessorInfo()
{
    char info[256] = { 0 };
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (file)
    {
        int status = 0;
        while (status == 0)
        {
            status = fscanf(file, "%*[^\n]\n"); // Read up till the end of the current line, leaving the file pointer at the beginning of the next line (skipping any leading whitespace).
            OVR_UNUSED(status);                 // Prevent GCC compiler warning: "ignoring return value of ‘int fscanf(FILE*, const char*, ...)"

            status = fscanf(file, "model name : %255[^\n]", info);
        }
        fclose(file);
        if (status == 1)
        {
            return String(info);
        }
    }
    return String("No processor details found.");
}
#endif //OVR_OS_MAC
#endif // WIN32

//-----------------------------------------------------------------------------
// Get a path under BaseOVRPath

String GetOVRPath(const wchar_t* subPath, bool create_dir)
{
    wchar_t fullPath[MAX_PATH];
    SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, NULL, 0, fullPath);
    PathAppendW(fullPath, L"\\Oculus");
    PathAppendW(fullPath, subPath);

    if (create_dir)
    {
        DWORD attrib = ::GetFileAttributesW(fullPath);
        bool exists = attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY);
        if (!exists)
        {
            ::CreateDirectoryW(fullPath, NULL);
        }
    }

    return String(fullPath);
}

//-----------------------------------------------------------------------------
// Get the path for local app data.

String GetBaseOVRPath(bool create_dir)
{
#if defined(OVR_OS_WIN32)

    wchar_t path[MAX_PATH];
    ::SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, NULL, 0, path);

    OVR_wcscat(path, MAX_PATH, L"\\Oculus");

    if (create_dir)
    {   // Create the Oculus directory if it doesn't exist
        DWORD attrib = ::GetFileAttributesW(path);
        bool exists = attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY);
        if (!exists)
        {
            ::CreateDirectoryW(path, NULL);
        }
    }

#elif defined(OVR_OS_MS) // Other Microsoft OSs

    // TODO: figure this out.
    OVR_UNUSED ( create_dir );
    path = "";

#elif defined(OVR_OS_MAC)

    const char* home = getenv("HOME");
    path = home;
    path += "/Library/Preferences/Oculus";

    if (create_dir)
    {   // Create the Oculus directory if it doesn't exist
        DIR* dir = opendir(path);
        if (dir == NULL)
        {
            mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
        }
        else
        {
            closedir(dir);
        }
    }

#else

    const char* home = getenv("HOME");
    String path = home;
    path += "/.config/Oculus";

    if (create_dir)
    {   // Create the Oculus directory if it doesn't exist
        DIR* dir = opendir(path);
        if (dir == NULL)
        {
            mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
        }
        else
        {
            closedir(dir);
        }
    }

#endif

    return String(path);
}

#ifdef OVR_OS_MS
//widechar functions are Windows only for now
bool GetRegistryStringW(const wchar_t* pSubKey, const wchar_t* stringName, wchar_t out[MAX_PATH], bool wow64value, bool currentUser)
{
    HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    DWORD dwType = REG_SZ;
    HKEY hKey = 0;
    wchar_t value[MAX_PATH + 1]; // +1 because RegQueryValueEx doesn't necessarily 0-terminate.
    DWORD value_length = MAX_PATH;

    if ((RegOpenKeyExW(root, pSubKey, 0, KEY_QUERY_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY), &hKey) != ERROR_SUCCESS) ||
        (RegQueryValueExW(hKey, stringName, NULL, &dwType, (LPBYTE)&value, &value_length) != ERROR_SUCCESS) || (dwType != REG_SZ))
    {
        out[0] = L'\0';
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);

    value[value_length] = L'\0';
    wcscpy_s(out, MAX_PATH, value);
    return true;
}


bool GetRegistryDwordW(const wchar_t* pSubKey, const wchar_t* stringName, DWORD& out, bool wow64value, bool currentUser)
{
    HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    DWORD dwType = REG_DWORD;
    HKEY hKey = 0;
    DWORD value_length = sizeof(DWORD);

    if ((RegOpenKeyExW(root, pSubKey, 0, KEY_QUERY_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY), &hKey) != ERROR_SUCCESS) ||
        (RegQueryValueExW(hKey, stringName, NULL, &dwType, (LPBYTE)&out, &value_length) != ERROR_SUCCESS) || (dwType != REG_DWORD))
    {
        out = 0;
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);

    return true;
}

bool GetRegistryFloatW(const wchar_t* pSubKey, const wchar_t* stringName, float& out, bool wow64value, bool currentUser)
{
    out = 0.f;

    wchar_t stringValue[MAX_PATH];
    bool result = GetRegistryStringW(pSubKey, stringName, stringValue, wow64value, currentUser);

    if (result)
    {
        out = wcstof(stringValue, nullptr);

        if (errno == ERANGE)
        {
            out = 0.f;
            result = false;
        }
    }

    return result;
}

bool GetRegistryDoubleW(const wchar_t* pSubKey, const wchar_t* stringName, double& out, bool wow64value, bool currentUser)
{
    out = 0.0;

    wchar_t stringValue[MAX_PATH];
    bool result = GetRegistryStringW(pSubKey, stringName, stringValue, wow64value, currentUser);

    if (result)
    {
        out = wcstod(stringValue, nullptr);

        if (errno == ERANGE)
        {
            out = 0.0;
            result = false;
        }
    }

    return result;
}


bool GetRegistryBinaryW(const wchar_t* pSubKey, const wchar_t* stringName, LPBYTE out, DWORD* size, bool wow64value, bool currentUser)
{
    HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    DWORD dwType = REG_BINARY;
    HKEY hKey = 0;

    if ((RegOpenKeyExW(root, pSubKey, 0, KEY_QUERY_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY), &hKey) != ERROR_SUCCESS) ||
        (RegQueryValueExW(hKey, stringName, NULL, &dwType, out, size) != ERROR_SUCCESS) || (dwType != REG_BINARY))
    {
        *out = 0;
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);

    return true;
}


// When reading Oculus registry keys, we recognize that the user may have inconsistently
// used a DWORD 1 vs. a string "1", and so we support either when checking booleans.
bool GetRegistryBoolW(const wchar_t* pSubKey, const wchar_t* stringName, bool defaultValue, bool wow64value, bool currentUser)
{
    wchar_t out[MAX_PATH];
    if (GetRegistryStringW(pSubKey, stringName, out, wow64value, currentUser))
    {
        return (_wtoi64(out) != 0);
    }

    DWORD dw;
    if (GetRegistryDwordW(pSubKey, stringName, dw, wow64value, currentUser))
    {
        return (dw != 0);
    }

    return defaultValue;
}

bool SetRegistryBinaryW(const wchar_t* pSubKey, const wchar_t* stringName, LPBYTE value, DWORD size, bool wow64value, bool currentUser)
{
    HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    HKEY hKey = 0;

    if ((RegCreateKeyExW(root, pSubKey, 0, nullptr, 0, KEY_CREATE_SUB_KEY | KEY_SET_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY), nullptr, &hKey, nullptr) != ERROR_SUCCESS) ||
        (RegSetValueExW(hKey, stringName, 0, REG_BINARY, value, size) != ERROR_SUCCESS))
    {
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);
    return true;
}

bool SetRegistryDwordW(const wchar_t* pSubKey, const wchar_t* stringName, DWORD newValue, bool wow64value, bool currentUser)
{
    HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    HKEY hKey = 0;

    if ((RegCreateKeyExW(root, pSubKey, 0, nullptr, 0, KEY_CREATE_SUB_KEY | KEY_SET_VALUE | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY), nullptr, &hKey, nullptr) != ERROR_SUCCESS) ||
        (RegSetValueExW(hKey, stringName, 0, REG_DWORD, reinterpret_cast<BYTE*>( &newValue ), sizeof(newValue)) != ERROR_SUCCESS))
    {
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);
    return true;
}

bool DeleteRegistryValue(const wchar_t* pSubKey, const wchar_t* stringName, bool wow64value, bool currentUser)
{
    HKEY root = currentUser ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
    HKEY hKey = 0;

    if (RegOpenKeyExW(root, pSubKey, 0, KEY_ALL_ACCESS | (wow64value ? KEY_WOW64_32KEY : KEY_WOW64_64KEY), &hKey) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }
    bool result = (RegDeleteValueW(hKey, stringName) == ERROR_SUCCESS);

    RegCloseKey(hKey);
    return result;
}

// Usually this returns C:\Program Files (x86)\Oculus\Support\oculus-runtime
bool GetOVRRuntimePathW(wchar_t runtimePath[MAX_PATH])
{
    if (GetRegistryStringW(L"Software\\Oculus VR, LLC\\Oculus", L"Base", runtimePath, Is64BitWindows(), false))
    {
        // At this point, runtimePath is usually "C:\Program Files (x86)\Oculus\", so append what we need to it.
        return (OVR_strlcat(runtimePath, L"Support\\oculus-runtime", MAX_PATH) < MAX_PATH);
    }

    runtimePath[0] = 0;
    return false;
}

#endif // OVR_OS_MS


bool GetOVRRuntimePath(String& runtimePath)
{
    runtimePath.Clear();

    #ifdef OVR_OS_MS
        wchar_t path[MAX_PATH];
        if (GetOVRRuntimePathW(path))
        {
            runtimePath = String(path);
            return true;
        }
    #else
        //mac/linux uses environment variables
    #endif

    return false;
}

bool GetDefaultFirmwarePath(String& firmwarePath)
{
    if (!GetOVRRuntimePath(firmwarePath))
    {
        return false;
    }
    else
    {
        firmwarePath + "\\Tools\\FirmwareBundle.json";
        return true;
    }
}


//-----------------------------------------------------------------------------
// GetFirmwarePath
//
// This function searches for likely locations of the firmware.zip file when
// the file path is not specified by the user.

// We search the following paths relative to the executable:
static const wchar_t* FWRelativePaths[] = {
    L"firmware.zip",
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
    L"../oculus-tools/firmware/Firmware.zip"
};
static const int RelativePathsCount = OVR_ARRAY_COUNT(FWRelativePaths);

#ifdef OVR_OS_MS
static std::wstring GetModulePath()
{
    wchar_t wpath[MAX_PATH];
    DWORD len = ::GetModuleFileNameW(nullptr, wpath, MAX_PATH);

    if (len <= 0 || len >= MAX_PATH)
    {
        return std::wstring();
    }

    return wpath;
}

#endif //OVR_OS_MS
bool GetFirmwarePath(std::wstring* outPath)
{
#ifdef OVR_OS_MS

    std::wstring basePath = GetModulePath();

    // Remove trailing slash.
    std::wstring::size_type lastSlashOffset = basePath.find_last_of(L"/\\", std::wstring::npos, 2);
    if (lastSlashOffset == std::string::npos)
    {
        // Abort if no trailing slash.
        return false;
    }
    basePath = basePath.substr(0, lastSlashOffset + 1);

    // For each relative path to test,
    for (int i = 0; i < RelativePathsCount; ++i)
    {
        // Concatenate the relative path on the base path (base path contains a trailing slash)
        std::wstring candidatePath = basePath + FWRelativePaths[i];

        // If a file exists at this location,
        if (::PathFileExistsW(candidatePath.c_str()))
        {
            // Return the path if requested.
            if (outPath)
            {
                *outPath = candidatePath;
            }

            return true;
        }
    }
#else
    OVR_UNUSED2(firmwarePath, numSearchDirs);
    //#error "FIXME"
#endif //OVR_OS_MS
    return false;
}

// Returns if the computer is currently locked
bool CheckIsComputerLocked()
{
#ifdef OVR_OS_MS
    LPWSTR pBuf = nullptr;
    DWORD bytesReturned = 0;

    if (::WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, WTSSessionInfoEx, &pBuf, &bytesReturned))
    {
        if (pBuf && bytesReturned >= sizeof(WTSINFOEX))
        {
            WTSINFOEXW* info = (WTSINFOEXW*)pBuf;

            WTSINFOEX_LEVEL1_W* level1 = (WTSINFOEX_LEVEL1_W*)&info->Data;


            bool isLocked = false;

            if (level1->SessionFlags == WTS_SESSIONSTATE_LOCK)
            {
                isLocked = true;
            }
            else if (level1->SessionFlags != WTS_SESSIONSTATE_UNLOCK) // if not unlocked, we expect locked
            {
                LogError("Unknown Lock State = %d", (int)level1->SessionFlags);
            }

            // Note: On Windows 7, the locked and unlocked flags are reversed!
            // See: https://msdn.microsoft.com/en-us/library/windows/desktop/ee621019(v=vs.85).aspx
            if (IsAtMostWindowsVersion(WindowsVersion::Windows7_SP1))
            {
                isLocked = !isLocked;
            }

            return isLocked;
        }
        else
        {
            LogError("Wrong return size from WTSQuerySessionInformation %u", bytesReturned );
        }
        if ( pBuf )
        {
            WTSFreeMemory(pBuf);
        }
    }
#endif // OVR_OS_MS
    return false;
}


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
    L"RiftFirmLoad2.exe"
};

static const int RFL2RelativePathsCount = OVR_ARRAY_COUNT(RFL2RelativePaths);


bool GetRFL2Path(std::wstring* outPath)
{
#ifdef OVR_OS_MS

    std::wstring basePath = GetModulePath();

    // Remove trailing slash.
    std::wstring::size_type lastSlashOffset = basePath.find_last_of(L"/\\", std::wstring::npos, 2);
    if (lastSlashOffset == std::string::npos)
    {
        // Abort if no trailing slash.
        return false;
    }
    basePath = basePath.substr(0, lastSlashOffset + 1);

    // For each relative path to test,
    for (int i = 0; i < RFL2RelativePathsCount; ++i)
    {
        // Concatenate the relative path on the base path (base path contains a trailing slash)
        std::wstring candidatePath = basePath + RFL2RelativePaths[i];

        // If a file exists at this location,
        if (::PathFileExistsW(candidatePath.c_str()))
        {
            // Return the path if requested.
            if (outPath)
            {
                *outPath = candidatePath;
            }

            return true;
        }
    }
#else
    OVR_UNUSED(outPath);
    //#error "FIXME"
#endif //OVR_OS_MS
    return false;
}

#ifdef OVR_OS_MS

static INIT_ONCE OSVersionInitOnce = INIT_ONCE_STATIC_INIT;
static uint32_t OSVersion;
static uint32_t OSBuildNumber;

BOOL CALLBACK VersionCheckInitOnceCallback(PINIT_ONCE, PVOID, PVOID*)
{
    typedef NTSTATUS(WINAPI * pfnRtlGetVersion)(PRTL_OSVERSIONINFOEXW lpVersionInformation);

    NTSTATUS status = STATUS_DLL_NOT_FOUND;

    HMODULE hNTDll = LoadLibraryW(L"ntdll.dll");
    OVR_ASSERT(hNTDll);

    if (hNTDll)
    {
        status = STATUS_ENTRYPOINT_NOT_FOUND;

        pfnRtlGetVersion pRtlGetVersion = (pfnRtlGetVersion)GetProcAddress(hNTDll, "RtlGetVersion");
        OVR_ASSERT(pRtlGetVersion);

        if (pRtlGetVersion)
        {
            RTL_OSVERSIONINFOEXW OSVersionInfoEx;
            OSVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVersionInfoEx);
            status = pRtlGetVersion(&OSVersionInfoEx);
            OVR_ASSERT(status == 0);

            if (status == 0)
            {
                OSVersion = OSVersionInfoEx.dwMajorVersion * 100 + OSVersionInfoEx.dwMinorVersion;
                OSBuildNumber = OSVersionInfoEx.dwBuildNumber;
            }
        }

        FreeLibrary(hNTDll);
    }

    if (status != 0)
    {
        LogError("[VersionCheckInitOnceCallback] Failed to obtain OS version information. 0x%08x\n", status);
    }

    return (status == 0);
}

#endif // OVR_OS_MS

bool IsAtLeastWindowsVersion(WindowsVersion version)
{
#ifdef OVR_OS_MS
    if (!InitOnceExecuteOnce(&OSVersionInitOnce, VersionCheckInitOnceCallback, nullptr, nullptr))
    {
        OVR_ASSERT(false);
        return false;
    }

    switch (version)
    {
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

bool IsAtMostWindowsVersion(WindowsVersion version)
{
#ifdef OVR_OS_MS
    if (!InitOnceExecuteOnce(&OSVersionInitOnce, VersionCheckInitOnceCallback, nullptr, nullptr))
    {
        OVR_ASSERT(false);
        return false;
    }

    switch (version)
    {
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




ProcessMemoryInfo GetCurrentProcessMemoryInfo()
{
    ProcessMemoryInfo pmi = {};
    PROCESS_MEMORY_COUNTERS_EX pmce = { sizeof(PROCESS_MEMORY_COUNTERS_EX), 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    // This should always succeed for the current process.
    // We need PROCESS_QUERY_LIMITED_INFORMATION rights if we want to read the info from a different process.
    // This call used nearly 1ms on a sampled computer, and so should be called infrequently.
    if (::GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmce), sizeof(pmce)))
    {
        pmi.UsedMemory = (uint64_t)pmce.WorkingSetSize; // The set of pages in the virtual address space of the process that are currently resident in physical memory.
    }

    return pmi;
}


//-----------------------------------------------------------------------------
// PdhHelper is a helper class for querying perf counters using PDH.
class PdhHelper
{
public:
    // RateCounter encapsulates a single counter that represents rate information.
    struct RateCounter
    {
        // Requires a query object and counter path.
        RateCounter(PDH_HQUERY hQuery, LPCWSTR counterPath)
            : Valid(false)
            , HasLast(false)
            , CounterHandle(0)
        {
            if (!hQuery)
                return;

            PDH_STATUS status = PdhAddCounterW(hQuery, counterPath, 0, &CounterHandle);
            if (ERROR_SUCCESS != status)
                return;

            Valid = true;
        }

        ~RateCounter()
        {
            if (CounterHandle)
                PdhRemoveCounter(CounterHandle);
        }

        bool Valid;                 // Whether this counter object was initialized properly
        bool HasLast;               // Whether we have a previous snapshot. Need it to compute rate.
        PDH_RAW_COUNTER Last;       // Previous counter values
        PDH_HCOUNTER CounterHandle; // PDH handle

        // Call this to obtain the latest counter value.
        // Note that PdhHelper::Collect() must be called prior to this call.
        double Query()
        {
            if (!Valid)
                return 0.0;

            PDH_RAW_COUNTER now;
            DWORD counterType;
            PDH_STATUS status = PdhGetRawCounterValue(CounterHandle, &counterType, &now);
            if (ERROR_SUCCESS == status && PDH_CSTATUS_VALID_DATA == now.CStatus)
            {
                // Calculate the rate (since this is a rate counter).
                // FirstValue is the instance count. SecondValue is the timestamp.
                double result = HasLast ? double(now.FirstValue - Last.FirstValue) / (double(now.SecondValue - Last.SecondValue) * Timer::GetPerfFrequencyInverse()) : 0.0;

                Last = now;
                HasLast = true;

                return result;
            }

            return 0.0;
        }
    };

public:
    PdhHelper()
        : PdhQuery(0)
    {
        PDH_STATUS status = PdhOpenQueryW(NULL, 0, &PdhQuery);
        if (ERROR_SUCCESS != status)
        {
            // If this fails, PdhQuery will be null, and subsequent query will
            // check for this and return default data if we have no query object.
            CleanUp();
        }

        return;
    }

    ~PdhHelper()
    {
        CleanUp();
    }

    void CleanUp()
    {
        if (PdhQuery)
            PdhCloseQuery(PdhQuery);
    }

    // GetHandle() is used when creating new counter objects (RateCounter)
    PDH_HQUERY GetHandle()
    {
        return PdhQuery;
    }

    // Collects raw counter values for all counters in this query.
    bool Collect()
    {
        return ERROR_SUCCESS == PdhCollectQueryData(PdhQuery);
    }

private:
    PDH_HQUERY PdhQuery;
};

static PdhHelper PdhHelperInstance; // This is our query instance. Only one is needed.

// Declare all counters used here.
static PdhHelper::RateCounter PdhPageFault(PdhHelperInstance.GetHandle(), L"\\Memory\\Page Reads/sec");

SystemMemoryInfo GetSystemMemoryInfo()
{
    SystemMemoryInfo smi = {};

    // A single Collect call is required for the query object regardless of how many counters
    // we are interested in.
    if (PdhHelperInstance.Collect())
    {
        smi.PageFault = PdhPageFault.Query();
    }
    else
    {
        // Can't get the counter for some reason. Use default.
        smi.PageFault = 0.0;
    }

    PERFORMANCE_INFORMATION pi = { sizeof(pi) };
    if (GetPerformanceInfo(&pi, sizeof(pi)))
        smi.CommittedTotal = pi.CommitTotal;

    return smi;
}


}} // namespace OVR::Util
