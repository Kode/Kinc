#include <kinc/graphics4/graphics.h>

#define NOMINMAX

#include <kinc/backend/SystemMicrosoft.h>

#ifdef KORE_WINDOWSAPP
#include <d3d11_1.h>
#else
#pragma warning(disable : 4005)
#include <d3d11.h>
#endif

ID3D11Device *device;
ID3D11DeviceContext *context;
static ID3D11RenderTargetView *renderTargetView;
static ID3D11Texture2D *depthStencil;
static ID3D11DepthStencilView *depthStencilView;
static ID3D11Texture2D *backBuffer;

static int renderTargetWidth = 4096;
static int renderTargetHeight = 4096;
static int newRenderTargetWidth = 4096;
static int newRenderTargetHeight = 4096;

static uint8_t vertexConstants[1024 * 4];
static uint8_t fragmentConstants[1024 * 4];
static uint8_t geometryConstants[1024 * 4];
static uint8_t tessControlConstants[1024 * 4];
static uint8_t tessEvalConstants[1024 * 4];

#include "Direct3D11.cpp.h"
#include "ShaderHash.c.h"
#include "indexbuffer.cpp.h"
#include "pipeline.cpp.h"
#include "rendertarget.cpp.h"
#include "shader.cpp.h"
#include "texture.cpp.h"
#include "texturearray.cpp.h"
#include "vertexbuffer.cpp.h"
