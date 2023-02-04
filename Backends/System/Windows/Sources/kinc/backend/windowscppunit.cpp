#ifdef KORE_DIRECT3D12
// Windows 10
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#else
// Windows 7
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#endif

#define NOATOM
// #define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
// #define NOCTLMGR
#define NODEFERWINDOWPOS
#define NODRAWTEXT
// #define NOGDI
#define NOGDICAPMASKS
#define NOHELP
#define NOICONS
#define NOKANJI
#define NOKEYSTATES
// #define NOMB
#define NOMCX
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
// #define NOMSG
// #define NONLS
#define NOOPENFILE
#define NOPROFILER
#define NORASTEROPS
#define NOSCROLL
#define NOSERVICE
// #define NOSHOWWINDOW
#define NOSOUND
// #define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
// #define NOUSER
// #define NOVIRTUALKEYCODES
#define NOWH
// #define NOWINMESSAGES
// #define NOWINOFFSETS
// #define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include "VrInterface_SteamVR.cpp.h"
#include "video.cpp.h"
