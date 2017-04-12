#include "pch.h"

#include "DirectShow.h"

#include <Windows.h>
#include <stdio.h>

#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}

typedef INonDelegatingUnknown *PNDUNKNOWN;

CBaseObject::CBaseObject(__in_opt const wchar_t* pName)
{
	/* Increment the number of active objects */
	InterlockedIncrement(&m_cObjects);

#ifdef DEBUG

#ifdef UNICODE
	m_dwCookie = DbgRegisterObjectCreation(0, pName);
#else
	m_dwCookie = DbgRegisterObjectCreation(pName, 0);
#endif

#endif
}

CUnknown::CUnknown(__in_opt const wchar_t* pName, __in_opt LPUNKNOWN pUnk)
	: CBaseObject(pName), m_cRef(0),
	m_pUnknown(pUnk != 0 ? pUnk : reinterpret_cast<LPUNKNOWN>(static_cast<PNDUNKNOWN>(this)))
{ }

CBaseFilter::CBaseFilter(__in_opt const wchar_t* pName,
	__in_opt LPUNKNOWN  pUnk,
	__in CCritSec   *pLock,
	REFCLSID   clsid) :
	CUnknown(pName, pUnk),
	m_pLock(pLock),
	m_clsid(clsid),
	m_State(State_Stopped),
	m_pClock(NULL),
	m_pGraph(NULL),
	m_pSink(NULL),
	m_pName(NULL),
	m_PinVersion(1)
{
#ifdef DXMPERF
	PERFLOG_CTOR(L"CBaseFilter", (IBaseFilter *) this);
#endif // DXMPERF

	ASSERT(pLock != NULL);
}

STDMETHODIMP
CBaseFilter::JoinFilterGraph(
	__inout_opt IFilterGraph * pGraph,
	__in_opt LPCWSTR pName)
{
	CAutoLock cObjectLock(m_pLock);

	// NOTE: we no longer hold references on the graph (m_pGraph, m_pSink)

	m_pGraph = pGraph;
	if (m_pGraph) {
		HRESULT hr = m_pGraph->QueryInterface(IID_IMediaEventSink,
			(void**)&m_pSink);
		if (FAILED(hr)) {
			ASSERT(m_pSink == NULL);
		}
		else m_pSink->Release();        // we do NOT keep a reference on it.
	}
	else {
		// if graph pointer is null, then we should
		// also release the IMediaEventSink on the same object - we don't
		// refcount it, so just set it to null
		m_pSink = NULL;
	}


	if (m_pName) {
		delete[] m_pName;
		m_pName = NULL;
	}

	if (pName) {
		size_t namelen;
		HRESULT hr = StringCchLengthW(pName, STRSAFE_MAX_CCH, &namelen);
		if (FAILED(hr)) {
			return hr;
		}
		m_pName = new WCHAR[namelen + 1];
		if (m_pName) {
			(void)StringCchCopyW(m_pName, namelen + 1, pName);
		}
		else {
			return E_OUTOFMEMORY;
		}
	}

#ifdef DXMPERF
	PERFLOG_JOINGRAPH(m_pName ? m_pName : L"CBaseFilter", (IBaseFilter *) this, pGraph);
#endif // DXMPERF

	return NOERROR;
}

STDMETHODIMP
CBaseFilter::QueryVendorInfo(
	__deref_out LPWSTR* pVendorInfo)
{
	UNREFERENCED_PARAMETER(pVendorInfo);
	return E_NOTIMPL;
}

CBaseRenderer::CBaseRenderer(REFCLSID RenderClass, // CLSID for this renderer
	__in_opt LPCTSTR pName,         // Debug ONLY description
	__inout_opt LPUNKNOWN pUnk,       // Aggregated owner object
	__inout HRESULT *phr) :       // General OLE return code

	CBaseFilter(pName, pUnk, &m_InterfaceLock, RenderClass),
	m_evComplete(TRUE, phr),
	m_RenderEvent(FALSE, phr),
	m_bAbort(FALSE),
	m_pPosition(NULL),
	m_ThreadSignal(TRUE, phr),
	m_bStreaming(FALSE),
	m_bEOS(FALSE),
	m_bEOSDelivered(FALSE),
	m_pMediaSample(NULL),
	m_dwAdvise(0),
	m_pQSink(NULL),
	m_pInputPin(NULL),
	m_bRepaintStatus(TRUE),
	m_SignalTime(0),
	m_bInReceive(FALSE),
	m_EndOfStreamTimer(0)
{
	if (SUCCEEDED(*phr)) {
		Ready();
#ifdef PERF
		m_idBaseStamp = MSR_REGISTER(TEXT("BaseRenderer: sample time stamp"));
		m_idBaseRenderTime = MSR_REGISTER(TEXT("BaseRenderer: draw time (msec)"));
		m_idBaseAccuracy = MSR_REGISTER(TEXT("BaseRenderer: Accuracy (msec)"));
#endif
	}
}

STDMETHODIMP CBaseRenderer::FindPin(LPCWSTR Id, __deref_out IPin **ppPin)
{
	CheckPointer(ppPin, E_POINTER);

	if (0 == lstrcmpW(Id, L"In")) {
		*ppPin = GetPin(0);
		if (*ppPin) {
			(*ppPin)->AddRef();
		}
		else {
			return E_OUTOFMEMORY;
		}
	}
	else {
		*ppPin = NULL;
		return VFW_E_NOT_FOUND;
	}
	return NOERROR;
}

CBaseVideoRenderer::CBaseVideoRenderer(
	REFCLSID RenderClass, // CLSID for this renderer
	__in_opt LPCTSTR pName,         // Debug ONLY description
	__inout_opt LPUNKNOWN pUnk,       // Aggregated owner object
	__inout HRESULT *phr) :       // General OLE return code

	CBaseRenderer(RenderClass, pName, pUnk, phr),
	m_cFramesDropped(0),
	m_cFramesDrawn(0),
	m_bSupplierHandlingQuality(FALSE)
{
	ResetStreamingTimes();

#ifdef PERF
	m_idTimeStamp = MSR_REGISTER(TEXT("Frame time stamp"));
	m_idEarliness = MSR_REGISTER(TEXT("Earliness fudge"));
	m_idTarget = MSR_REGISTER(TEXT("Target (mSec)"));
	m_idSchLateTime = MSR_REGISTER(TEXT("mSec late when scheduled"));
	m_idDecision = MSR_REGISTER(TEXT("Scheduler decision code"));
	m_idQualityRate = MSR_REGISTER(TEXT("Quality rate sent"));
	m_idQualityTime = MSR_REGISTER(TEXT("Quality time sent"));
	m_idWaitReal = MSR_REGISTER(TEXT("Render wait"));
	// m_idWait            = MSR_REGISTER(TEXT("wait time recorded (msec)"));
	m_idFrameAccuracy = MSR_REGISTER(TEXT("Frame accuracy (msecs)"));
	m_bDrawLateFrames = GetProfileInt(AMQUALITY, DRAWLATEFRAMES, FALSE);
	//m_idSendQuality      = MSR_REGISTER(TEXT("Processing Quality message"));

	m_idRenderAvg = MSR_REGISTER(TEXT("Render draw time Avg"));
	m_idFrameAvg = MSR_REGISTER(TEXT("FrameAvg"));
	m_idWaitAvg = MSR_REGISTER(TEXT("WaitAvg"));
	m_idDuration = MSR_REGISTER(TEXT("Duration"));
	m_idThrottle = MSR_REGISTER(TEXT("Audio-video throttle wait"));
	// m_idDebug           = MSR_REGISTER(TEXT("Debug stuff"));
#endif // PERF
} // Constructor

STDMETHODIMP
CBaseVideoRenderer::JoinFilterGraph(__inout_opt IFilterGraph *pGraph, __in_opt LPCWSTR pName)
{
	// Since we send EC_ACTIVATE, we also need to ensure
	// we send EC_WINDOW_DESTROYED or the resource manager may be
	// holding us as a focus object
	if (!pGraph && m_pGraph) {

		// We were in a graph and now we're not
		// Do this properly in case we are aggregated
		IBaseFilter* pFilter = this;
		NotifyEvent(EC_WINDOW_DESTROYED, (LPARAM)pFilter, 0);
	}
	return CBaseFilter::JoinFilterGraph(pGraph, pName);
}

void WINAPI DbgAssert(LPCTSTR pCondition, LPCTSTR pFileName, INT iLine) {
	
}

int (WINAPIV * __vsnprintf)(wchar_t *, size_t, const wchar_t*, va_list) = _vsnwprintf; // fix for unresolved symbol chaos, see http://stackoverflow.com/questions/31053670/unresolved-external-symbol-vsnprintf-in-dxerr-lib
