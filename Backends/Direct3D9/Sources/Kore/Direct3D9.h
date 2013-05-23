#pragma once

#include <Kore/Graphics/Graphics.h>
#include <Kore/Math/Matrix.h>

#ifdef SYS_WINDOWS
#include <d3d9.h>
#endif

#ifdef SYS_XBOX360
#include <xtl.h>
#include <xboxmath.h>
#endif

extern IDirect3D9* d3d;
extern IDirect3DDevice9* device;
