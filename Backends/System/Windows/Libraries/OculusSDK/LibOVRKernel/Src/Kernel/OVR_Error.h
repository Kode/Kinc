/************************************************************************************

Filename    :   OVR_Error.h
Content     :   Structs and functions for handling OVRErrors
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

#pragma once

#include "Kernel/OVR_Log.h"
#include "Kernel/OVR_String.h"
#include "Kernel/OVR_Array.h"
#include "Kernel/OVR_System.h"
#include "Kernel/OVR_Threads.h"
#include "Kernel/OVR_Hash.h"
#include <chrono>
#include <utility>
#include <functional>

OVR_DISABLE_MSVC_WARNING(4351) // new behavior: elements of array will be default initialized

#ifndef OVR_RESULT_DEFINED
#define OVR_RESULT_DEFINED ///< Allows ovrResult to be independently defined.
/// API call results are represented at the highest level by a single ovrResult.
typedef int32_t ovrResult;
#endif

/// \brief Indicates if an ovrResult indicates success.
///
/// Some functions return additional successful values other than ovrSucces and
/// require usage of this macro to indicate successs.
///
#if !defined(OVR_SUCCESS)
#define OVR_SUCCESS(result) (result >= 0)
#endif

/// \brief Indicates if an ovrResult indicates an unqualified success.
///
/// This is useful for indicating that the code intentionally wants to
/// check for result == ovrSuccess as opposed to OVR_SUCCESS(), which
/// checks for result >= ovrSuccess.
///
#if !defined(OVR_UNQUALIFIED_SUCCESS)
#define OVR_UNQUALIFIED_SUCCESS(result) (result == ovrSuccess)
#endif

/// \brief Indicates if an ovrResult indicates failure.
///
#if !defined(OVR_FAILURE)
#define OVR_FAILURE(result) (!OVR_SUCCESS(result))
#endif

// Success is a value greater or equal to 0, while all error types are negative values.
#ifndef OVR_SUCCESS_DEFINED
#define OVR_SUCCESS_DEFINED ///< Allows ovrResult to be independently defined.
typedef enum ovrSuccessType_ {
  /// This is a general success result. Use OVR_SUCCESS to test for success.
  ovrSuccess = 0,
} ovrSuccessType;
#endif

/// -----------------------------------------------------------------------------
/// ***** OVR_FILE / OVR_LINE
///
#if !defined(OVR_FILE)
#if defined(OVR_BUILD_DEBUG)
#define OVR_FILE __FILE__
#define OVR_LINE __LINE__
#else
#define OVR_FILE nullptr
#define OVR_LINE 0
#endif
#endif

// LOG_VAARG_ATTRIBUTE macro, enforces printf-style formatting for message types
#ifdef __GNUC__
#define OVR_LOG_VAARG_ATTRIBUTE(a, b) __attribute__((format(printf, a, b)))
#else
#define OVR_LOG_VAARG_ATTRIBUTE(a, b)
#endif

namespace OVR {

/// -----------------------------------------------------------------------------
/// ***** OVR_MAKE_ERROR, OVR_MAKE_ERROR_F, OVR_MAKE_SYS_ERROR, OVR_MAKE_SYS_ERROR_F
///
/// Declaration:
///   OVRError OVR_MAKE_ERROR(ovrResult r, const char* description);
///   OVRError OVR_MAKE_ERROR_F(ovrResult r, const char* format, ...);
///
///   OVRError OVR_MAKE_SYS_ERROR(ovrResult r, ovrSysErrorCode sysCode, const char* description);
///   OVRError OVR_MAKE_SYS_ERROR_F(ovrResult r, ovrSysErrorCode sysCode, const char* format, ...);
///
/// Example usage:
///      OVRError InitGraphics()
///      {
///          if(!GraphicsCardPresent())
///          {
///              return OVR_MAKE_ERROR(ovrError_GraphicsInit, "Failed to init graphics; graphics
///                                                             support absent.");
///          }
///
///          HRESULT hr = pDevice->CreateTexture2D(&dsDesc, nullptr, &Texture);
///          if(FAILED(hr))
///          {
///              return OVR_MAKE_SYS_ERROR_F(ovrError_GraphicsInit, hr, "Failed to create texture of
///                                                size %u x %u", dsDesc.Width, dsDesc.Height);
///          }
///          or:
///              OVR_HR_CHECK_RET_ERROR(ovrError_GraphicsInit, hr, "Failed to create texture of size
///                                                %u x %u", dsDesc.Width, dsDesc.Height);
///
///          return ovrSuccess; // Converts to an OVRError instance that has no error.
///      }
///
#define OVR_MAKE_ERROR(errorCode, pDescription) \
  OVR::MakeError(                               \
      (errorCode),                              \
      OVR::ovrSysErrorCodeType::None,           \
      OVR::ovrSysErrorCodeSuccess,              \
      nullptr,                                  \
      OVR_FILE,                                 \
      OVR_LINE,                                 \
      true,                                     \
      true,                                     \
      nullptr,                                  \
      "%s",                                     \
      (pDescription))

// Note: The format string is the first part of the .../__VA_ARGS__ as per the C99-C++11 Standards.
#define OVR_MAKE_ERROR_F(errorCode, ...) \
  OVR::MakeError(                        \
      (errorCode),                       \
      OVR::ovrSysErrorCodeType::None,    \
      OVR::ovrSysErrorCodeSuccess,       \
      nullptr,                           \
      OVR_FILE,                          \
      OVR_LINE,                          \
      true,                              \
      true,                              \
      nullptr,                           \
      __VA_ARGS__)

#define OVR_MAKE_SYS_ERROR(errorCode, sysErrorCode, pDescription) \
  OVR::MakeError(                                                 \
      (errorCode),                                                \
      OVR::ovrSysErrorCodeType::OS,                               \
      (sysErrorCode),                                             \
      nullptr,                                                    \
      OVR_FILE,                                                   \
      OVR_LINE,                                                   \
      true,                                                       \
      true,                                                       \
      nullptr,                                                    \
      "%s",                                                       \
      (pDescription))

// Note: The format string is the first part of the .../__VA_ARGS__ as per the C99-C++11 Standards.
#define OVR_MAKE_SYS_ERROR_F(errorCode, sysErrorCode, ...) \
  OVR::MakeError(                                          \
      (errorCode),                                         \
      OVR::ovrSysErrorCodeType::OS,                        \
      (sysErrorCode),                                      \
      nullptr,                                             \
      OVR_FILE,                                            \
      OVR_LINE,                                            \
      true,                                                \
      true,                                                \
      nullptr,                                             \
      __VA_ARGS__)

#define OVR_MAKE_VULKAN_ERROR(errorCode, sysErrorCode, pDescription) \
  OVR::MakeError(                                                    \
      (errorCode),                                                   \
      OVR::ovrSysErrorCodeType::Vulkan,                              \
      (sysErrorCode),                                                \
      nullptr,                                                       \
      OVR_FILE,                                                      \
      OVR_LINE,                                                      \
      true,                                                          \
      true,                                                          \
      nullptr,                                                       \
      "%s",                                                          \
      (pDescription))

// Note: The format string is the first part of the .../__VA_ARGS__ as per the C99-C++11 Standards.
#define OVR_MAKE_VULKAN_ERROR_F(errorCode, sysErrorCode, ...) \
  OVR::MakeError(                                             \
      (errorCode),                                            \
      OVR::ovrSysErrorCodeType::Vulkan,                       \
      (sysErrorCode),                                         \
      nullptr,                                                \
      OVR_FILE,                                               \
      OVR_LINE,                                               \
      true,                                                   \
      true,                                                   \
      nullptr,                                                \
      __VA_ARGS__)

#define OVR_MAKE_OPENGL_ERROR(errorCode, sysErrorCode, pDescription) \
  OVR::MakeError(                                                    \
      (errorCode),                                                   \
      OVR::ovrSysErrorCodeType::OpenGL,                              \
      (sysErrorCode),                                                \
      nullptr,                                                       \
      OVR_FILE,                                                      \
      OVR_LINE,                                                      \
      true,                                                          \
      true,                                                          \
      nullptr,                                                       \
      "%s",                                                          \
      (pDescription))

// Note: The format string is the first part of the .../__VA_ARGS__ as per the C99-C++11 Standards.
#define OVR_MAKE_OPENGL_ERROR_F(errorCode, sysErrorCode, ...) \
  OVR::MakeError(                                             \
      (errorCode),                                            \
      OVR::ovrSysErrorCodeType::OpenGL,                       \
      (sysErrorCode),                                         \
      nullptr,                                                \
      OVR_FILE,                                               \
      OVR_LINE,                                               \
      true,                                                   \
      true,                                                   \
      nullptr,                                                \
      __VA_ARGS__)

// Consider using OVR_MAKE_QUIET_ERROR() instead of OVR_MAKE_ERROR() where the error
// should not automatically log/assert.  If the error is normal (HMD unplugged) or
// repetitive then please use OVR_MAKE_QUIET_ERROR().

#define OVR_MAKE_QUIET_ERROR(errorCode, pDescription) \
  OVR::MakeError(                                     \
      (errorCode),                                    \
      OVR::ovrSysErrorCodeType::None,                 \
      OVR::ovrSysErrorCodeSuccess,                    \
      nullptr,                                        \
      OVR_FILE,                                       \
      OVR_LINE,                                       \
      false,                                          \
      false,                                          \
      nullptr,                                        \
      "%s",                                           \
      (pDescription))

// Note: The format string is the first part of the .../__VA_ARGS__ as per the C99-C++11 Standards.
#define OVR_MAKE_QUIET_ERROR_F(errorCode, ...) \
  OVR::MakeError(                              \
      (errorCode),                             \
      OVR::ovrSysErrorCodeType::None,          \
      OVR::ovrSysErrorCodeSuccess,             \
      nullptr,                                 \
      OVR_FILE,                                \
      OVR_LINE,                                \
      false,                                   \
      false,                                   \
      nullptr,                                 \
      __VA_ARGS__)

#define OVR_MAKE_QUIET_SYS_ERROR(errorCode, sysErrorCode, pDescription) \
  OVR::MakeError(                                                       \
      (errorCode),                                                      \
      OVR::ovrSysErrorCodeType::OS,                                     \
      (sysErrorCode),                                                   \
      nullptr,                                                          \
      OVR_FILE,                                                         \
      OVR_LINE,                                                         \
      false,                                                            \
      false,                                                            \
      nullptr,                                                          \
      "%s",                                                             \
      (pDescription))

// Note: The format string is the first part of the .../__VA_ARGS__ as per the C99-C++11 Standards.
#define OVR_MAKE_QUIET_SYS_ERROR_F(errorCode, sysErrorCode, ...) \
  OVR::MakeError(                                                \
      (errorCode),                                               \
      ovrSysErrorCodeType::OS,                                   \
      (sysErrorCode),                                            \
      nullptr,                                                   \
      OVR_FILE,                                                  \
      OVR_LINE,                                                  \
      false,                                                     \
      false,                                                     \
      nullptr,                                                   \
      __VA_ARGS__)

// Consider using OVR_MAKE_NOASSERT_ERROR() instead of OVR_MAKE_ERROR() where the error
// should not automatically log/assert.  If the error is normal (HMD unplugged) or
// repetitive then please use OVR_MAKE_NOASSERT_ERROR().

#define OVR_MAKE_NOASSERT_ERROR(errorCode, pDescription) \
  OVR::MakeError(                                        \
      (errorCode),                                       \
      OVR::ovrSysErrorCodeType::None,                    \
      OVR::ovrSysErrorCodeSuccess,                       \
      nullptr,                                           \
      OVR_FILE,                                          \
      OVR_LINE,                                          \
      true,                                              \
      false,                                             \
      nullptr,                                           \
      "%s",                                              \
      (pDescription))

// Note: The format string is the first part of the .../__VA_ARGS__ as per the C99-C++11 Standards.
#define OVR_MAKE_NOASSERT_ERROR_F(errorCode, ...) \
  OVR::MakeError(                                 \
      (errorCode),                                \
      OVR::ovrSysErrorCodeType::None,             \
      OVR::ovrSysErrorCodeSuccess,                \
      nullptr,                                    \
      OVR_FILE,                                   \
      OVR_LINE,                                   \
      true,                                       \
      false,                                      \
      nullptr,                                    \
      __VA_ARGS__)

#define OVR_MAKE_NOASSERT_SYS_ERROR(errorCode, sysErrorCode, pDescription) \
  OVR::MakeError(                                                          \
      (errorCode),                                                         \
      OVR::ovrSysErrorCodeType::OS,                                        \
      (sysErrorCode),                                                      \
      nullptr,                                                             \
      OVR_FILE,                                                            \
      OVR_LINE,                                                            \
      true,                                                                \
      false,                                                               \
      nullptr,                                                             \
      "%s",                                                                \
      (pDescription))

// Note: The format string is the first part of the .../__VA_ARGS__ as per the C99-C++11 Standards.
#define OVR_MAKE_NOASSERT_SYS_ERROR_F(errorCode, sysErrorCode, ...) \
  OVR::MakeError(                                                   \
      (errorCode),                                                  \
      OVR::ovrSysErrorCodeType::OS,                                 \
      (sysErrorCode),                                               \
      nullptr,                                                      \
      OVR_FILE,                                                     \
      OVR_LINE,                                                     \
      true,                                                         \
      false,                                                        \
      nullptr,                                                      \
      __VA_ARGS__)

// Set the TLS last error:
#define OVR_SET_ERROR(ovrError) OVR::SetError(ovrError)

// Log the error if it has not already been logged:
#define OVR_LOG_ERROR(ovrError) OVR::LogError(ovrError)

// Check an HRESULT error code and assert/log on failure:
#define OVR_HR_CHECK_RET_ERROR(errorCode, sysErrorCode, pDescription)               \
  if (FAILED(sysErrorCode)) {                                                       \
    return OVR_MAKE_SYS_ERROR_F((errorCode), (sysErrorCode), "%s", (pDescription)); \
  }

// Note: The format string is the first part of the .../__VA_ARGS__ as per the C99-C++11 Standards.
#define OVR_HR_CHECK_RET_ERROR_F(errorCode, sysErrorCode, ...)             \
  if (FAILED(sysErrorCode)) {                                              \
    return OVR_MAKE_SYS_ERROR_F((errorCode), (sysErrorCode), __VA_ARGS__); \
  }

/// -----------------------------------------------------------------------------
/// ***** ovrSysErrorCodeType
///
enum class ovrSysErrorCodeType {
  None,
  OS, /// Windows: HRESULT or DWORD system error code from GetLastError.
  ALVR, /// AMD LiquidVR error
  NVAPI, /// NVidia API error
  NVENC, /// NVidia video encode API error
  Vulkan, /// Vulkan graphics API
  OpenGL /// OpenGL graphics API
};

/// -----------------------------------------------------------------------------
/// ***** ovrSysErrorCode
///
/// Identifies a platform- or API-specific error identifier.
/// The error space is identified by ovrSysErrorCodeType.
///
typedef uint32_t ovrSysErrorCode;

/// -----------------------------------------------------------------------------
/// ***** ovrSysErrorCodeSuccess
///
/// Identifies a ovrSysErrorCode that's success.
///
const ovrSysErrorCode ovrSysErrorCodeSuccess = 0;

/// -----------------------------------------------------------------------------
/// ***** ovrSysErrorCodeNone
///
/// Identifies a ovrSysErrorCode that's un-set.
///
const ovrSysErrorCode ovrSysErrorCodeNone = 0;

// SysClockTime is a C++11 equivalent to C time_t.
typedef std::chrono::time_point<std::chrono::system_clock> SysClockTime;

/// -----------------------------------------------------------------------------
/// ***** OVRError
///
/// Represents an error and relevant information about it.
/// While you can create error instances directly via this class, it's better if
/// you create them via the OVR_MAKE_ERROR family of macros, or at least via the
/// MakeError function.
///
/// Relevant design analogues:
///     https://developer.apple.com/library/mac/documentation/Cocoa/Reference/Foundation/Classes/NSError_Class/
///     https://msdn.microsoft.com/en-us/library/windows/desktop/ms723041%28v=vs.85%29.aspx
///
class OVRError {
 private:
  // Cannot convert boolean to OVRError - It must be done explicitly.
  OVRError(bool) {
    OVR_ASSERT(false);
  }
  OVRError(bool, const char*, ...) {
    OVR_ASSERT(false);
  }

 public:
  OVRError();
  OVRError(ovrResult code); // Intentionally not explicit.
  OVRError(ovrResult code, const char* pFormat, ...);

  OVRError(const OVRError& OVRError);
  OVRError(OVRError&& OVRError);

  virtual ~OVRError();

  // Construct a success code.  Use Succeeded() to check for success.
  static OVRError Success() {
    return OVRError();
  }

  OVRError& operator=(const OVRError& OVRError);
  OVRError& operator=(OVRError&& OVRError);

  // Use this to check if result is a success code
  bool Succeeded() const {
    return Code >= ovrSuccess;
  }
  bool Failed() const {
    return !Succeeded();
  }

  // Sets the OVRTime, ClockTime, Backtrace to current values.
  void SetCurrentValues(); // To do: Come up with a more appropiate name.

  // Clears all members to a newly default-constructed state.
  void Reset();

  // Get the full error string for this error. May include newlines.
  String GetErrorString() const;

  // Property accessors
  void SetCode(ovrResult code);
  ovrResult GetCode() const;

  // Example usage:
  //   ovrResult SomeFunction() {
  //     OVRError err = SomeOtherFunction();
  //     return err;
  //   }
  operator ovrResult() const {
    return Code;
  }

  void SetSysCode(
      ovrSysErrorCodeType sysCodeType,
      ovrSysErrorCode sysCode,
      const char* sysCodeString = nullptr);
  ovrSysErrorCodeType GetSysCodeType() const;
  ovrSysErrorCode GetSysCode() const;

  void SetDescription(const char* pDescription);
  String GetDescription() const;

  void SetContext(const char* pContext);
  String GetContext() const;

  void SetOVRTime(double ovrTime);
  double GetOVRTime() const;

  void SetSysClockTime(const SysClockTime& clockTime);
  SysClockTime GetSysClockTime() const;

  static const int64_t kLogLineUnset = -1;
  void SetLogLine(int64_t logLine);
  int64_t GetLogLine() const;

  bool IsAlreadyLogged() const {
    return AlreadyLogged;
  }
  void SetAlreadyLogged() {
    AlreadyLogged = true;
  }
  void ResetAlreadyLogged() {
    AlreadyLogged = false;
  }

  void SetSource(const char* pSourceFilePath, int sourceFileLine);
  std::pair<String, int> GetSource() const;

  typedef OVR::Array<void*> AddressArray;
  AddressArray GetBacktrace() const;

  static void RegisterResultCodeName(ovrResult number, const char* name);

 protected:
  mutable StringBuffer ErrorString; /// Complete error string describing the variables below.
  ovrResult Code; /// The main ovrResult, which is a high level error id.
  ovrSysErrorCodeType SysCodeType;
  ovrSysErrorCode SysCode; /// ovrSysErrorCodeSuccess means no system error code.
  char SysCodeStr[64]; /// String describing just the sys code error.
  String Description; /// Unlocalized error description string.
  String Context; /// Context string. For example, for a file open failure this is the file path.
  double OVRTime; /// Time when the error was generated. Same format as OVR time.
  SysClockTime ClockTime; /// Wall clock time.
  int64_t LogLine; /// Log line of the error. -1 if not set (not logged).
  String SourceFilePath; /// The __FILE__ where the error was first encountered.
  int SourceFileLine; /// The __LINE__ where the error was first encountered.
  AddressArray Backtrace; /// Backtrace at point of error. May be empty in publicly released builds.
  bool AlreadyLogged; /// Error has already been logged to avoid double-printing it.
};

/// -----------------------------------------------------------------------------
/// ***** SetError
///
/// This function sets the error as the last error via the TLS last error class.
///
void SetError(OVRError& ovrError);

/// -----------------------------------------------------------------------------
/// ***** LogError
///
/// Utility function for logging an error based on the Log subsystem.
///
void LogError(OVRError& ovrError);

/// -----------------------------------------------------------------------------
/// ***** MakeError
///
/// Utility function for making an error, logging it, and setting it as the last error for the
/// current thread. It's preferred to instead use the OVR_MAKE_ERROR macro functions, as they handle
/// file/line functionality cleanly between debug and release. The "quiet" parameter will prevent it
/// from automatically logging/asserting.
///
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
    ...) OVR_LOG_VAARG_ATTRIBUTE(8, 9);

// -----------------------------------------------------------------------------
// ***** LastErrorTLS
//
// We don't use C++11 thread-local storage nor C-style __thread/__declsped(thread)
// to manager thread-local storage, as neither of these provide a means for us
// to control the lifetime of the data. Rather it can be controlled only
// passively by the thread's lifetime. Similarly we don't use pthread_getspecific
// pthread_setspecific (and Windows equivalents) because they too don't let us
// control the lifetime of the data. Our solution is to have a map of threads
// to thread-specific data, and we can clear the entire map on ovrShutdown as-needed.
// this scheme is not as fast as the aforementioned schemes but it doesn't need to
// be fast for our use.
//
// We use pointers below instead of concrete objects because we want to have their
// lifetimes entirely controlled by ovr_Initialize/ovr_Shutdown.

class LastErrorTLS : public NewOverrideBase, public SystemSingletonBase<LastErrorTLS> {
  OVR_DECLARE_SINGLETON(LastErrorTLS);

 public:
  OVRError& LastError();

 protected:
  // Protect hash from multiple thread access.
  Lock TheLock;

  // Map thread-id to OVRError objects.
  typedef Hash<ThreadId, OVRError> TLSHash;
  TLSHash TLSDictionary;
};

/// -----------------------------------------------------------------------------
/// ***** GetErrorCodeName
///
/// Utility function which converts an ovrResult error code to a string which matches
/// errorCode's ovrResult enumeration identifier.
///
const char* GetErrorCodeName(ovrResult errorCode);

/// -----------------------------------------------------------------------------
/// ***** GetErrorCodeString
///
/// Utility function which converts an ovrResult error code to a readable string version.
///
bool GetErrorCodeString(ovrResult errorCode, bool prefixErrorCode, OVR::String& sResult);

/// -----------------------------------------------------------------------------
/// ***** GetSysErrorCodeString
///
/// Utility function which converts a system error to a string. Similar to the Windows FormatMessage
/// function and the Unix strerror_r function.
/// If prefixErrorCode is true then the string is prefixed with "<code>: "
/// Returns true if the sysErrorCode was a known valid code and a string was produced from it.
/// Else the returned string will be empty. The returned string may have tabs or newlines.
/// Users of OVR_MAKE_SYS_ERROR and MakeError don't need to call this function, as it's done
/// automatically internally.
///
bool GetSysErrorCodeString(
    ovrSysErrorCodeType sysErrorCodeType,
    ovrSysErrorCode sysErrorCode,
    bool prefixErrorCode,
    OVR::String& sResult);

//-------------------------------------------------------------------------------------
// C++ Exception state

// CPPExceptionInfo contains information about a possible exception state for the current library.
// We cannot safely use the OVRError system for this, as it could fail when we are in an exceptional
// state. It would be a non-trivial effort to make the OVRError system work in a way that doesn't
// allocate memory, for various reasons.

struct CPPExceptionInfo {
  CPPExceptionInfo() OVR_NOEXCEPT : ExceptionOccurred(false), Description() {}

  void Reset() OVR_NOEXCEPT {
    ExceptionOccurred = false;
    Description[0] = '\0';
  }

  void FromStdException(const std::exception& e) OVR_NOEXCEPT {
    ExceptionOccurred = true;
    OVR_strlcpy(Description, e.what(), sizeof(Description));
  }

  void FromString(const char* str) OVR_NOEXCEPT {
    ExceptionOccurred = true;
    OVR_strlcpy(Description, str, sizeof(Description));
  }

  // If this get set to true then it should never be set back to false
  // except by an unloading of the module (i.e. DLL) itself.
  bool ExceptionOccurred;

  // Describes the exception in a way that makes no external calls
  // nor allocates any memory.
  char Description[256];
};

extern OVR::CPPExceptionInfo gCPPExceptionInfo;

// Helper macros to make repetitive exception handling tasks non-redundant.
//
// Example usage:
//    OVR_PUBLIC_FUNCTION(void) ovr_DoSomething(ovrSession session)
//    {
//        OVR_CAPI_TRY_VOID
//        {
//            HMDState* hmds = GetHMDStateFromOvrHmd(session);
//            if (hmds)
//                hmds->DoSomething();
//        }
//        OVR_CAPI_CATCH
//    }
//
//    OVR_PUBLIC_FUNCTION(int) ovr_DoSomething(ovrSession session)
//    {
//        int result = 0;
//
//        OVR_CAPI_TRY_VALUE(result)
//        {
//            HMDState* hmds = GetHMDStateFromOvrHmd(session);
//            if (hmds)
//                result = hmds->DoSomething;
//        }
//        OVR_CAPI_CATCH
//
//        return result;
//    }
//
//    OVR_PUBLIC_FUNCTION(ovrResult) ovr_DoSomething(ovrSession session)
//    {
//        OVR_CAPI_TRY_OVRRESULT
//        {
//            HMDState* pState = GetHMDStateFromOvrHmd(session);
//            if (!pState)
//            {
//                OVR_MAKE_ERROR(ovrError_InvalidSession, L"Invalid session.");
//                return ovrError_InvalidSession;
//            }
//            return hmds->DoSomething();
//        }
//        OVR_CAPI_CATCH
//
//        return ovrError_CPPException;
//    }

#if defined(OVR_BUILD_DEBUG)

#define OVR_CAPI_TRY_VOID \
  do {                    \
  } while (false);

#define OVR_CAPI_TRY_VALUE(exceptionStateReturnValue) \
  do {                                                \
  } while (false);

#define OVR_CAPI_TRY_OVRRESULT \
  do {                         \
  } while (false);

#define OVR_CAPI_CATCH \
  do {                 \
  } while (false);

#else // OVR_BUILD_DEBUG

#define OVR_CAPI_TRY_VOID                       \
  if (OVR::gCPPExceptionInfo.ExceptionOccurred) \
    return;                                     \
  try

#define OVR_CAPI_TRY_VALUE(exceptionStateReturnValue) \
  if (OVR::gCPPExceptionInfo.ExceptionOccurred)       \
    return exceptionStateReturnValue;                 \
  try

#define OVR_CAPI_TRY_OVRRESULT                  \
  if (OVR::gCPPExceptionInfo.ExceptionOccurred) \
    return ovrError_RuntimeException;           \
  try

#define OVR_CAPI_CATCH                                                         \
  catch (const std::exception& e) {                                            \
    OVR::gCPPExceptionInfo.FromStdException(e);                                \
  }                                                                            \
  catch (...) {                                                                \
    OVR::gCPPExceptionInfo.FromString("Non-standard C++ exception occurred."); \
  }

#endif // OVR_BUILD_DEBUG

/// -----------------------------------------------------------------------------
/// ***** OVRErrorCallback
///
/// Identifies a callback error handler.
/// Callbacks were added instead of integrating logic directly in OVRError
/// to make application specific logic easier
///
typedef std::function<void(const OVRError& err, bool quiet)> OVRErrorCallback;

/// -----------------------------------------------------------------------------
/// ***** SetErrorCallback
///
/// Sets callback to be issued whenever an error is encountered.
///
void SetErrorCallback(OVRErrorCallback callback);

} // namespace OVR

#ifndef MICRO_OVR
namespace ovrlog {

template <>
LOGGING_INLINE void LogStringize(LogStringBuffer& buffer, const OVR::OVRError& first) {
  buffer.Stream << first.GetErrorString().ToCStr();
}

} // namespace ovrlog
#endif
