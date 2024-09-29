#ifndef KOPE_D3D12_UNIT_HEADER
#define KOPE_D3D12_UNIT_HEADER

// Windows 7
#define WINVER 0x0601
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0601

#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
#ifndef KOPE_PIX
#define NOCTLMGR
#endif
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
// #define NOMSG
#define NONLS
#define NOOPENFILE
#define NOPROFILER
#define NORASTEROPS
#define NOSCROLL
#define NOSERVICE
#ifndef KOPE_PIX
#define NOSHOWWINDOW
#endif
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
// #define NOUSER
#define NOVIRTUALKEYCODES
#define NOWH
#ifndef KOPE_PIX
#define NOWINMESSAGES
#endif
#define NOWINOFFSETS
#define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

#include <d3d12.h>
#include <dxgi.h>

#ifndef IID_GRAPHICS_PPV_ARGS
#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)
#endif

// https://learn.microsoft.com/en-us/windows/win32/direct3d12/subresources
static inline UINT D3D12CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize) {
	return MipSlice + (ArraySlice * MipLevels) + (PlaneSlice * MipLevels * ArraySize);
}

#endif
