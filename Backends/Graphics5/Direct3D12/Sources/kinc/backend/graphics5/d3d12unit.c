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

#ifndef IID_GRAPHICS_PPV_ARGS
#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)
#endif

#define QUEUE_SLOT_COUNT 2
#define textureCount 16
static int currentBackBuffer = -1;
ID3D12Device *device = NULL;
static ID3D12RootSignature *globalRootSignature = NULL;
static ID3D12RootSignature *globalComputeRootSignature = NULL;
// extern ID3D12GraphicsCommandList* commandList;

// This vtable-struct is broken in the original D3D12 header
typedef struct FixedID3D12DescriptorHeapVtbl {
	BEGIN_INTERFACE

	HRESULT(STDMETHODCALLTYPE *QueryInterface)(ID3D12DescriptorHeap *This, REFIID riid, _COM_Outptr_ void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(ID3D12DescriptorHeap *This);

	ULONG(STDMETHODCALLTYPE *Release)(ID3D12DescriptorHeap *This);

	HRESULT(STDMETHODCALLTYPE *GetPrivateData)
	(ID3D12DescriptorHeap *This, _In_ REFGUID guid, _Inout_ UINT *pDataSize, _Out_writes_bytes_opt_(*pDataSize) void *pData);

	HRESULT(STDMETHODCALLTYPE *SetPrivateData)
	(ID3D12DescriptorHeap *This, _In_ REFGUID guid, _In_ UINT DataSize, _In_reads_bytes_opt_(DataSize) const void *pData);

	HRESULT(STDMETHODCALLTYPE *SetPrivateDataInterface)(ID3D12DescriptorHeap *This, _In_ REFGUID guid, _In_opt_ const IUnknown *pData);

	HRESULT(STDMETHODCALLTYPE *SetName)(ID3D12DescriptorHeap *This, _In_z_ LPCWSTR Name);

	HRESULT(STDMETHODCALLTYPE *GetDevice)(ID3D12DescriptorHeap *This, REFIID riid, _COM_Outptr_opt_ void **ppvDevice);

	D3D12_DESCRIPTOR_HEAP_DESC(STDMETHODCALLTYPE *GetDesc)(ID3D12DescriptorHeap *This);

	D3D12_CPU_DESCRIPTOR_HANDLE(STDMETHODCALLTYPE *GetCPUDescriptorHandleForHeapStart)(ID3D12DescriptorHeap *This, D3D12_CPU_DESCRIPTOR_HANDLE *pOut);

	D3D12_GPU_DESCRIPTOR_HANDLE(STDMETHODCALLTYPE *GetGPUDescriptorHandleForHeapStart)(ID3D12DescriptorHeap *This, D3D12_GPU_DESCRIPTOR_HANDLE *pOut);

	END_INTERFACE
} FixedID3D12DescriptorHeapVtbl;

inline static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap *heap) {
	FixedID3D12DescriptorHeapVtbl *vtable = (FixedID3D12DescriptorHeapVtbl *)heap->lpVtbl;
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	vtable->GetCPUDescriptorHandleForHeapStart(heap, &handle);
	return handle;
}

inline static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap *heap) {
	FixedID3D12DescriptorHeapVtbl *vtable = (FixedID3D12DescriptorHeapVtbl *)heap->lpVtbl;
	D3D12_GPU_DESCRIPTOR_HANDLE handle;
	vtable->GetGPUDescriptorHandleForHeapStart(heap, &handle);
	return handle;
}

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
