#pragma once

#include "HolographicMain.winrt.h"
#include "DeviceResources.winrt.h"

using namespace Windows::Graphics::Holographic;
using namespace Windows::Perception::Spatial;

extern std::unique_ptr<HolographicMain> m_main;
extern HolographicSpace^ m_holographicSpace;
extern std::shared_ptr<DX::DeviceResources>  m_deviceResources;
extern SpatialLocatorAttachedFrameOfReference^ m_referenceFrame;
extern HolographicFrame^ m_currentHolographicFrame;