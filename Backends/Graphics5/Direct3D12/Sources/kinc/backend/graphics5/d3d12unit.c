// Windows 7
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601

#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
#define NOCTLMGR
#define NODEFERWINDOWPOS
#define NODRAWTEXT
#define NOGDI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOKANJI
#define NOKEYSTATES
#define NOMB
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
//#define NOMSG
#define NONLS
#define NOOPENFILE
#define NOPROFILER
#define NORASTEROPS
#define NOSCROLL
#define NOSERVICE
#define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
//#define NOUSER
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

#include <d3d12.h>
#ifdef KORE_WINDOWS
#include "d3dx12.h"
#endif

#ifndef IID_GRAPHICS_PPV_ARGS
#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)
#endif

#define QUEUE_SLOT_COUNT 2
#define textureCount 16
static int currentBackBuffer = -1;
ID3D12Device *device;
static ID3D12RootSignature *globalRootSignature = NULL;
static ID3D12RootSignature *globalComputeRootSignature = NULL;
// extern ID3D12GraphicsCommandList* commandList;

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>

#include "Direct3D12.cpp.h"
#include "ShaderHash.c.h"
#include "commandlist.cpp.h"
#include "constantbuffer.cpp.h"
#include "indexbuffer.cpp.h"
#include "pipeline.cpp.h"
#include "raytrace.cpp.h"
#include "rendertarget.cpp.h"
#include "shader.cpp.h"
#include "texture.cpp.h"
#include "vertexbuffer.cpp.h"
