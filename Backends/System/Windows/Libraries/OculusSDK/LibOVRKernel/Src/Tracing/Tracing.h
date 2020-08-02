/************************************************************************************

PublicHeader:   n/a
Filename    :   Tracing.h
Content     :   Performance tracing
Created     :   December 4, 2014
Author      :   Ed Hutchins

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

#ifndef OVR_Tracing_h
#define OVR_Tracing_h

//-----------------------------------------------------------------------------------
// ***** OVR_ENABLE_ETW_TRACING definition (XXX default to on for windows builds?)
//

#ifdef OVR_OS_WIN32
#define OVR_ENABLE_ETW_TRACING
#endif

//-----------------------------------------------------------------------------------
// ***** Trace* definitions
//

#ifdef OVR_ENABLE_ETW_TRACING

#define TracingIsEnabled() (OVR_SDK_LibOVREnableBits[0] != 0)

#ifdef TRACE_STATE_CAPTURE_FUNC
// hook in our own state capture callback to record the state of all opened HMDs (supress unused
// parameter warnings with void() casts)
#define MCGEN_PRIVATE_ENABLE_CALLBACK_V2(                                                         \
    SourceId, ControlCode, Level, MatchAnyKeyword, MatchAllKeywords, FilterData, CallbackContext) \
  (void(SourceId),                                                                                \
   void(Level),                                                                                   \
   void(MatchAnyKeyword),                                                                         \
   void(MatchAllKeywords),                                                                        \
   void(FilterData),                                                                              \
   void(CallbackContext),                                                                         \
   (((ControlCode) == EVENT_CONTROL_CODE_CAPTURE_STATE) ? (TRACE_STATE_CAPTURE_FUNC) : 0))
#endif

#if !defined(_In_reads_)
// get VS2010 working
#define _In_reads_(x)
#endif

#include "LibOVREvents.h"

// Register/Unregister the OVR_SDK_LibOVR provider with ETW
// (MCGEN_PRIVATE_ENABLE_CALLBACK_V2 hooks in our state capture)
#define TraceInit()                                                       \
  do {                                                                    \
    ULONG status = EventRegisterOVR_SDK_LibOVR();                         \
    if (ERROR_SUCCESS != status) {                                        \
      LogError("[LibOVR] Failed to register ETW provider (%ul)", status); \
    }                                                                     \
  } while (0)
#define TraceFini() EventUnregisterOVR_SDK_LibOVR()

// Trace function call and return for perf, and waypoints for debug
#define TraceCall(frameIndex) EventWriteCall(__FUNCTIONW__, __LINE__, (frameIndex))
#define TraceReturn(frameIndex) EventWriteReturn(__FUNCTIONW__, __LINE__, (frameIndex))
#define TraceWaypoint(frameIndex) EventWriteWaypoint(__FUNCTIONW__, __LINE__, (frameIndex))

// DistortionRenderer events
#define TraceDistortionBegin(id, frameIndex) EventWriteDistortionBegin((id), (frameIndex))
#define TraceDistortionWaitGPU(id, frameIndex) EventWriteDistortionWaitGPU((id), (frameIndex))
#define TraceDistortionPresent(id, frameIndex) EventWriteDistortionPresent((id), (frameIndex))
#define TraceDistortionEnd(id, frameIndex) EventWriteDistortionEnd((id), (frameIndex))
#define TraceDistortionEndToEndTiming(elapsedMs) EventWriteDistortionEndToEndTiming((elapsedMs))

// Tracking Camera events
#define _TraceCameraFrameData(fn, camIdx, img) \
  fn((camIdx),                                 \
     (uint32_t)(img).FrameNumber,              \
     (img).HmdFrameNumber,                     \
     (img).ArrivalTime,                        \
     (img).CaptureTime)
#define TraceCameraFrameReceived(img) _TraceCameraFrameData(EventWriteCameraFrameReceived, 0, (img))
#define TraceCameraBeginProcessing(camIdx, img) \
  _TraceCameraFrameData(EventWriteCameraBeginProcessing, (camIdx), (img))
#define TraceCameraEndProcessing(camIdx, img) \
  _TraceCameraFrameData(EventWriteCameraEndProcessing, (camIdx), (img))
#define TraceCameraFrameRequest(requestNumber, frameCount, lastFrameNumber) \
  EventWriteCameraFrameRequest(requestNumber, frameCount, lastFrameNumber)
#define TraceCameraSkippedFrames(camIdx, skippedFrameCount) \
  EventWriteCameraSkippedFrames(camIdx, skippedFrameCount)

// Trace the interesting parts of an ovrHmdDesc structure
#define TraceHmdDesc(desc)          \
  EventWriteHmdDesc(                \
      (desc).Type,                  \
      (desc).VendorId,              \
      (desc).ProductId,             \
      (desc).SerialNumber,          \
      (desc).FirmwareMajor,         \
      (desc).FirmwareMinor,         \
      (desc).AvailableHmdCaps,      \
      (desc).AvailableTrackingCaps, \
      (desc).Resolution.w,          \
      (desc).Resolution.h)
#define TraceHmdDisplay(dpy)             \
  EventWriteHmdDisplay(                  \
      (0),                               \
      (0),                               \
      (dpy).Edid.VendorID,               \
      (dpy).Edid.ModelNumber,            \
      (dpy).DisplayIdentifier.ToCStr(),  \
      (dpy).ModelName.ToCStr(),          \
      (dpy).EdidSerialNumber.ToCStr(),   \
      (dpy).LogicalResolutionInPixels.w, \
      (dpy).LogicalResolutionInPixels.h, \
      (dpy).NativeResolutionInPixels.w,  \
      (dpy).NativeResolutionInPixels.h,  \
      0,                                 \
      0,                                 \
      (dpy).DeviceNumber,                \
      (dpy).Rotation,                    \
      (dpy).ApplicationExclusive)

// Trace part of a JSON string (events have a 64k limit)
#define TraceJSONChunk(Name, TotalChunks, ChunkSequence, TotalSize, ChunkSize, ChunkOffset, Chunk) \
  EventWriteJSONChunk(Name, TotalChunks, ChunkSequence, TotalSize, ChunkSize, ChunkOffset, Chunk)

// Trace messages from the public ovr_Trace API and our internal logger
#define TraceLogDebug(message) EventWriteLogDebugMessage(message)
#define TraceLogInfo(message) EventWriteLogInfoMessage(message)
#define TraceLogError(message) EventWriteLogErrorMessage(message)

// Trace an ovrTrackingState
#define TraceTrackingState(ts)              \
  EventWriteHmdTrackingState(               \
      (ts).HeadPose.TimeInSeconds,          \
      &(ts).HeadPose.ThePose.Orientation.x, \
      &(ts).HeadPose.ThePose.Position.x,    \
      &(ts).HeadPose.AngularVelocity.x,     \
      &(ts).HeadPose.LinearVelocity.x,      \
      0,                                    \
      0,                                    \
      (ts).StatusFlags)

#define TraceCameraBlobs(camIdx, frame)            \
  if (EventEnabledCameraBlobs()) {                 \
    const int max_blobs = 80;                      \
    int count = (frame).Blobs.GetSizeI();          \
    double x[max_blobs];                           \
    double y[max_blobs];                           \
    int size[max_blobs];                           \
    if (count > max_blobs)                         \
      count = max_blobs;                           \
    for (int i = 0; i < count; ++i) {              \
      x[i] = (frame).Blobs[i].DistortedPosition.x; \
      y[i] = (frame).Blobs[i].DistortedPosition.y; \
      size[i] = (frame).Blobs[i].BlobSize;         \
    }                                              \
    EventWriteCameraBlobs(                         \
        camIdx,                                    \
        (uint32_t)(frame).Frame->FrameNumber,      \
        (frame).Frame->ArrivalTime,                \
        (frame).Frame->Width,                      \
        (frame).Frame->Height,                     \
        count,                                     \
        x,                                         \
        y,                                         \
        size);                                     \
  } else                                           \
    ((void)0)

#define TracePosePrediction(                                                           \
    OriginalPose, PredictedPose, PredictionTimeDeltaSeconds, CurrentTimeInSeconds, id) \
  EventWritePosePrediction(                                                            \
      &(OriginalPose).Translation.x,                                                   \
      &(OriginalPose).Rotation.x,                                                      \
      &(PredictedPose).Translation.x,                                                  \
      &(PredictedPose).Rotation.x,                                                     \
      (PredictionTimeDeltaSeconds),                                                    \
      (CurrentTimeInSeconds),                                                          \
      (id))

// Trace PoseLatching CPU pinned memory write
#define TracePoseLatchCPUWrite( \
    Sequence,                   \
    Layer,                      \
    MotionSensorTime,           \
    PredictedScanlineFirst,     \
    PredictedScanlineLast,      \
    TimeToScanlineFirst,        \
    TimeToScanlineLast,         \
    StartPosition,              \
    EndPosition,                \
    StartQuat,                  \
    EndQuat)                    \
  EventWritePoseLatchCPUWrite(  \
      Sequence,                 \
      Layer,                    \
      MotionSensorTime,         \
      PredictedScanlineFirst,   \
      PredictedScanlineLast,    \
      TimeToScanlineFirst,      \
      TimeToScanlineLast,       \
      StartPosition,            \
      EndPosition,              \
      StartQuat,                \
      EndQuat)

// Trace PoseLatching GPU latch
#define TracePoseLatchGPULatchReadback( \
    Sequence,                           \
    Layer,                              \
    MotionSensorTime,                   \
    PredictedScanlineFirst,             \
    PredictedScanlineLast,              \
    TimeToScanlineFirst,                \
    TimeToScanlineLast)                 \
  EventWritePoseLatchGPULatchReadback(  \
      Sequence,                         \
      Layer,                            \
      MotionSensorTime,                 \
      PredictedScanlineFirst,           \
      PredictedScanlineLast,            \
      TimeToScanlineFirst,              \
      TimeToScanlineLast)

#define TraceVSync(VSyncTime, FrameIndex, TWGpuEndTime) \
  EventWriteVSync(VSyncTime, FrameIndex, TWGpuEndTime)

#define TraceAppCompositorFocus(Pid) EventWriteAppCompositorFocus(Pid)
#define TraceAppConnect(Pid) EventWriteAppConnect(Pid)
#define TraceAppDisconnect(Pid) EventWriteAppDisconnect(Pid)
#define TraceAppNoOp(Pid) EventWriteAppNoOp(Pid)

#define TraceLatencyTiming(LatencyTiming)   \
  EventWriteLatencyTiming(                  \
      LatencyTiming.LatencyRenderCpuBegin,  \
      LatencyTiming.LatencyRenderCpuEnd,    \
      LatencyTiming.LatencyRenderIMU,       \
      LatencyTiming.LatencyTimewarpCpu,     \
      LatencyTiming.LatencyTimewarpLatched, \
      LatencyTiming.LatencyTimewarpGpuEnd,  \
      LatencyTiming.LatencyPostPresent,     \
      LatencyTiming.ErrorRender,            \
      LatencyTiming.ErrorTimewarp)

#define TraceEndFrameAppTiming(AppTiming, RenderCount) \
  EventWriteEndFrameAppTiming(                         \
      AppTiming.AppFrameIndex,                         \
      AppTiming.AppRenderIMUTime,                      \
      AppTiming.AppVisibleMidpointTime,                \
      AppTiming.AppGpuRenderDuration,                  \
      AppTiming.AppBeginRenderingTime,                 \
      AppTiming.AppEndRenderingTime,                   \
      AppTiming.QueueAheadSeconds,                     \
      RenderCount)
#define TraceEndFrameOrigAppTiming(AppTiming, RenderCount) \
  EventWriteEndFrameOrigAppTiming(                         \
      AppTiming.AppFrameIndex,                             \
      AppTiming.AppRenderIMUTime,                          \
      AppTiming.AppVisibleMidpointTime,                    \
      AppTiming.AppGpuRenderDuration,                      \
      AppTiming.AppBeginRenderingTime,                     \
      AppTiming.AppEndRenderingTime,                       \
      AppTiming.QueueAheadSeconds,                         \
      RenderCount)

// XXX for future reference this could have been done with events with different opcodes and
// identical templates
#define VirtualDisplayPacketTrace_Begin 0
#define VirtualDisplayPacketTrace_End 1
#define VirtualDisplayPacketTrace_Queue 2
#define VirtualDisplayPacketTrace_QueueRelease 3
#define VirtualDisplayPacketTrace_Result 5

#define TraceVirtualDisplayPacket(PacketType, Stage, SubmittingProcessID, ActiveProcessID) \
  EventWriteVirtualDisplayPacketTrace(PacketType, Stage, SubmittingProcessID, ActiveProcessID)
#define TraceClientFrameMissed(FrameIndex, ProcessID) \
  EventWriteClientFrameMissed(FrameIndex, ProcessID)
#define TraceCompositionBegin(ExpectedCPUStartTimeInSeconds, ActualCPUStartTimeInSeconds) \
  EventWriteCompositionBegin(ExpectedCPUStartTimeInSeconds, ActualCPUStartTimeInSeconds)
#define TraceCompositionEnd() EventWriteCompositionEnd()
#define TraceCompositionEndSpinWait() EventWriteCompositionEndSpinWait()
#define TraceCompositionFlushingToGPU() EventWriteCompositionFlushingToGPU()
#define TraceRenderPacket(Stage, ClientPID) EventWriteRenderPacketTrace(Stage, ClientPID)
#define TraceHardwareInfo(data) \
  EventWriteHardwareInfo(       \
      data.RequestedBits,       \
      data.CollectedBits,       \
      data.ImuTemp,             \
      data.StmTemp,             \
      data.NrfTemp,             \
      data.VBusVoltage,         \
      data.IAD,                 \
      data.Proximity,           \
      data.PanelOnTime,         \
      data.UseRolling,          \
      data.HighBrightness,      \
      data.DP,                  \
      data.SelfRefresh,         \
      data.Persistence,         \
      data.LightingOffset,      \
      data.PixelSettle,         \
      data.TotalRows)

#define TraceCompositionMissedCompositorFrame() EventWriteCompositionMissedCompositorFrame()
#define TraceCompositionGPUStartTime(Seconds) EventWriteCompositionGPUStartTime(Seconds)

#define TraceNotificationEnd(                                                        \
    AppFrameIndex, CpuBeginToGpuEndSeconds, CpuBeginSeconds, GpuEndSeconds, SleepMs) \
  EventWriteNotificationEnd(                                                         \
      AppFrameIndex, CpuBeginToGpuEndSeconds, CpuBeginSeconds, GpuEndSeconds, SleepMs)
#define TraceNotificationBegin( \
    AppFrameIndex,              \
    CpuBeginToGpuEndSeconds,    \
    CompositeTimeSeconds,       \
    VSyncTimeSeconds,           \
    CompositeDeltaSeconds,      \
    VSyncDeltaSeconds)          \
  EventWriteNotificationBegin(  \
      AppFrameIndex,            \
      CpuBeginToGpuEndSeconds,  \
      CompositeTimeSeconds,     \
      VSyncTimeSeconds,         \
      CompositeDeltaSeconds,    \
      VSyncDeltaSeconds)
#define TraceNotificationCompSubmit(IsEnabled, IsDisabled, FrameIndex) \
  EventWriteNotificationCompSubmit(IsEnabled, IsDisabled, FrameIndex)

#define TraceMotionEstimationCostStats(Count, Average, Log2Histogram) \
  EventWriteMotionEstimationCostStats(Count, Average, Log2Histogram)

#else // OVR_ENABLE_ETW_TRACING

// Eventually other platforms could support their form of performance tracing
#define TracingIsEnabled() (false)
#define TraceInit() ((void)0)
#define TraceFini() ((void)0)
#define TraceCall(frameIndex) ((void)0)
#define TraceReturn(frameIndex) ((void)0)
#define TraceWaypoint(frameIndex) ((void)0)
#define TraceDistortionBegin(id, frameIndex) ((void)0)
#define TraceDistortionWaitGPU(id, frameIndex) ((void)0)
#define TraceDistortionPresent(id, frameIndex) ((void)0)
#define TraceDistortionEnd(id, frameIndex) ((void)0)
#define TraceDistortionEndToEndTiming(elapsedMs) ((void)0)
#define TraceCameraFrameReceived(cfd) ((void)0)
#define TraceCameraBeginProcessing(camIdx, img) ((void)0)
#define TraceCameraFrameRequest(requestNumber, frameCount, lastFrameNumber) ((void)0)
#define TraceCameraEndProcessing(camIdx, img) ((void)0)
#define TraceCameraSkippedFrames(camIdx, skippedFrameCount) ((void)0)
#define TraceHmdDesc(desc) ((void)0)
#define TraceHmdDisplay(dpy) ((void)0)
#define TraceJSONChunk(Name, TotalChunks, ChunkSequence, TotalSize, ChunkSize, ChunkOffset, Chunk) \
  ((void)0)
#define TraceLogDebug(message) ((void)0)
#define TraceLogInfo(message) ((void)0)
#define TraceLogError(message) ((void)0)
#define TraceTrackingState(ts) ((void)0)
#define TraceCameraBlobs(camIdx, frame) ((void)0)
#define TracePoseLatchCPUWrite( \
    Sequence,                   \
    Layer,                      \
    MotionSensorTime,           \
    PredictedScanlineFirst,     \
    PredictedScanlineLast,      \
    TimeToScanlineFirst,        \
    TimeToScanlineLast,         \
    StartPosition,              \
    EndPosition,                \
    StartQuat,                  \
    EndQuat)                    \
  ((void)0)
#define TracePoseLatchGPULatchReadback( \
    Sequence,                           \
    Layer,                              \
    MotionSensorTime,                   \
    PredictedScanlineFirst,             \
    PredictedScanlineLast,              \
    TimeToScanlineFirst,                \
    TimeToScanlineLast)                 \
  ((void)0)
#define TraceVSync(VSyncTime, FrameIndex, TWGpuEndTime) ((void)0)
#define TracePosePrediction(                                                           \
    OriginalPose, PredictedPose, PredictionTimeDeltaSeconds, CurrentTimeInSeconds, id) \
  ((void)0)
#define TraceAppCompositorFocus(Pid) ((void)0)
#define TraceAppConnect(Pid) ((void)0)
#define TraceAppDisconnect(Pid) ((void)0)
#define TraceAppNoOp(Pid) ((void)0)
#define TraceLatencyTiming(LatencyTiming) ((void)0)
#define TraceEndFrameAppTiming(AppTiming, RenderCount) ((void)0)
#define TraceEndFrameOrigAppTiming(AppTiming, RenderCount) ((void)0)
#define TraceVirtualDisplayPacket(PacketType, Stage, SubmittingProcessID, ActiveProcessID) ((void)0)
#define TraceClientFrameMissed(FrameIndex, ProcessID) ((void)0)
#define TraceCompositionBegin(ExpectedCPUStartTimeInSeconds, ActualCPUStartTimeInSeconds) ((void)0)
#define TraceCompositionEnd() ((void)0)
#define TraceCompositionEndSpinWait() ((void)0)
#define TraceCompositionFlushingToGPU() ((void)0)
#define TraceRenderPacket(Stage, ClientPID) ((void)0)
#define TraceHardwareInfo(data) ((void)0)
#define TraceCompositionMissedCompositorFrame() ((void)0)
#define TraceCompositionGPUStartTime(Seconds) ((void)0)
#define TraceNotificationEnd(                                                        \
    AppFrameIndex, CpuBeginToGpuEndSeconds, CpuBeginSeconds, GpuEndSeconds, SleepMs) \
  ((void)0)
#define TraceNotificationBegin( \
    AppFrameIndex,              \
    CpuBeginToGpuEndSeconds,    \
    CompositeTimeSeconds,       \
    VSyncTimeSeconds,           \
    CompositeDeltaSeconds,      \
    VSyncDeltaSeconds)          \
  ((void)0)
#define TraceNotificationCompSubmit(IsEnabled, IsDisabled, FrameIndex) ((void)0)
#define TraceMotionEstimationCostStats(Count, Average, Log2Histogram) ((void)0)

#endif // OVR_ENABLE_ETW_TRACING

#endif // OVR_Tracing_h
