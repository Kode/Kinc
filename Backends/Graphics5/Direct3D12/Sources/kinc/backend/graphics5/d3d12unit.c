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

// These following vtable-structs are broken in Windows SDKs before version 10.0.20348.0

typedef struct FixedID3D12DescriptorHeapVtbl {
	BEGIN_INTERFACE

	DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
	HRESULT(STDMETHODCALLTYPE *QueryInterface)(ID3D12DescriptorHeap *This, REFIID riid, _COM_Outptr_ void **ppvObject);

	DECLSPEC_XFGVIRT(IUnknown, AddRef)
	ULONG(STDMETHODCALLTYPE *AddRef)(ID3D12DescriptorHeap *This);

	DECLSPEC_XFGVIRT(IUnknown, Release)
	ULONG(STDMETHODCALLTYPE *Release)(ID3D12DescriptorHeap *This);

	DECLSPEC_XFGVIRT(ID3D12Object, GetPrivateData)
	HRESULT(STDMETHODCALLTYPE *GetPrivateData)
	(ID3D12DescriptorHeap *This, _In_ REFGUID guid, _Inout_ UINT *pDataSize, _Out_writes_bytes_opt_(*pDataSize) void *pData);

	DECLSPEC_XFGVIRT(ID3D12Object, SetPrivateData)
	HRESULT(STDMETHODCALLTYPE *SetPrivateData)
	(ID3D12DescriptorHeap *This, _In_ REFGUID guid, _In_ UINT DataSize, _In_reads_bytes_opt_(DataSize) const void *pData);

	DECLSPEC_XFGVIRT(ID3D12Object, SetPrivateDataInterface)
	HRESULT(STDMETHODCALLTYPE *SetPrivateDataInterface)(ID3D12DescriptorHeap *This, _In_ REFGUID guid, _In_opt_ const IUnknown *pData);

	DECLSPEC_XFGVIRT(ID3D12Object, SetName)
	HRESULT(STDMETHODCALLTYPE *SetName)(ID3D12DescriptorHeap *This, _In_z_ LPCWSTR Name);

	DECLSPEC_XFGVIRT(ID3D12DeviceChild, GetDevice)
	HRESULT(STDMETHODCALLTYPE *GetDevice)(ID3D12DescriptorHeap *This, REFIID riid, _COM_Outptr_opt_ void **ppvDevice);

	DECLSPEC_XFGVIRT(ID3D12DescriptorHeap, GetDesc)
	D3D12_DESCRIPTOR_HEAP_DESC *(STDMETHODCALLTYPE *GetDesc)(ID3D12DescriptorHeap *This, D3D12_DESCRIPTOR_HEAP_DESC *RetVal);

	DECLSPEC_XFGVIRT(ID3D12DescriptorHeap, GetCPUDescriptorHandleForHeapStart)
	D3D12_CPU_DESCRIPTOR_HANDLE *(STDMETHODCALLTYPE *GetCPUDescriptorHandleForHeapStart)(ID3D12DescriptorHeap *This, D3D12_CPU_DESCRIPTOR_HANDLE *RetVal);

	DECLSPEC_XFGVIRT(ID3D12DescriptorHeap, GetGPUDescriptorHandleForHeapStart)
	D3D12_GPU_DESCRIPTOR_HANDLE *(STDMETHODCALLTYPE *GetGPUDescriptorHandleForHeapStart)(ID3D12DescriptorHeap *This, D3D12_GPU_DESCRIPTOR_HANDLE *RetVal);

	END_INTERFACE
} FixedID3D12DescriptorHeapVtbl;

typedef struct FixedID3D12ResourceVtbl {
	BEGIN_INTERFACE

	DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
	HRESULT(STDMETHODCALLTYPE *QueryInterface)(ID3D12Resource *This, REFIID riid, _COM_Outptr_ void **ppvObject);

	DECLSPEC_XFGVIRT(IUnknown, AddRef)
	ULONG(STDMETHODCALLTYPE *AddRef)(ID3D12Resource *This);

	DECLSPEC_XFGVIRT(IUnknown, Release)
	ULONG(STDMETHODCALLTYPE *Release)(ID3D12Resource *This);

	DECLSPEC_XFGVIRT(ID3D12Object, GetPrivateData)
	HRESULT(STDMETHODCALLTYPE *GetPrivateData)
	(ID3D12Resource *This, _In_ REFGUID guid, _Inout_ UINT *pDataSize, _Out_writes_bytes_opt_(*pDataSize) void *pData);

	DECLSPEC_XFGVIRT(ID3D12Object, SetPrivateData)
	HRESULT(STDMETHODCALLTYPE *SetPrivateData)(ID3D12Resource *This, _In_ REFGUID guid, _In_ UINT DataSize, _In_reads_bytes_opt_(DataSize) const void *pData);

	DECLSPEC_XFGVIRT(ID3D12Object, SetPrivateDataInterface)
	HRESULT(STDMETHODCALLTYPE *SetPrivateDataInterface)(ID3D12Resource *This, _In_ REFGUID guid, _In_opt_ const IUnknown *pData);

	DECLSPEC_XFGVIRT(ID3D12Object, SetName)
	HRESULT(STDMETHODCALLTYPE *SetName)(ID3D12Resource *This, _In_z_ LPCWSTR Name);

	DECLSPEC_XFGVIRT(ID3D12DeviceChild, GetDevice)
	HRESULT(STDMETHODCALLTYPE *GetDevice)(ID3D12Resource *This, REFIID riid, _COM_Outptr_opt_ void **ppvDevice);

	DECLSPEC_XFGVIRT(ID3D12Resource, Map)
	HRESULT(STDMETHODCALLTYPE *Map)
	(ID3D12Resource *This, UINT Subresource, _In_opt_ const D3D12_RANGE *pReadRange,
	 _Outptr_opt_result_bytebuffer_(_Inexpressible_("Dependent on resource")) void **ppData);

	DECLSPEC_XFGVIRT(ID3D12Resource, Unmap)
	void(STDMETHODCALLTYPE *Unmap)(ID3D12Resource *This, UINT Subresource, _In_opt_ const D3D12_RANGE *pWrittenRange);

	DECLSPEC_XFGVIRT(ID3D12Resource, GetDesc)
	D3D12_RESOURCE_DESC *(STDMETHODCALLTYPE *GetDesc)(ID3D12Resource *This, D3D12_RESOURCE_DESC *RetVal);

	DECLSPEC_XFGVIRT(ID3D12Resource, GetGPUVirtualAddress)
	D3D12_GPU_VIRTUAL_ADDRESS(STDMETHODCALLTYPE *GetGPUVirtualAddress)(ID3D12Resource *This);

	DECLSPEC_XFGVIRT(ID3D12Resource, WriteToSubresource)
	HRESULT(STDMETHODCALLTYPE *WriteToSubresource)
	(ID3D12Resource *This, UINT DstSubresource, _In_opt_ const D3D12_BOX *pDstBox, _In_ const void *pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch);

	DECLSPEC_XFGVIRT(ID3D12Resource, ReadFromSubresource)
	HRESULT(STDMETHODCALLTYPE *ReadFromSubresource)
	(ID3D12Resource *This, _Out_ void *pDstData, UINT DstRowPitch, UINT DstDepthPitch, UINT SrcSubresource, _In_opt_ const D3D12_BOX *pSrcBox);

	DECLSPEC_XFGVIRT(ID3D12Resource, GetHeapProperties)
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
