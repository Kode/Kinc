/********************************************************************************/ /**
 \file  OVR_ErrorCode.h
 \brief     This header provides LibOVR error code declarations.
 \copyright Copyright 2015-2016 Oculus VR, LLC All Rights reserved.
 *************************************************************************************/

#ifndef OVR_ErrorCode_h
#define OVR_ErrorCode_h

#include "OVR_Version.h"
#include <stdint.h>



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

// Public success types
// Success is a value greater or equal to 0, while all error types are negative values.
typedef enum ovrSuccessTypes_ {
  /// Returned from a call to SubmitFrame. The call succeeded, but what the app
  /// rendered will not be visible on the HMD. Ideally the app should continue
  /// calling SubmitFrame, but not do any rendering. When the result becomes
  /// ovrSuccess, rendering should continue as usual.
  ovrSuccess_NotVisible = 1000,

  /// Boundary is invalid due to sensor change or was not setup.
  ovrSuccess_BoundaryInvalid = 1001,

  /// Device is not available for the requested operation.
  ovrSuccess_DeviceUnavailable = 1002,

} ovrSuccessTypes;

// Public error types
typedef enum ovrErrorType_ {
  /******************/
  /* General errors */
  /******************/

  /// Failure to allocate memory.
  ovrError_MemoryAllocationFailure = -1000,

  /// Invalid ovrSession parameter provided.
  ovrError_InvalidSession = -1002,

  /// The operation timed out.
  ovrError_Timeout = -1003,

  /// The system or component has not been initialized.
  ovrError_NotInitialized = -1004,

  /// Invalid parameter provided. See error info or log for details.
  ovrError_InvalidParameter = -1005,

  /// Generic service error. See error info or log for details.
  ovrError_ServiceError = -1006,

  /// The given HMD doesn't exist.
  ovrError_NoHmd = -1007,

  /// Function call is not supported on this hardware/software
  ovrError_Unsupported = -1009,

  /// Specified device type isn't available.
  ovrError_DeviceUnavailable = -1010,

  /// The headset was in an invalid orientation for the requested
  /// operation (e.g. vertically oriented during ovr_RecenterPose).
  ovrError_InvalidHeadsetOrientation = -1011,

  /// The client failed to call ovr_Destroy on an active session before calling ovr_Shutdown.
  /// Or the client crashed.
  ovrError_ClientSkippedDestroy = -1012,

  /// The client failed to call ovr_Shutdown or the client crashed.
  ovrError_ClientSkippedShutdown = -1013,

  ///< The service watchdog discovered a deadlock.
  ovrError_ServiceDeadlockDetected = -1014,

  ///< Function call is invalid for object's current state
  ovrError_InvalidOperation = -1015,

  ///< Increase size of output array
  ovrError_InsufficientArraySize = -1016,

  /// There is not any external camera information stored by ovrServer.
  ovrError_NoExternalCameraInfo = -1017,

  /// Tracking is lost when ovr_GetDevicePoses() is called.
  ovrError_LostTracking = -1018,

  /*************************************************/
  /* Audio error range, reserved for Audio errors. */
  /*************************************************/

  /// Failure to find the specified audio device.
  ovrError_AudioDeviceNotFound = -2001,

  /// Generic COM error.
  ovrError_AudioComError = -2002,

  /**************************/
  /* Initialization errors. */
  /**************************/

  /// Generic initialization error.
  ovrError_Initialize = -3000,

  /// Couldn't load LibOVRRT.
  ovrError_LibLoad = -3001,

  /// LibOVRRT version incompatibility.
  ovrError_LibVersion = -3002,

  /// Couldn't connect to the OVR Service.
  ovrError_ServiceConnection = -3003,

  /// OVR Service version incompatibility.
  ovrError_ServiceVersion = -3004,

  /// The operating system version is incompatible.
  ovrError_IncompatibleOS = -3005,

  /// Unable to initialize the HMD display.
  ovrError_DisplayInit = -3006,

  /// Unable to start the server. Is it already running?
  ovrError_ServerStart = -3007,

  /// Attempting to re-initialize with a different version.
  ovrError_Reinitialization = -3008,

  /// Chosen rendering adapters between client and service do not match
  ovrError_MismatchedAdapters = -3009,

  /// Calling application has leaked resources
  ovrError_LeakingResources = -3010,

  /// Client version too old to connect to service
  ovrError_ClientVersion = -3011,

  /// The operating system is out of date.
  ovrError_OutOfDateOS = -3012,

  /// The graphics driver is out of date.
  ovrError_OutOfDateGfxDriver = -3013,

  /// The graphics hardware is not supported
  ovrError_IncompatibleGPU = -3014,

  /// No valid VR display system found.
  ovrError_NoValidVRDisplaySystem = -3015,

  /// Feature or API is obsolete and no longer supported.
  ovrError_Obsolete = -3016,

  /// No supported VR display system found, but disabled or driverless adapter found.
  ovrError_DisabledOrDefaultAdapter = -3017,

  /// The system is using hybrid graphics (Optimus, etc...), which is not support.
  ovrError_HybridGraphicsNotSupported = -3018,

  /// Initialization of the DisplayManager failed.
  ovrError_DisplayManagerInit = -3019,

  /// Failed to get the interface for an attached tracker
  ovrError_TrackerDriverInit = -3020,

  /// LibOVRRT signature check failure.
  ovrError_LibSignCheck = -3021,

  /// LibOVRRT path failure.
  ovrError_LibPath = -3022,

  /// LibOVRRT symbol resolution failure.
  ovrError_LibSymbols = -3023,

  /// Failed to connect to the service because remote connections to the service are not allowed.
  ovrError_RemoteSession = -3024,

  /// Vulkan initialization error.
  ovrError_InitializeVulkan = -3025,

  /********************/
  /* Rendering errors */
  /********************/

  /// In the event of a system-wide graphics reset or cable unplug this is returned to the app.
  ovrError_DisplayLost = -6000,

  /// ovr_CommitTextureSwapChain was called too many times on a texture swapchain without
  /// calling submit to use the chain.
  ovrError_TextureSwapChainFull = -6001,

  /// The ovrTextureSwapChain is in an incomplete or inconsistent state.
  /// Ensure ovr_CommitTextureSwapChain was called at least once first.
  ovrError_TextureSwapChainInvalid = -6002,

  /// Graphics device has been reset (TDR, etc...)
  ovrError_GraphicsDeviceReset = -6003,

  /// HMD removed from the display adapter
  ovrError_DisplayRemoved = -6004,

  /// Content protection is not available for the display.
  ovrError_ContentProtectionNotAvailable = -6005,

  /// Application declared itself as an invisible type and is not allowed to submit frames.
  ovrError_ApplicationInvisible = -6006,

  /// The given request is disallowed under the current conditions.
  ovrError_Disallowed = -6007,

  /// Display portion of HMD is plugged into an incompatible port (ex: IGP)
  ovrError_DisplayPluggedIncorrectly = -6008,

  /****************/
  /* Fatal errors */
  /****************/

  ///< A runtime exception occurred. The application is required to shutdown LibOVR and
  /// re-initialize it before this error state will be cleared.
  ovrError_RuntimeException = -7000,

  /**********************/
  /* Calibration errors */
  /**********************/

  /// Result of a missing calibration block
  ovrError_NoCalibration = -9000,

  /// Result of an old calibration block
  ovrError_OldVersion = -9001,

  /// Result of a bad calibration block due to lengths
  ovrError_MisformattedBlock = -9002,

/****************/
/* Other errors */
/****************/


} ovrErrorType;

/// Provides information about the last error.
/// \see ovr_GetLastErrorInfo
typedef struct ovrErrorInfo_ {
  /// The result from the last API call that generated an error ovrResult.
  ovrResult Result;

  /// A UTF8-encoded null-terminated English string describing the problem.
  /// The format of this string is subject to change in future versions.
  char ErrorString[512];
} ovrErrorInfo;

#endif /* OVR_ErrorCode_h */
