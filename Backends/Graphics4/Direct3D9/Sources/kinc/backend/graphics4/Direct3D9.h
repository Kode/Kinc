#pragma once

#include <kinc/graphics4/graphics.h>
#include <kinc/math/matrix.h>

#ifndef NDEBUG
#define D3D_DEBUG_INFO
#endif
#include <d3d9.h>

extern IDirect3D9 *d3d;
extern IDirect3DDevice9 *device;
