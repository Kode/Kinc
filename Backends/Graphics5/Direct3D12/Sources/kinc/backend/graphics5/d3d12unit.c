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
// #define NOMSG
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
// #define NOUSER
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINMESSAGES
#define NOWINOFFSETS
#define NOWINSTYLES
#define WIN32_LEAN_AND_MEAN

#include <d3d12.h>
#ifdef KORE_DIRECT3D_HAS_NO_SWAPCHAIN
struct DXGI_SWAP_CHAIN_DESC1;
#else
#include <dxgi.h>
#endif

#include "d3d12mini.h"

#ifndef IID_GRAPHICS_PPV_ARGS
#define IID_GRAPHICS_PPV_ARGS(x) IID_PPV_ARGS(x)
#endif

#define textureCount 16

ID3D12Device *device = NULL;
static ID3D12RootSignature *globalRootSignature = NULL;
static ID3D12RootSignature *globalComputeRootSignature = NULL;
// extern ID3D12GraphicsCommandList* commandList;

#include <stdbool.h>

#define MAXIMUM_WINDOWS 16

struct dx_ctx {
	int current_window;
	struct dx_window windows[MAXIMUM_WINDOWS];
};

static struct dx_ctx dx_ctx = {0};

inline struct dx_window *kinc_dx_current_window() {
	return &dx_ctx.windows[dx_ctx.current_window];
}

// These following vtable-structs are broken in Windows SDKs before version 10.0.20348.0

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

	D3D12_DESCRIPTOR_HEAP_DESC *(STDMETHODCALLTYPE *GetDesc)(ID3D12DescriptorHeap *This, D3D12_DESCRIPTOR_HEAP_DESC *RetVal);

	D3D12_CPU_DESCRIPTOR_HANDLE *(STDMETHODCALLTYPE *GetCPUDescriptorHandleForHeapStart)(ID3D12DescriptorHeap *This, D3D12_CPU_DESCRIPTOR_HANDLE *RetVal);

	D3D12_GPU_DESCRIPTOR_HANDLE *(STDMETHODCALLTYPE *GetGPUDescriptorHandleForHeapStart)(ID3D12DescriptorHeap *This, D3D12_GPU_DESCRIPTOR_HANDLE *RetVal);

	END_INTERFACE
} FixedID3D12DescriptorHeapVtbl;

typedef struct FixedID3D12ResourceVtbl {
	BEGIN_INTERFACE

	HRESULT(STDMETHODCALLTYPE *QueryInterface)(ID3D12Resource *This, REFIID riid, _COM_Outptr_ void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(ID3D12Resource *This);

	ULONG(STDMETHODCALLTYPE *Release)(ID3D12Resource *This);

	HRESULT(STDMETHODCALLTYPE *GetPrivateData)
	(ID3D12Resource *This, _In_ REFGUID guid, _Inout_ UINT *pDataSize, _Out_writes_bytes_opt_(*pDataSize) void *pData);

	HRESULT(STDMETHODCALLTYPE *SetPrivateData)(ID3D12Resource *This, _In_ REFGUID guid, _In_ UINT DataSize, _In_reads_bytes_opt_(DataSize) const void *pData);

	HRESULT(STDMETHODCALLTYPE *SetPrivateDataInterface)(ID3D12Resource *This, _In_ REFGUID guid, _In_opt_ const IUnknown *pData);

	HRESULT(STDMETHODCALLTYPE *SetName)(ID3D12Resource *This, _In_z_ LPCWSTR Name);

	HRESULT(STDMETHODCALLTYPE *GetDevice)(ID3D12Resource *This, REFIID riid, _COM_Outptr_opt_ void **ppvDevice);

	HRESULT(STDMETHODCALLTYPE *Map)
	(ID3D12Resource *This, UINT Subresource, _In_opt_ const D3D12_RANGE *pReadRange,
	 _Outptr_opt_result_bytebuffer_(_Inexpressible_("Dependent on resource")) void **ppData);

	void(STDMETHODCALLTYPE *Unmap)(ID3D12Resource *This, UINT Subresource, _In_opt_ const D3D12_RANGE *pWrittenRange);

	D3D12_RESOURCE_DESC *(STDMETHODCALLTYPE *GetDesc)(ID3D12Resource *This, D3D12_RESOURCE_DESC *RetVal);

	D3D12_GPU_VIRTUAL_ADDRESS(STDMETHODCALLTYPE *GetGPUVirtualAddress)(ID3D12Resource *This);

	HRESULT(STDMETHODCALLTYPE *WriteToSubresource)
	(ID3D12Resource *This, UINT DstSubresource, _In_opt_ const D3D12_BOX *pDstBox, _In_ const void *pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch);

	HRESULT(STDMETHODCALLTYPE *ReadFromSubresource)
	(ID3D12Resource *This, _Out_ void *pDstData, UINT DstRowPitch, UINT DstDepthPitch, UINT SrcSubresource, _In_opt_ const D3D12_BOX *pSrcBox);

	HRESULT(STDMETHODCALLTYPE *GetHeapProperties)
	(ID3D12Resource *This, _Out_opt_ D3D12_HEAP_PROPERTIES *pHeapProperties, _Out_opt_ D3D12_HEAP_FLAGS *pHeapFlags);

	END_INTERFACE
} FixedID3D12ResourceVtbl;

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

inline static D3D12_RESOURCE_DESC D3D12ResourceGetDesc(ID3D12Resource *resource) {
	FixedID3D12ResourceVtbl *vtable = (FixedID3D12ResourceVtbl *)resource->lpVtbl;
	D3D12_RESOURCE_DESC desc;
	vtable->GetDesc(resource, &desc);
	return desc;
}

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>

#include "Direct3D12.c.h"
#include "ShaderHash.c.h"
#include "commandlist.c.h"
#include "compute.c.h"
#include "constantbuffer.c.h"
#include "indexbuffer.c.h"
#include "pipeline.c.h"
#include "raytrace.c.h"
#include "rendertarget.c.h"
#include "shader.c.h"
#include "texture.c.h"
#include "vertexbuffer.c.h"
