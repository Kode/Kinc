#pragma once

#ifdef KORE_HOLOLENS 

#include "HolographicFrameController.winrt.h"

extern std::unique_ptr<HolographicFrameController> holographicFrameController;
extern std::shared_ptr<VideoFrameProcessor> videoFrameProcessor;

#endif