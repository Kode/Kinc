#pragma once

#include <Kore/Graphics4/Graphics.h>
#include <Kore/Math/Matrix.h>

#ifdef KORE_WINDOWS
#include <d3d9.h>
#endif

extern IDirect3D9* d3d;
extern IDirect3DDevice9* device;
