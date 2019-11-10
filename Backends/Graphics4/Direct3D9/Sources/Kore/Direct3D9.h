#pragma once

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Matrix.h>

#ifndef NDEBUG
#define D3D_DEBUG_INFO
#endif
#include <d3d9.h>

extern IDirect3D9 *d3d;
extern IDirect3DDevice9 *device;
