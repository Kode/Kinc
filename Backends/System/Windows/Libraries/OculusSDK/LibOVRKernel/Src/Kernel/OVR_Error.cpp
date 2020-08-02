/************************************************************************************

PublicHeader:   None
Filename    :   OVR_Error.cpp
Content     :   Structs and functions for handling OVRErrorInfos
Created     :   February 15, 2015
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

#include "OVR_Error.h"
#include "OVR_Types.h"
#include "OVR_Std.h"
#include "OVR_String.h"
#include "OVR_Timer.h"
#include "OVR_DebugHelp.h"
#include "OVR_Atomic.h"
#include "OVR_UTF8Util.h"
#include "OVR_Threads.h"
#include "OVR_Win32_IncludeWindows.h"
#include "Logging/Logging_Library.h"

OVR_DISABLE_ALL_MSVC_WARNINGS()
OVR_DISABLE_MSVC_WARNING(4265)
#if defined(OVR_OS_WIN32)
#include <winerror.h>
#include <dxgitype.h>
#endif
#include <utility>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <stdarg.h>
#include <cstdio>
#include <mutex>
#include <unordered_map>
OVR_RESTORE_MSVC_WARNING()
OVR_RESTORE_ALL_MSVC_WARNINGS()

OVR_DEFINE_SINGLETON(OVR::LastErrorTLS);

OVR_DISABLE_MSVC_WARNING(4996) // 'localtime': This function or variable may be unsafe.

static ovrlog::Channel Logger("Kernel:Error");

// -----------------------------------------------------------------------------
// ***** OVR_ERROR_ENABLE_BACKTRACES
//
// If defined then we record backtraces in Errors.
//
#if !defined(OVR_ERROR_ENABLE_BACKTRACES)
#if defined(OVR_BUILD_DEBUG)
#define OVR_ERROR_ENABLE_BACKTRACES 1
#endif
#endif

namespace OVR {

//-----------------------------------------------------------------------------
// LastErrorTLS

static SymbolLookup Symbols;

LastErrorTLS::LastErrorTLS() {
  Symbols.Initialize();

  // Must be at end of function
  PushDestroyCallbacks();
}

LastErrorTLS::~LastErrorTLS() {
  Symbols.Shutdown();
}

void LastErrorTLS::OnSystemDestroy() {
  delete this;
}

// This is an accessor which auto-allocates and initializes the return value if needed.
OVRError& LastErrorTLS::LastError() {
  Lock::Locker autoLock(&TheLock);

  ThreadId threadId = GetCurrentThreadId();
  auto i = TLSDictionary.Find(threadId);

  if (i == TLSDictionary.End()) {
    TLSDictionary.Add(threadId, OVRError::Success());
    i = TLSDictionary.Find(threadId);
  }

  return (*i).Second;
}

// ****** OVRFormatDateTime
//
// Prints a date/time like so:
//     Y-M-d H:M:S [ms:us:ns]
// Example output:
//     2016-12-25 8:15:01 [392:445:23]
//
// SysClockTime is of type std::chrono::time_point<std::chrono::system_clock>.
//
// To consider: Move SysClockTime and OVRFormatDateTime to OVRKernel.
//
static void OVRFormatDateTime(SysClockTime sysClockTime, OVR::String& dateTimeString) {
  // Get the basic Date and HMS time.
  char buffer[128];
  struct tm tmResult;
  const time_t cTime = std::chrono::system_clock::to_time_t(sysClockTime);

#if defined(_MSC_VER)
  localtime_s(&tmResult, &cTime);
#else
  localtime_r(&cTime, &tmResult);
#endif

  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tmResult);

  // Append milli:micro:nano time.
  std::chrono::system_clock::duration timeSinceEpoch = sysClockTime.time_since_epoch();

  std::chrono::seconds s = std::chrono::duration_cast<std::chrono::seconds>(timeSinceEpoch);
  timeSinceEpoch -= s;

  std::chrono::milliseconds ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch);
  timeSinceEpoch -= ms;

  std::chrono::microseconds us =
      std::chrono::duration_cast<std::chrono::microseconds>(timeSinceEpoch);
  timeSinceEpoch -= us;

  std::chrono::nanoseconds ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(timeSinceEpoch);

  char buffer2[384];
  sprintf(buffer2, "%s [%d:%d:%d]", buffer, (int)ms.count(), (int)us.count(), (int)ns.count());

  dateTimeString = buffer2;
}

static void OVRRemoveTrailingNewlines(String& s) {
  while (!s.IsEmpty() && ((s.Back() == '\n') || (s.Back() == '\r')))
    s.PopBack();
}

OVR_SELECTANY const int64_t OVRError::kLogLineUnset;

OVRError::OVRError() {
  Reset();
}

OVRError::OVRError(ovrResult code) : OVRError() {
  Code = code;
}

OVRError::OVRError(ovrResult code, const char* pFormat, ...) : OVRError(code) {
  va_list argList;
  va_start(argList, pFormat);
  StringBuffer strbuff;
  strbuff.AppendFormatV(pFormat, argList);
  SetDescription(strbuff.ToCStr());
  va_end(argList);
}

OVRError::OVRError(const OVRError& ovrError) {
  operator=(ovrError);
}

OVRError::OVRError(OVRError&& ovrError) {
  operator=(std::move(ovrError));
}

OVRError::~OVRError() {
  // Empty
}

OVRError& OVRError::operator=(const OVRError& ovrError) {
  ErrorString = ovrError.ErrorString;
  Code = ovrError.Code;
  SysCodeType = ovrError.SysCodeType;
  SysCode = ovrError.SysCode;
  strcpy(SysCodeStr, ovrError.SysCodeStr);
  Description = ovrError.Description;
  Context = ovrError.Context;
  OVRTime = ovrError.OVRTime;
  ClockTime = ovrError.ClockTime;
  LogLine = ovrError.LogLine;
  SourceFilePath = ovrError.SourceFilePath;
  SourceFileLine = ovrError.SourceFileLine;
  Backtrace = ovrError.Backtrace;
  AlreadyLogged = ovrError.AlreadyLogged;

  return *this;
}

OVRError& OVRError::operator=(OVRError&& ovrError) {
  ErrorString = ovrError.ErrorString;
  Code = ovrError.Code;
  SysCodeType = ovrError.SysCodeType;
  SysCode = ovrError.SysCode;
  strcpy(SysCodeStr, ovrError.SysCodeStr);
  Description = std::move(ovrError.Description);
  Context = std::move(ovrError.Context);
  OVRTime = ovrError.OVRTime;
  ClockTime = ovrError.ClockTime;
  LogLine = ovrError.LogLine;
  SourceFilePath = std::move(ovrError.SourceFilePath);
  SourceFileLine = ovrError.SourceFileLine;
  Backtrace = std::move(ovrError.Backtrace);
  AlreadyLogged = ovrError.AlreadyLogged;

  return *this;
}

void OVRError::SetCurrentValues() {
  OVRTime = Timer::GetSeconds(); // It would be better if we called ovr_GetTimeInSeconds, but that
  // doesn't have a constant header to use.

  ClockTime = std::chrono::system_clock::now();

#if defined(OVR_ERROR_ENABLE_BACKTRACES)
  if (Symbols.IsInitialized()) {
    void* addressArray[32];
    size_t n = Symbols.GetBacktrace(
        addressArray, OVR_ARRAY_COUNT(addressArray), 2, nullptr, OVR_THREADSYSID_INVALID);
    Backtrace.Clear();
    Backtrace.Append(addressArray, n);
  }
#endif
}

void OVRError::Reset() {
  ErrorString.Clear();
  Code = ovrSuccess;
  SysCodeType = ovrSysErrorCodeType::None;
  SysCode = ovrSysErrorCodeSuccess;
  SysCodeStr[0] = '\0';
  Description.Clear();
  Context.Clear();
  OVRTime = 0;
  ClockTime = SysClockTime();
  LogLine = kLogLineUnset;
  SourceFilePath.Clear();
  SourceFileLine = 0;
  Backtrace.ClearAndRelease();
  AlreadyLogged = false;
}

String OVRError::GetErrorString() const {
  if (ErrorString.GetSize() == 0) { // If not already cached...
    ErrorString.AppendString("OVR Error:\n");

    // Code
    OVR::String errorCodeString;
    GetErrorCodeString(Code, false, errorCodeString);
    ErrorString.AppendFormat("  Code: %d -- %s\n", Code, errorCodeString.ToCStr());

    // SysCode
    if (SysCode != ovrSysErrorCodeSuccess) {
      if (SysCodeStr[0]) { // If the sys error was previously set to a custom value...
        ErrorString.AppendFormat(
            "  System error: %d (%x) -- %s\n", (int)SysCode, (int)SysCode, SysCodeStr);
      } else { // Else just build it with the system error code.
        OVR::String sysErrorString;
        GetSysErrorCodeString(SysCodeType, SysCode, false, sysErrorString);
        OVRRemoveTrailingNewlines(sysErrorString);
        ErrorString.AppendFormat(
            "  System error: %d (%x) -- %s\n", (int)SysCode, (int)SysCode, sysErrorString.ToCStr());
      }
    }

    // Description
    if (Description.GetLength()) {
      ErrorString.AppendFormat("  Description: %s\n", Description.ToCStr());
    }

    // OVRTime
    ErrorString.AppendFormat("  OVRTime: %f\n", OVRTime);

    // SysClockTime
    OVR::String sysClockTimeString;
    OVRFormatDateTime(ClockTime, sysClockTimeString);
    ErrorString.AppendFormat("  Time: %s\n", sysClockTimeString.ToCStr());

    // Context
    if (Context.GetLength())
      ErrorString.AppendFormat("  Context: %s\n", Context.ToCStr());

    // If LogLine is set,
    if (LogLine != kLogLineUnset)
      ErrorString.AppendFormat("  LogLine: %lld\n", LogLine);

    // FILE/LINE
    if (SourceFilePath.GetLength())
      ErrorString.AppendFormat("  File/Line: %s:%d\n", SourceFilePath.ToCStr(), SourceFileLine);

    // Backtrace
    if (Backtrace.GetSize()) {
      // We can trace symbols in a debug build here or we can trace just addresses. See other code
      // for examples of how to trace symbols.
      ErrorString.AppendFormat("  Backtrace: ");
      for (size_t i = 0, iEnd = Backtrace.GetSize(); i != iEnd; ++i)
        ErrorString.AppendFormat(" %p", Backtrace[i]);
      ErrorString.AppendChar('\n');
    }
  }

  return OVR::String(ErrorString.ToCStr(), ErrorString.GetSize());
}

void OVRError::SetCode(ovrResult code) {
  Code = code;
}

ovrResult OVRError::GetCode() const {
  return Code;
}

void OVRError::SetSysCode(
    ovrSysErrorCodeType sysCodeType,
    ovrSysErrorCode sysCode,
    const char* sysCodeString) {
  SysCodeType = sysCodeType;
  SysCode = sysCode;
  if (sysCodeString)
    OVR_strlcpy(SysCodeStr, sysCodeString, sizeof(SysCodeStr));
}

ovrSysErrorCodeType OVRError::GetSysCodeType() const {
  return SysCodeType;
}

ovrSysErrorCode OVRError::GetSysCode() const {
  return SysCode;
}

void OVRError::SetDescription(const char* pDescription) {
  if (pDescription) {
    Description = pDescription;
    OVRRemoveTrailingNewlines(Description); // Users sometimes send text with trailing newlines,
    // which have no purpose in the error report.
  } else
    Description.Clear();
}

String OVRError::GetDescription() const {
  return Description;
}

void OVRError::SetContext(const char* pContext) {
  if (pContext) {
    Context = pContext;
    OVRRemoveTrailingNewlines(Context);
  } else
    Context.Clear();
}

String OVRError::GetContext() const {
  return Context;
}

void OVRError::SetOVRTime(double ovrTime) {
  OVRTime = ovrTime;
}

double OVRError::GetOVRTime() const {
  return OVRTime;
}

void OVRError::SetSysClockTime(const SysClockTime& clockTime) {
  ClockTime = clockTime;
}

SysClockTime OVRError::GetSysClockTime() const {
  return ClockTime;
}

void OVRError::SetLogLine(int64_t logLine) {
  LogLine = logLine;
}

int64_t OVRError::GetLogLine() const {
  return LogLine;
}

void OVRError::SetSource(const char* pSourceFilePath, int sourceFileLine) {
  if (pSourceFilePath)
    SourceFilePath = pSourceFilePath;
  else
    SourceFilePath.Clear();
  SourceFileLine = sourceFileLine;
}

std::pair<OVR::String, int> OVRError::GetSource() const {
  return std::make_pair(SourceFilePath, SourceFileLine);
}

OVRError::AddressArray OVRError::GetBacktrace() const {
  return Backtrace;
}

void LogError(OVRError& ovrError) {
  // If not already logged,
  if (!ovrError.IsAlreadyLogged()) {
    Logger.LogError(ovrError.GetErrorString());

    ovrError.SetAlreadyLogged();
  }
}

void SetError(OVRError& ovrError) {
  // Record that the current thread's last error is this error. If we wanted to support
  // chaining of errors such that multiple OVRErrors could be concurrent in a thread
  // (e.g. one that occurred deep in the call chain and a higher level version of it higher
  // in the call chain), we could handle that here.
  LastErrorTLS::GetInstance()->LastError() = ovrError;
}

static OVRErrorCallback ErrorCallback;

void SetErrorCallback(OVRErrorCallback callback) {
  ErrorCallback = callback;
}

OVRError MakeError(
    ovrResult errorCode,
    ovrSysErrorCodeType sysCodeType,
    ovrSysErrorCode sysCode,
    const char* sysCodeString, // Optional pre-generated sys error code string.
    const char* pSourceFile,
    int sourceLine,
    bool logError,
    bool assertError,
    const char* pContext,
    const char* pDescriptionFormat,
    ...) {
  OVRError ovrError(errorCode);

  ovrError.SetCurrentValues(); // Sets the current time, etc.

  ovrError.SetSysCode(sysCodeType, sysCode, sysCodeString);

  va_list argList;
  va_start(argList, pDescriptionFormat);
  StringBuffer strbuff;
  strbuff.AppendFormatV(pDescriptionFormat, argList);
  va_end(argList);
  ovrError.SetDescription(strbuff.ToCStr());

  ovrError.SetContext(pContext);

  ovrError.SetSource(pSourceFile, sourceLine);

  // Set the TLS last error.
  LastErrorTLS::GetInstance()->LastError() = ovrError;

  int silencerOptions = ovrlog::ErrorSilencer::GetSilenceOptions();

  if (silencerOptions & ovrlog::ErrorSilencer::CompletelySilenceLogs)
    logError = false;

  if (silencerOptions & ovrlog::ErrorSilencer::PreventErrorAsserts)
    assertError = false;

  if (logError)
    Logger.LogError(ovrError.GetErrorString().ToCStr());

  if (assertError) {
    // Assert in debug mode to alert unit tester/developer of the error as it occurs.
    OVR_FAIL_M(ovrError.GetErrorString().ToCStr());
  }

  // ErrorCallback should be called after LastErrorTLS is set.
  // ErrorCallback could choose to save LastErrorTLS if desired.
  if (ErrorCallback) {
    const bool quiet = !logError && !assertError;
    ErrorCallback(ovrError, quiet);
  }

  return ovrError;
}

typedef std::unordered_map<ovrResult, const char*> ResultNameMap;

static ResultNameMap& GetResultNameMap() {
  static ResultNameMap resultNameMap;
  return resultNameMap;
}

const char* GetErrorCodeName(ovrResult errorCode) {
  if (errorCode == ovrSuccess) {
    return "ovrSuccess"; // Speed up a common case
  }

  const ResultNameMap& resultNameMap = GetResultNameMap();
  ResultNameMap::const_iterator it = resultNameMap.find(errorCode);

  if (it == resultNameMap.end()) {
    OVR_FAIL_M("Unknown result code. It should have been registered");
    return "UnknownResultCode";
  }

  return it->second;
}

void OVRError::RegisterResultCodeName(ovrResult number, const char* name) {
  ResultNameMap& resultNameMap = GetResultNameMap();
  if (resultNameMap.find(number) != resultNameMap.end()) {
    OVR_FAIL_M("Duplicate result code registered");
    return;
  }
  resultNameMap[number] = name;
}

bool GetErrorCodeString(ovrResult resultIn, bool prefixErrorCode, OVR::String& sResult) {
  char codeBuffer[256];

  const char* errorCodeName = GetErrorCodeName(resultIn);

  if (prefixErrorCode) {
    snprintf(
        codeBuffer,
        OVR_ARRAY_COUNT(codeBuffer),
        "0x%llx (%lld) %s",
        (uint64_t)resultIn,
        (int64_t)resultIn,
        errorCodeName);
  } else {
    snprintf(codeBuffer, OVR_ARRAY_COUNT(codeBuffer), "%s", errorCodeName);
  }

  sResult = codeBuffer;

  return true;
}

#if defined(OVR_OS_WIN32)
static const wchar_t* OVR_DXGetErrorStringW(HRESULT dwDXGIErrorCode) {
  switch (dwDXGIErrorCode) {
    case DXGI_ERROR_DEVICE_HUNG:
      return L"DXGI_ERROR_DEVICE_HUNG"; // The application's device failed due to badly formed
    // commands sent by the application. This is an design-time
    // issue that should be investigated and fixed.
    case DXGI_ERROR_DEVICE_REMOVED:
      return L"DXGI_ERROR_DEVICE_REMOVED"; // The video card has been physically removed from the
    // system, or a driver upgrade for the video card has
    // occurred. The application should destroy and recreate
    // the device. For help debugging the problem, call
    // ID3D10Device::GetDeviceRemovedReason.
    case DXGI_ERROR_DEVICE_RESET:
      return L"DXGI_ERROR_DEVICE_RESET"; // The device failed due to a badly formed command. This is
    // a run-time issue; The application should destroy and
    // recreate the device.
    case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
      return L"DXGI_ERROR_DRIVER_INTERNAL_ERROR"; // The driver encountered a problem and was put
    // into the device removed state.
    case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
      return L"DXGI_ERROR_FRAME_STATISTICS_DISJOINT"; // An event (for example, a power cycle)
    // interrupted the gathering of presentation
    // statistics.
    case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:
      return L"DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE"; // The application attempted to acquire
    // exclusive ownership of an output, but
    // failed because some other application
    // (or device within the application)
    // already acquired ownership.
    case DXGI_ERROR_INVALID_CALL:
      return L"DXGI_ERROR_INVALID_CALL"; // The application provided invalid parameter data; this
    // must be debugged and fixed before the application is
    // released.
    case DXGI_ERROR_MORE_DATA:
      return L"DXGI_ERROR_MORE_DATA"; // The buffer supplied by the application is not big enough to
    // hold the requested data.
    case DXGI_ERROR_NONEXCLUSIVE:
      return L"DXGI_ERROR_NONEXCLUSIVE"; // A global counter resource is in use, and the Direct3D
    // device can't currently use the counter resource.
    case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
      return L"DXGI_ERROR_NOT_CURRENTLY_AVAILABLE"; // The resource or request is not currently
    // available, but it might become available
    // later.
    case DXGI_ERROR_NOT_FOUND:
      return L"DXGI_ERROR_NOT_FOUND"; // When calling IDXGIObject::GetPrivateData, the GUID passed
    // in is not recognized as one previously passed to
    // IDXGIObject::SetPrivateData or
    // IDXGIObject::SetPrivateDataInterface. When calling
    // IDXGIFactory::EnumAdapters or IDXGIAdapter::EnumOutputs,
    // the enumerated ordinal is out of range.
    case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:
      return L"DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED"; // Reserved
    case DXGI_ERROR_REMOTE_OUTOFMEMORY:
      return L"DXGI_ERROR_REMOTE_OUTOFMEMORY"; // Reserved
    case DXGI_ERROR_WAS_STILL_DRAWING:
      return L"DXGI_ERROR_WAS_STILL_DRAWING"; // The GPU was busy at the moment when a call was made
    // to perform an operation, and did not execute or
    // schedule the operation.
    case DXGI_ERROR_UNSUPPORTED:
      return L"DXGI_ERROR_UNSUPPORTED"; // The requested functionality is not supported by the
    // device or the driver.
    case DXGI_ERROR_ACCESS_LOST:
      return L"DXGI_ERROR_ACCESS_LOST"; // The desktop duplication interface is invalid. The desktop
    // duplication interface typically becomes invalid when a
    // different type of image is displayed on the desktop.
    case DXGI_ERROR_WAIT_TIMEOUT:
      return L"DXGI_ERROR_WAIT_TIMEOUT"; // The time-out interval elapsed before the next desktop
    // frame was available.
    case DXGI_ERROR_SESSION_DISCONNECTED:
      return L"DXGI_ERROR_SESSION_DISCONNECTED"; // The Remote Desktop Services session is currently
    // disconnected.
    case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:
      return L"DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE"; // The DXGI output (monitor) to which the swap
    // chain content was restricted is now
    // disconnected or changed.
    case DXGI_ERROR_CANNOT_PROTECT_CONTENT:
      return L"DXGI_ERROR_CANNOT_PROTECT_CONTENT"; // DXGI can't provide content protection on the
    // swap chain. This error is typically caused by
    // an older driver, or when you use a swap chain
    // that is incompatible with content protection.
    case DXGI_ERROR_ACCESS_DENIED:
      return L"DXGI_ERROR_ACCESS_DENIED"; // You tried to use a resource to which you did not have
    // the required access privileges. This error is most
    // typically caused when you write to a shared resource
    // with read-only access.
    case DXGI_ERROR_NAME_ALREADY_EXISTS:
      return L"DXGI_ERROR_NAME_ALREADY_EXISTS"; // The supplied name of a resource in a call to
    // IDXGIResource1::CreateSharedHandle is already
    // associated with some other resource.
    case DXGI_ERROR_SDK_COMPONENT_MISSING:
      return L"DXGI_ERROR_SDK_COMPONENT_MISSING"; // The operation depends on an SDK component that
      // is missing or mismatched.
  }

  return nullptr;
}
#endif

bool GetSysErrorCodeString(
    ovrSysErrorCodeType sysErrorCodeType,
    ovrSysErrorCode sysErrorCode,
    bool prefixErrorCode,
    OVR::String& sResult) {
  char errorBuffer[1024];
  errorBuffer[0] = '\0';

  if (prefixErrorCode) {
    char prefixBuffer[64];
    snprintf(
        prefixBuffer,
        OVR_ARRAY_COUNT(prefixBuffer),
        "0x%llx (%lld): ",
        (uint64_t)sysErrorCode,
        (int64_t)sysErrorCode);
    sResult = prefixBuffer;
  } else {
    sResult.Clear();
  }

  if (sysErrorCodeType == ovrSysErrorCodeType::Vulkan) {
    // We have a problem here in that we can't #include <vulkan.h> because this source is
    // distributed as part of a public SDK, and we don't want to create that dependency.
    // However, vulkan.h errors are stable, so we can repeat them here.
    char buffer[32];
    itoa(sysErrorCode, buffer, 10);
    const char* vkResultStr = buffer; // Default to the numerical value.

    switch (static_cast<int32_t>(sysErrorCode)) {
      case 0:
        vkResultStr = "VK_SUCESS";
        break;
      case 1:
        vkResultStr = "VK_NOT_READY";
        break;
      case 2:
        vkResultStr = "VK_TIMEOUT";
        break;
      case 3:
        vkResultStr = "VK_EVENT_SET";
        break;
      case 4:
        vkResultStr = "VK_EVENT_RESET";
        break;
      case 5:
        vkResultStr = "VK_INCOMPLETE";
        break;
      case -1:
        vkResultStr = "VK_ERROR_OUT_OF_HOST_MEMORY";
        break;
      case -2:
        vkResultStr = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        break;
      case -3:
        vkResultStr = "VK_ERROR_INITIALIZATION_FAILED";
        break;
      case -4:
        vkResultStr = "VK_ERROR_DEVICE_LOST";
        break;
      case -5:
        vkResultStr = "VK_ERROR_MEMORY_MAP_FAILED";
        break;
      case -6:
        vkResultStr = "VK_ERROR_LAYER_NOT_PRESENT";
        break;
      case -7:
        vkResultStr = "VK_ERROR_EXTENSION_NOT_PRESENT";
        break;
      case -8:
        vkResultStr = "VK_ERROR_FEATURE_NOT_PRESENT";
        break;
      case -9:
        vkResultStr = "VK_ERROR_INCOMPATIBLE_DRIVER";
        break;
      case -10:
        vkResultStr = "VK_ERROR_TOO_MANY_OBJECTS";
        break;
      case -11:
        vkResultStr = "VK_ERROR_FORMAT_NOT_SUPPORTED";
        break;
      case -12:
        vkResultStr = "VK_ERROR_FRAGMENTED_POOL";
        break;
      case -1000000000:
        vkResultStr = "VK_ERROR_SURFACE_LOST_KHR";
        break;
      case -1000000001:
        vkResultStr = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        break;
      case 1000001003:
        vkResultStr = "VK_SUBOPTIMAL_KHR";
        break;
      case -1000001004:
        vkResultStr = "VK_ERROR_OUT_OF_DATE_KHR";
        break;
      case -1000003001:
        vkResultStr = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        break;
      case -1000011001:
        vkResultStr = "VK_ERROR_VALIDATION_FAILED_EXT";
        break;
      case -1000012000:
        vkResultStr = "VK_ERROR_INVALID_SHADER_NV";
        break;
      case -1000069000:
        vkResultStr = "VK_ERROR_OUT_OF_POOL_MEMORY_KHR";
        break;
      case -1000072003:
        vkResultStr = "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR";
        break;
      case -1000174001:
        vkResultStr = "VK_ERROR_NOT_PERMITTED_EXT";
        break;
      default:
        // Use the numerical default above.
        break;
    }

    OVR::OVR_strlcat(errorBuffer, vkResultStr, sizeof(errorBuffer));
  } else if (sysErrorCodeType == ovrSysErrorCodeType::OpenGL) {
    char buffer[32];
    itoa(sysErrorCode, buffer, 10);
    const char* glResultStr = buffer; // Default to the numerical value.

    switch (sysErrorCode) {
      case 0:
        glResultStr = "GL_NO_ERROR";
        break;
      case 0x0500:
        glResultStr = "GL_INVALID_ENUM";
        break;
      case 0x0501:
        glResultStr = "GL_INVALID_VALUE";
        break;
      case 0x0502:
        glResultStr = "GL_INVALID_OPERATION";
        break;
      case 0x0503:
        glResultStr = "GL_STACK_OVERFLOW";
        break;
      case 0x0504:
        glResultStr = "GL_STACK_UNDERFLOW";
        break;
      case 0x0505:
        glResultStr = "GL_OUT_OF_MEMORY";
        break;
      default:
        // Use the numerical default above.
        break;
    }

    OVR::OVR_strlcat(errorBuffer, glResultStr, sizeof(errorBuffer));
  } else if (sysErrorCodeType == ovrSysErrorCodeType::OS) {
#if defined(OVR_OS_WIN32)
    // Note: It may be useful to use FORMAT_MESSAGE_FROM_HMODULE here to get a module-specific error
    // string if our source of errors ends up including more than just system-native errors. For
    // example, a third party module with custom errors defined in it.

    WCHAR errorBufferW[1024];
    DWORD errorBufferWCapacity = OVR_ARRAY_COUNT(errorBufferW);
    DWORD length = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        (DWORD)sysErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        errorBufferW,
        errorBufferWCapacity,
        nullptr);

    if (!length) // If failed...
    {
      if (HRESULT_FACILITY(sysErrorCode) == _FACDXGI) // If it is a DXGI error...
      {
        // This situation occurs on Windows 7. You can't use FORMAT_MESSAGE_FROM_HMODULE to solve it
        // either. We can only use DXGetErrorString or manually handle it.
        const wchar_t* pStr = OVR_DXGetErrorStringW(sysErrorCode);

        if (pStr) {
          wcscpy_s(errorBufferW, OVR_ARRAY_COUNT(errorBufferW), pStr);
          length = (DWORD)wcslen(errorBufferW);
        }
      }
    }

    if (length) // If errorBufferW contains what we are looking for...
    {
      // Need to convert WCHAR errorBuffer to UTF8 char sResult;
      const auto requiredUTF8Length =
          OVR::UTF8Util::Strlcpy(errorBuffer, OVR_ARRAY_COUNT(errorBuffer), errorBufferW);
      if (requiredUTF8Length >=
          OVR_ARRAY_COUNT(errorBuffer)) // Zero out if too big (XXX truncate instead?)
        errorBuffer[0] = '\0';
      // Else fall through
    } // Else fall through
#else
#if (((_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOURCE >= 600)) && !_GNU_SOURCE) || \
    defined(__APPLE__) || defined(__BSD__)
    const int result = strerror_r((int)sysErrorCode, errorBuffer, OVR_ARRAY_COUNT(errorBuffer));

    if (result != 0) // If failed... [result is 0 upon success; result will be EINVAL if the code is
      // not recognized; ERANGE if buffer didn't have enough capacity.]
      errorBuffer[0] = '\0'; // re-null-terminate, in case strerror_r left it in an invalid state.
#else
    const char* result = strerror_r((int)sysErrorCode, errorBuffer, OVR_ARRAY_COUNT(errorBuffer));

    if (result ==
        nullptr) // Implementations in practice seem to always return a pointer, though the
      // behavior isn't formally standardized.
      errorBuffer[0] = '\0'; // re-null-terminate, in case strerror_r left it in an invalid state.
#endif
#endif
  }

  // Fall through functionality of simply printing the value as an integer.
  if (errorBuffer[0]) // If errorBuffer was successfully written above...
  {
    sResult += errorBuffer;
    return true;
  }

  sResult += "(unknown)"; // This is not localized. Question: Is there a way to get the error
  // formatting functions above to print this themselves in a localized way?
  return false;
}

} // namespace OVR
