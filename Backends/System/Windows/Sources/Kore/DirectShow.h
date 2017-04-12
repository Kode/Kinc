#include <Windows.h>

#include <dshow.h>

#define DECLARE_IUNKNOWN                                        \
    STDMETHODIMP QueryInterface(REFIID riid, __deref_out void **ppv) {      \
        return GetOwner()->QueryInterface(riid,ppv);            \
    };                                                          \
    STDMETHODIMP_(ULONG) AddRef() {                             \
        return GetOwner()->AddRef();                            \
    };                                                          \
    STDMETHODIMP_(ULONG) Release() {                            \
        return GetOwner()->Release();                           \
    };

#ifndef INONDELEGATINGUNKNOWN_DEFINED
DECLARE_INTERFACE(INonDelegatingUnknown)
{
	STDMETHOD(NonDelegatingQueryInterface) (THIS_ REFIID, LPVOID *) PURE;
	STDMETHOD_(ULONG, NonDelegatingAddRef)(THIS) PURE;
	STDMETHOD_(ULONG, NonDelegatingRelease)(THIS) PURE;
};
#define INONDELEGATINGUNKNOWN_DEFINED
#endif

class CBaseObject
{

private:

	// Disable the copy constructor and assignment by default so you will get
	//   compiler errors instead of unexpected behaviour if you pass objects
	//   by value or assign objects.
	CBaseObject(const CBaseObject& objectSrc);          // no implementation
	void operator=(const CBaseObject& objectSrc);       // no implementation

private:
	static LONG m_cObjects;     /* Total number of objects active */

protected:
#ifdef DEBUG
	DWORD m_dwCookie;           /* Cookie identifying this object */
#endif


public:

	/* These increment and decrement the number of active objects */

	CBaseObject(__in_opt LPCTSTR pName);
#ifdef UNICODE
	CBaseObject(__in_opt LPCSTR pName);
#endif
	~CBaseObject();

	/* Call this to find if there are any CUnknown derived objects active */

	static LONG ObjectsActive() {
		return m_cObjects;
	};
};

class AM_NOVTABLE CUnknown : public INonDelegatingUnknown,
	public CBaseObject
{
private:
	const LPUNKNOWN m_pUnknown; /* Owner of this object */

protected:                      /* So we can override NonDelegatingRelease() */
	volatile LONG m_cRef;       /* Number of reference counts */

public:

	CUnknown(__in_opt LPCTSTR pName, __in_opt LPUNKNOWN pUnk);
	virtual ~CUnknown() {};

	// This is redundant, just use the other constructor
	//   as we never touch the HRESULT in this anyway
	CUnknown(__in_opt LPCTSTR Name, __in_opt LPUNKNOWN pUnk, __inout_opt HRESULT *phr);
#ifdef UNICODE
	CUnknown(__in_opt LPCSTR pName, __in_opt LPUNKNOWN pUnk);
	CUnknown(__in_opt LPCSTR pName, __in_opt LPUNKNOWN pUnk, __inout_opt HRESULT *phr);
#endif

	/* Return the owner of this object */

	LPUNKNOWN GetOwner() const {
		return m_pUnknown;
	};

	/* Called from the class factory to create a new instance, it is
	pure virtual so it must be overriden in your derived class */

	/* static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *) */

	/* Non delegating unknown implementation */

	STDMETHODIMP NonDelegatingQueryInterface(REFIID, __deref_out void **);
	STDMETHODIMP_(ULONG) NonDelegatingAddRef();
	STDMETHODIMP_(ULONG) NonDelegatingRelease();
};

const LONGLONG MILLISECONDS = (1000);            // 10 ^ 3
const LONGLONG NANOSECONDS = (1000000000);       // 10 ^ 9
const LONGLONG UNITS = (NANOSECONDS / 100);      // 10 ^ 7

												 /*  Unfortunately an inline function here generates a call to __allmul
												 - even for constants!
												 */
#define MILLISECONDS_TO_100NS_UNITS(lMs) \
    Int32x32To64((lMs), (UNITS / MILLISECONDS))

class CRefTime
{
public:

	// *MUST* be the only data member so that this class is exactly
	// equivalent to a REFERENCE_TIME.
	// Also, must be *no virtual functions*

	REFERENCE_TIME m_time;

	inline CRefTime()
	{
		// default to 0 time
		m_time = 0;
	};

	inline CRefTime(LONG msecs)
	{
		m_time = MILLISECONDS_TO_100NS_UNITS(msecs);
	};

	inline CRefTime(REFERENCE_TIME rt)
	{
		m_time = rt;
	};

	inline operator REFERENCE_TIME() const
	{
		return m_time;
	};

	inline CRefTime& operator=(const CRefTime& rt)
	{
		m_time = rt.m_time;
		return *this;
	};

	inline CRefTime& operator=(const LONGLONG ll)
	{
		m_time = ll;
		return *this;
	};

	inline CRefTime& operator+=(const CRefTime& rt)
	{
		return (*this = *this + rt);
	};

	inline CRefTime& operator-=(const CRefTime& rt)
	{
		return (*this = *this - rt);
	};

	inline LONG Millisecs(void)
	{
		return (LONG)(m_time / (UNITS / MILLISECONDS));
	};

	inline LONGLONG GetUnits(void)
	{
		return m_time;
	};
};

class CCritSec {

	// make copy constructor and assignment operator inaccessible

	CCritSec(const CCritSec &refCritSec);
	CCritSec &operator=(const CCritSec &refCritSec);

	CRITICAL_SECTION m_CritSec;

#ifdef DEBUG
public:
	DWORD   m_currentOwner;
	DWORD   m_lockCount;
	BOOL    m_fTrace;        // Trace this one
public:
	CCritSec();
	~CCritSec();
	void Lock();
	void Unlock();
#else

public:
	CCritSec() {
		InitializeCriticalSection(&m_CritSec);
	};

	~CCritSec() {
		DeleteCriticalSection(&m_CritSec);
	};

	void Lock() {
		EnterCriticalSection(&m_CritSec);
	};

	void Unlock() {
		LeaveCriticalSection(&m_CritSec);
	};
#endif
};

// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class CAutoLock {

	// make copy constructor and assignment operator inaccessible

	CAutoLock(const CAutoLock &refAutoLock);
	CAutoLock &operator=(const CAutoLock &refAutoLock);

protected:
	CCritSec * m_pLock;

public:
	CAutoLock(CCritSec * plock)
	{
		m_pLock = plock;
		m_pLock->Lock();
	};

	~CAutoLock() {
		m_pLock->Unlock();
	};
};

typedef REGPINTYPES
AMOVIESETUP_MEDIATYPE, *PAMOVIESETUP_MEDIATYPE, *FAR LPAMOVIESETUP_MEDIATYPE;

typedef REGFILTERPINS
AMOVIESETUP_PIN, *PAMOVIESETUP_PIN, *FAR LPAMOVIESETUP_PIN;

typedef struct _AMOVIESETUP_FILTER
{
	const CLSID * clsID;
	const WCHAR * strName;
	DWORD      dwMerit;
	UINT       nPins;
	const AMOVIESETUP_PIN * lpPin;
}
AMOVIESETUP_FILTER, *PAMOVIESETUP_FILTER, *FAR LPAMOVIESETUP_FILTER;

class AM_NOVTABLE CBaseFilter : public CUnknown,        // Handles an IUnknown
	public IBaseFilter,     // The Filter Interface
	public IAMovieSetup     // For un/registration
{

	friend class CBasePin;

protected:
	FILTER_STATE    m_State;            // current state: running, paused
	IReferenceClock *m_pClock;          // this graph's ref clock
	CRefTime        m_tStart;           // offset from stream time to reference time
	CLSID	    m_clsid;            // This filters clsid
									// used for serialization
	CCritSec        *m_pLock;           // Object we use for locking

	WCHAR           *m_pName;           // Full filter name
	IFilterGraph    *m_pGraph;          // Graph we belong to
	IMediaEventSink *m_pSink;           // Called with notify events
	LONG            m_PinVersion;       // Current pin version

public:

	CBaseFilter(
		__in_opt LPCTSTR pName,   // Object description
		__inout_opt LPUNKNOWN pUnk,  // IUnknown of delegating object
		__in CCritSec  *pLock,    // Object who maintains lock
		REFCLSID   clsid);        // The clsid to be used to serialize this filter

	CBaseFilter(
		__in_opt LPCTSTR pName,    // Object description
		__in_opt LPUNKNOWN pUnk,  // IUnknown of delegating object
		__in CCritSec  *pLock,    // Object who maintains lock
		REFCLSID   clsid,         // The clsid to be used to serialize this filter
		__inout HRESULT   *phr);  // General OLE return code
#ifdef UNICODE
	CBaseFilter(
		__in_opt LPCSTR pName,    // Object description
		__in_opt LPUNKNOWN pUnk,  // IUnknown of delegating object
		__in CCritSec  *pLock,    // Object who maintains lock
		REFCLSID   clsid);        // The clsid to be used to serialize this filter

	CBaseFilter(
		__in_opt LPCSTR pName,     // Object description
		__in_opt LPUNKNOWN pUnk,  // IUnknown of delegating object
		__in CCritSec  *pLock,    // Object who maintains lock
		REFCLSID   clsid,         // The clsid to be used to serialize this filter
		__inout HRESULT   *phr);  // General OLE return code
#endif
	~CBaseFilter();

	DECLARE_IUNKNOWN

	// override this to say what interfaces we support where
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void ** ppv);
#ifdef DEBUG
	STDMETHODIMP_(ULONG) NonDelegatingRelease();
#endif

	//
	// --- IPersist method ---
	//

	STDMETHODIMP GetClassID(__out CLSID *pClsID);

	// --- IMediaFilter methods ---

	STDMETHODIMP GetState(DWORD dwMSecs, __out FILTER_STATE *State);

	STDMETHODIMP SetSyncSource(__in_opt IReferenceClock *pClock);

	STDMETHODIMP GetSyncSource(__deref_out_opt IReferenceClock **pClock);


	// override Stop and Pause so we can activate the pins.
	// Note that Run will call Pause first if activation needed.
	// Override these if you want to activate your filter rather than
	// your pins.
	STDMETHODIMP Stop();
	STDMETHODIMP Pause();

	// the start parameter is the difference to be added to the
	// sample's stream time to get the reference time for
	// its presentation
	STDMETHODIMP Run(REFERENCE_TIME tStart);

	// --- helper methods ---

	// return the current stream time - ie find out what
	// stream time should be appearing now
	virtual HRESULT StreamTime(CRefTime& rtStream);

	// Is the filter currently active?
	BOOL IsActive() {
		CAutoLock cObjectLock(m_pLock);
		return ((m_State == State_Paused) || (m_State == State_Running));
	};

	// Is this filter stopped (without locking)
	BOOL IsStopped() {
		return (m_State == State_Stopped);
	};

	//
	// --- IBaseFilter methods ---
	//

	// pin enumerator
	STDMETHODIMP EnumPins(
		__deref_out IEnumPins ** ppEnum);


	// default behaviour of FindPin assumes pin ids are their names
	STDMETHODIMP FindPin(
		LPCWSTR Id,
		__deref_out IPin ** ppPin
	);

	STDMETHODIMP QueryFilterInfo(
		__out FILTER_INFO * pInfo);

	STDMETHODIMP JoinFilterGraph(
		__inout_opt IFilterGraph * pGraph,
		__in_opt LPCWSTR pName);

	// return a Vendor information string. Optional - may return E_NOTIMPL.
	// memory returned should be freed using CoTaskMemFree
	// default implementation returns E_NOTIMPL
	STDMETHODIMP QueryVendorInfo(
		__deref_out LPWSTR* pVendorInfo
	);

	// --- helper methods ---

	// send an event notification to the filter graph if we know about it.
	// returns S_OK if delivered, S_FALSE if the filter graph does not sink
	// events, or an error otherwise.
	HRESULT NotifyEvent(
		long EventCode,
		LONG_PTR EventParam1,
		LONG_PTR EventParam2);

	// return the filter graph we belong to
	__out_opt IFilterGraph *GetFilterGraph() {
		return m_pGraph;
	}

	// Request reconnect
	// pPin is the pin to reconnect
	// pmt is the type to reconnect with - can be NULL
	// Calls ReconnectEx on the filter graph
	HRESULT ReconnectPin(IPin *pPin, __in_opt AM_MEDIA_TYPE const *pmt);

	// find out the current pin version (used by enumerators)
	virtual LONG GetPinVersion();
	void IncrementPinVersion();

	// you need to supply these to access the pins from the enumerator
	// and for default Stop and Pause/Run activation.
	virtual int GetPinCount() PURE;
	virtual CBasePin *GetPin(int n) PURE;

	// --- IAMovieSetup methods ---

	STDMETHODIMP Register();    // ask filter to register itself
	STDMETHODIMP Unregister();  // and unregister itself

								// --- setup helper methods ---
								// (override to return filters setup data)

	virtual __out_opt LPAMOVIESETUP_FILTER GetSetupData() { return NULL; }

};

class CBaseDispatch
{
	ITypeInfo * m_pti;

public:

	CBaseDispatch() : m_pti(NULL) {}
	~CBaseDispatch();

	/* IDispatch methods */
	STDMETHODIMP GetTypeInfoCount(__out UINT * pctinfo);

	STDMETHODIMP GetTypeInfo(
		REFIID riid,
		UINT itinfo,
		LCID lcid,
		__deref_out ITypeInfo ** pptinfo);

	STDMETHODIMP GetIDsOfNames(
		REFIID riid,
		__in_ecount(cNames) LPOLESTR * rgszNames,
		UINT cNames,
		LCID lcid,
		__out_ecount(cNames) DISPID * rgdispid);
};

class AM_NOVTABLE CMediaPosition :
	public IMediaPosition,
	public CUnknown
{
	CBaseDispatch m_basedisp;


public:

	CMediaPosition(__in_opt LPCTSTR, __in_opt LPUNKNOWN);
	CMediaPosition(__in_opt LPCTSTR, __in_opt LPUNKNOWN, __inout HRESULT *phr);

	DECLARE_IUNKNOWN

	// override this to publicise our interfaces
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv);

	/* IDispatch methods */
	STDMETHODIMP GetTypeInfoCount(__out UINT * pctinfo);

	STDMETHODIMP GetTypeInfo(
		UINT itinfo,
		LCID lcid,
		__deref_out ITypeInfo ** pptinfo);

	STDMETHODIMP GetIDsOfNames(
		REFIID riid,
		__in_ecount(cNames) LPOLESTR * rgszNames,
		UINT cNames,
		LCID lcid,
		__out_ecount(cNames) DISPID * rgdispid);

	STDMETHODIMP Invoke(
		DISPID dispidMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		__in DISPPARAMS * pdispparams,
		__out_opt VARIANT * pvarResult,
		__out_opt EXCEPINFO * pexcepinfo,
		__out_opt UINT * puArgErr);

};

class CPosPassThru : public IMediaSeeking, public CMediaPosition
{
	IPin *m_pPin;

	HRESULT GetPeer(__deref_out IMediaPosition **ppMP);
	HRESULT GetPeerSeeking(__deref_out IMediaSeeking **ppMS);

public:

	CPosPassThru(__in_opt LPCTSTR, __in_opt LPUNKNOWN, __inout HRESULT*, IPin *);
	DECLARE_IUNKNOWN

	HRESULT ForceRefresh() {
		return S_OK;
	};

	// override to return an accurate current position
	virtual HRESULT GetMediaTime(__out LONGLONG *pStartTime, __out_opt LONGLONG *pEndTime) {
		return E_FAIL;
	}

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void **ppv);

	// IMediaSeeking methods
	STDMETHODIMP GetCapabilities(__out DWORD * pCapabilities);
	STDMETHODIMP CheckCapabilities(__inout DWORD * pCapabilities);
	STDMETHODIMP SetTimeFormat(const GUID * pFormat);
	STDMETHODIMP GetTimeFormat(__out GUID *pFormat);
	STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
	STDMETHODIMP IsFormatSupported(const GUID * pFormat);
	STDMETHODIMP QueryPreferredFormat(__out GUID *pFormat);
	STDMETHODIMP ConvertTimeFormat(__out LONGLONG * pTarget,
		__in_opt const GUID * pTargetFormat,
		LONGLONG Source,
		__in_opt const GUID * pSourceFormat);
	STDMETHODIMP SetPositions(__inout_opt LONGLONG * pCurrent, DWORD CurrentFlags
		, __inout_opt LONGLONG * pStop, DWORD StopFlags);

	STDMETHODIMP GetPositions(__out_opt LONGLONG * pCurrent, __out_opt LONGLONG * pStop);
	STDMETHODIMP GetCurrentPosition(__out LONGLONG * pCurrent);
	STDMETHODIMP GetStopPosition(__out LONGLONG * pStop);
	STDMETHODIMP SetRate(double dRate);
	STDMETHODIMP GetRate(__out double * pdRate);
	STDMETHODIMP GetDuration(__out LONGLONG *pDuration);
	STDMETHODIMP GetAvailable(__out_opt LONGLONG *pEarliest, __out_opt LONGLONG *pLatest);
	STDMETHODIMP GetPreroll(__out LONGLONG *pllPreroll);

	// IMediaPosition properties
	STDMETHODIMP get_Duration(__out REFTIME * plength);
	STDMETHODIMP put_CurrentPosition(REFTIME llTime);
	STDMETHODIMP get_StopTime(__out REFTIME * pllTime);
	STDMETHODIMP put_StopTime(REFTIME llTime);
	STDMETHODIMP get_PrerollTime(__out REFTIME * pllTime);
	STDMETHODIMP put_PrerollTime(REFTIME llTime);
	STDMETHODIMP get_Rate(__out double * pdRate);
	STDMETHODIMP put_Rate(double dRate);
	STDMETHODIMP get_CurrentPosition(__out REFTIME * pllTime);
	STDMETHODIMP CanSeekForward(__out LONG *pCanSeekForward);
	STDMETHODIMP CanSeekBackward(__out LONG *pCanSeekBackward);

private:
	HRESULT GetSeekingLongLong(HRESULT(__stdcall IMediaSeeking::*pMethod)(LONGLONG *),
		__out LONGLONG * pll);
};

class CRendererPosPassThru : public CPosPassThru
{
	CCritSec m_PositionLock;    // Locks access to our position
	LONGLONG m_StartMedia;      // Start media time last seen
	LONGLONG m_EndMedia;        // And likewise the end media
	BOOL m_bReset;              // Have media times been set

public:

	// Used to help with passing media times through graph

	CRendererPosPassThru(__in_opt LPCTSTR, __in_opt LPUNKNOWN, __inout HRESULT*, IPin *);
	HRESULT RegisterMediaTime(IMediaSample *pMediaSample);
	HRESULT RegisterMediaTime(LONGLONG StartTime, LONGLONG EndTime);
	HRESULT GetMediaTime(__out LONGLONG *pStartTime, __out_opt LONGLONG *pEndTime);
	HRESULT ResetMediaTime();
	HRESULT EOS();
};

void WINAPI DbgAssert(const wchar_t* pCondition, const wchar_t* pFileName, INT iLine);

#ifndef ASSERT
#define ASSERT(_x_) if (!(_x_))         \
            DbgAssert(TEXT(#_x_),TEXT(__FILE__),__LINE__)
#endif

#define EXECUTE_ASSERT(_x_) ASSERT(_x_)

class CAMEvent
{

	// make copy constructor and assignment operator inaccessible

	CAMEvent(const CAMEvent &refEvent);
	CAMEvent &operator=(const CAMEvent &refEvent);

protected:
	HANDLE m_hEvent;
public:
	CAMEvent(BOOL fManualReset = FALSE, __inout_opt HRESULT *phr = NULL);
	CAMEvent(__inout_opt HRESULT *phr);
	~CAMEvent();

	// Cast to HANDLE - we don't support this as an lvalue
	operator HANDLE () const { return m_hEvent; };

	void Set() { EXECUTE_ASSERT(SetEvent(m_hEvent)); };
	BOOL Wait(DWORD dwTimeout = INFINITE) {
		return (WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0);
	};
	void Reset() { ResetEvent(m_hEvent); };
	BOOL Check() { return Wait(0); };
};

class CMediaType : public _AMMediaType {

public:

	~CMediaType();
	CMediaType();
	CMediaType(const GUID * majortype);
	CMediaType(const AM_MEDIA_TYPE&, __out_opt HRESULT* phr = NULL);
	CMediaType(const CMediaType&, __out_opt HRESULT* phr = NULL);

	CMediaType& operator=(const CMediaType&);
	CMediaType& operator=(const AM_MEDIA_TYPE&);

	BOOL operator == (const CMediaType&) const;
	BOOL operator != (const CMediaType&) const;

	HRESULT Set(const CMediaType& rt);
	HRESULT Set(const AM_MEDIA_TYPE& rt);

	BOOL IsValid() const;

	const GUID *Type() const { return &majortype; };
	void SetType(const GUID *);
	const GUID *Subtype() const { return &subtype; };
	void SetSubtype(const GUID *);

	BOOL IsFixedSize() const { return bFixedSizeSamples; };
	BOOL IsTemporalCompressed() const { return bTemporalCompression; };
	ULONG GetSampleSize() const;

	void SetSampleSize(ULONG sz);
	void SetVariableSize();
	void SetTemporalCompression(BOOL bCompressed);

	// read/write pointer to format - can't change length without
	// calling SetFormat, AllocFormatBuffer or ReallocFormatBuffer

	BYTE*   Format() const { return pbFormat; };
	ULONG   FormatLength() const { return cbFormat; };

	void SetFormatType(const GUID *);
	const GUID *FormatType() const { return &formattype; };
	BOOL SetFormat(__in_bcount(length) BYTE *pFormat, ULONG length);
	void ResetFormatBuffer();
	BYTE* AllocFormatBuffer(ULONG length);
	BYTE* ReallocFormatBuffer(ULONG length);

	void InitMediaType();

	BOOL MatchesPartial(const CMediaType* ppartial) const;
	BOOL IsPartiallySpecified(void) const;
};

class  AM_NOVTABLE CBasePin : public CUnknown, public IPin, public IQualityControl
{

protected:

	WCHAR *         m_pName;		        // This pin's name
	IPin            *m_Connected;               // Pin we have connected to
	PIN_DIRECTION   m_dir;                      // Direction of this pin
	CCritSec        *m_pLock;                   // Object we use for locking
	bool            m_bRunTimeError;            // Run time error generated
	bool            m_bCanReconnectWhenActive;  // OK to reconnect when active
	bool            m_bTryMyTypesFirst;         // When connecting enumerate
												// this pin's types first
	CBaseFilter    *m_pFilter;                  // Filter we were created by
	IQualityControl *m_pQSink;                  // Target for Quality messages
	LONG            m_TypeVersion;              // Holds current type version
	CMediaType      m_mt;                       // Media type of connection

	CRefTime        m_tStart;                   // time from NewSegment call
	CRefTime        m_tStop;                    // time from NewSegment
	double          m_dRate;                    // rate from NewSegment

#ifdef DEBUG
	LONG            m_cRef;                     // Ref count tracing
#endif

												// displays pin connection information

#ifdef DEBUG
	void DisplayPinInfo(IPin *pReceivePin);
	void DisplayTypeInfo(IPin *pPin, const CMediaType *pmt);
#else
	void DisplayPinInfo(IPin *pReceivePin) {};
	void DisplayTypeInfo(IPin *pPin, const CMediaType *pmt) {};
#endif

	// used to agree a media type for a pin connection

	// given a specific media type, attempt a connection (includes
	// checking that the type is acceptable to this pin)
	HRESULT
		AttemptConnection(
			IPin* pReceivePin,      // connect to this pin
			const CMediaType* pmt   // using this type
		);

	// try all the media types in this enumerator - for each that
	// we accept, try to connect using ReceiveConnection.
	HRESULT TryMediaTypes(
		IPin *pReceivePin,          // connect to this pin
		__in_opt const CMediaType *pmt,  // proposed type from Connect
		IEnumMediaTypes *pEnum);    // try this enumerator

									// establish a connection with a suitable mediatype. Needs to
									// propose a media type if the pmt pointer is null or partially
									// specified - use TryMediaTypes on both our and then the other pin's
									// enumerator until we find one that works.
	HRESULT AgreeMediaType(
		IPin *pReceivePin,      // connect to this pin
		const CMediaType *pmt);      // proposed type from Connect

public:

	CBasePin(
		__in_opt LPCTSTR pObjectName,         // Object description
		__in CBaseFilter *pFilter,       // Owning filter who knows about pins
		__in CCritSec *pLock,            // Object who implements the lock
		__inout HRESULT *phr,               // General OLE return code
		__in_opt LPCWSTR pName,              // Pin name for us
		PIN_DIRECTION dir);         // Either PINDIR_INPUT or PINDIR_OUTPUT
#ifdef UNICODE
	CBasePin(
		__in_opt LPCSTR pObjectName,         // Object description
		__in CBaseFilter *pFilter,       // Owning filter who knows about pins
		__in CCritSec *pLock,            // Object who implements the lock
		__inout HRESULT *phr,               // General OLE return code
		__in_opt LPCWSTR pName,              // Pin name for us
		PIN_DIRECTION dir);         // Either PINDIR_INPUT or PINDIR_OUTPUT
#endif
	virtual ~CBasePin();

	DECLARE_IUNKNOWN

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void ** ppv);
	STDMETHODIMP_(ULONG) NonDelegatingRelease();
	STDMETHODIMP_(ULONG) NonDelegatingAddRef();

	// --- IPin methods ---

	// take lead role in establishing a connection. Media type pointer
	// may be null, or may point to partially-specified mediatype
	// (subtype or format type may be GUID_NULL).
	STDMETHODIMP Connect(
		IPin * pReceivePin,
		__in_opt const AM_MEDIA_TYPE *pmt   // optional media type
	);

	// (passive) accept a connection from another pin
	STDMETHODIMP ReceiveConnection(
		IPin * pConnector,      // this is the initiating connecting pin
		const AM_MEDIA_TYPE *pmt   // this is the media type we will exchange
	);

	STDMETHODIMP Disconnect();

	STDMETHODIMP ConnectedTo(__deref_out IPin **pPin);

	STDMETHODIMP ConnectionMediaType(__out AM_MEDIA_TYPE *pmt);

	STDMETHODIMP QueryPinInfo(
		__out PIN_INFO * pInfo
	);

	STDMETHODIMP QueryDirection(
		__out PIN_DIRECTION * pPinDir
	);

	STDMETHODIMP QueryId(
		__deref_out LPWSTR * Id
	);

	// does the pin support this media type
	STDMETHODIMP QueryAccept(
		const AM_MEDIA_TYPE *pmt
	);

	// return an enumerator for this pins preferred media types
	STDMETHODIMP EnumMediaTypes(
		__deref_out IEnumMediaTypes **ppEnum
	);

	// return an array of IPin* - the pins that this pin internally connects to
	// All pins put in the array must be AddReffed (but no others)
	// Errors: "Can't say" - FAIL, not enough slots - return S_FALSE
	// Default: return E_NOTIMPL
	// The filter graph will interpret NOT_IMPL as any input pin connects to
	// all visible output pins and vice versa.
	// apPin can be NULL if nPin==0 (not otherwise).
	STDMETHODIMP QueryInternalConnections(
		__out_ecount_part(*nPin, *nPin) IPin* *apPin,     // array of IPin*
		__inout ULONG *nPin                  // on input, the number of slots
											 // on output  the number of pins
	) {
		return E_NOTIMPL;
	}

	// Called when no more data will be sent
	STDMETHODIMP EndOfStream(void);

	// Begin/EndFlush still PURE

	// NewSegment notifies of the start/stop/rate applying to the data
	// about to be received. Default implementation records data and
	// returns S_OK.
	// Override this to pass downstream.
	STDMETHODIMP NewSegment(
		REFERENCE_TIME tStart,
		REFERENCE_TIME tStop,
		double dRate);

	//================================================================================
	// IQualityControl methods
	//================================================================================

	STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

	STDMETHODIMP SetSink(IQualityControl * piqc);

	// --- helper methods ---

	// Returns true if the pin is connected. false otherwise.
	BOOL IsConnected(void) { return (m_Connected != NULL); };
	// Return the pin this is connected to (if any)
	IPin * GetConnected() { return m_Connected; };

	// Check if our filter is currently stopped
	BOOL IsStopped() {
		return (m_pFilter->m_State == State_Stopped);
	};

	// find out the current type version (used by enumerators)
	virtual LONG GetMediaTypeVersion();
	void IncrementTypeVersion();

	// switch the pin to active (paused or running) mode
	// not an error to call this if already active
	virtual HRESULT Active(void);

	// switch the pin to inactive state - may already be inactive
	virtual HRESULT Inactive(void);

	// Notify of Run() from filter
	virtual HRESULT Run(REFERENCE_TIME tStart);

	// check if the pin can support this specific proposed type and format
	virtual HRESULT CheckMediaType(const CMediaType *) PURE;

	// set the connection to use this format (previously agreed)
	virtual HRESULT SetMediaType(const CMediaType *);

	// check that the connection is ok before verifying it
	// can be overridden eg to check what interfaces will be supported.
	virtual HRESULT CheckConnect(IPin *);

	// Set and release resources required for a connection
	virtual HRESULT BreakConnect();
	virtual HRESULT CompleteConnect(IPin *pReceivePin);

	// returns the preferred formats for a pin
	virtual HRESULT GetMediaType(int iPosition, __inout CMediaType *pMediaType);

	// access to NewSegment values
	REFERENCE_TIME CurrentStopTime() {
		return m_tStop;
	}
	REFERENCE_TIME CurrentStartTime() {
		return m_tStart;
	}
	double CurrentRate() {
		return m_dRate;
	}

	//  Access name
	LPWSTR Name() { return m_pName; };

	//  Can reconnectwhen active?
	void SetReconnectWhenActive(bool bCanReconnect)
	{
		m_bCanReconnectWhenActive = bCanReconnect;
	}

	bool CanReconnectWhenActive()
	{
		return m_bCanReconnectWhenActive;
	}

protected:
	STDMETHODIMP DisconnectInternal();
};

class CBaseRenderer : public CBaseFilter
{
protected:

	friend class CRendererInputPin;

	friend void CALLBACK EndOfStreamTimer(UINT uID,      // Timer identifier
		UINT uMsg,     // Not currently used
		DWORD_PTR dwUser,  // User information
		DWORD_PTR dw1,     // Windows reserved
		DWORD_PTR dw2);    // Is also reserved

	CRendererPosPassThru *m_pPosition;  // Media seeking pass by object
	CAMEvent m_RenderEvent;             // Used to signal timer events
	CAMEvent m_ThreadSignal;            // Signalled to release worker thread
	CAMEvent m_evComplete;              // Signalled when state complete
	BOOL m_bAbort;                      // Stop us from rendering more data
	BOOL m_bStreaming;                  // Are we currently streaming
	DWORD_PTR m_dwAdvise;                   // Timer advise cookie
	IMediaSample *m_pMediaSample;       // Current image media sample
	BOOL m_bEOS;                        // Any more samples in the stream
	BOOL m_bEOSDelivered;               // Have we delivered an EC_COMPLETE
	CRendererInputPin *m_pInputPin;     // Our renderer input pin object
	CCritSec m_InterfaceLock;           // Critical section for interfaces
	CCritSec m_RendererLock;            // Controls access to internals
	IQualityControl * m_pQSink;         // QualityControl sink
	BOOL m_bRepaintStatus;              // Can we signal an EC_REPAINT
										//  Avoid some deadlocks by tracking filter during stop
	volatile BOOL  m_bInReceive;        // Inside Receive between PrepareReceive
										// And actually processing the sample
	REFERENCE_TIME m_SignalTime;        // Time when we signal EC_COMPLETE
	UINT m_EndOfStreamTimer;            // Used to signal end of stream
	CCritSec m_ObjectCreationLock;      // This lock protects the creation and
										// of m_pPosition and m_pInputPin.  It
										// ensures that two threads cannot create
										// either object simultaneously.

public:

	CBaseRenderer(REFCLSID RenderClass, // CLSID for this renderer
		__in_opt LPCTSTR pName,         // Debug ONLY description
		__inout_opt LPUNKNOWN pUnk,       // Aggregated owner object
		__inout HRESULT *phr);        // General OLE return code

	~CBaseRenderer();

	// Overriden to say what interfaces we support and where

	virtual HRESULT GetMediaPositionInterface(REFIID riid, __deref_out void **ppv);
	STDMETHODIMP NonDelegatingQueryInterface(REFIID, __deref_out void **);

	virtual HRESULT SourceThreadCanWait(BOOL bCanWait);

#ifdef DEBUG
	// Debug only dump of the renderer state
	void DisplayRendererState();
#endif
	virtual HRESULT WaitForRenderTime();
	virtual HRESULT CompleteStateChange(FILTER_STATE OldState);

	// Return internal information about this filter

	BOOL IsEndOfStream() { return m_bEOS; };
	BOOL IsEndOfStreamDelivered() { return m_bEOSDelivered; };
	BOOL IsStreaming() { return m_bStreaming; };
	void SetAbortSignal(BOOL bAbort) { m_bAbort = bAbort; };
	virtual void OnReceiveFirstSample(IMediaSample *pMediaSample) { };
	CAMEvent *GetRenderEvent() { return &m_RenderEvent; };

	// Permit access to the transition state

	void Ready() { m_evComplete.Set(); };
	void NotReady() { m_evComplete.Reset(); };
	BOOL CheckReady() { return m_evComplete.Check(); };

	virtual int GetPinCount();
	virtual CBasePin *GetPin(int n);
	FILTER_STATE GetRealState();
	void SendRepaint();
	void SendNotifyWindow(IPin *pPin, HWND hwnd);
	BOOL OnDisplayChange();
	void SetRepaintStatus(BOOL bRepaint);

	// Override the filter and pin interface functions

	STDMETHODIMP Stop();
	STDMETHODIMP Pause();
	STDMETHODIMP Run(REFERENCE_TIME StartTime);
	STDMETHODIMP GetState(DWORD dwMSecs, __out FILTER_STATE *State);
	STDMETHODIMP FindPin(LPCWSTR Id, __deref_out IPin **ppPin);

	// These are available for a quality management implementation

	virtual void OnRenderStart(IMediaSample *pMediaSample);
	virtual void OnRenderEnd(IMediaSample *pMediaSample);
	virtual HRESULT OnStartStreaming() { return NOERROR; };
	virtual HRESULT OnStopStreaming() { return NOERROR; };
	virtual void OnWaitStart() { };
	virtual void OnWaitEnd() { };
	virtual void PrepareRender() { };

#ifdef PERF
	REFERENCE_TIME m_trRenderStart; // Just before we started drawing
									// Set in OnRenderStart, Used in OnRenderEnd
	int m_idBaseStamp;              // MSR_id for frame time stamp
	int m_idBaseRenderTime;         // MSR_id for true wait time
	int m_idBaseAccuracy;           // MSR_id for time frame is late (int)
#endif

									// Quality management implementation for scheduling rendering

	virtual BOOL ScheduleSample(IMediaSample *pMediaSample);
	virtual HRESULT GetSampleTimes(IMediaSample *pMediaSample,
		__out REFERENCE_TIME *pStartTime,
		__out REFERENCE_TIME *pEndTime);

	virtual HRESULT ShouldDrawSampleNow(IMediaSample *pMediaSample,
		__out REFERENCE_TIME *ptrStart,
		__out REFERENCE_TIME *ptrEnd);

	// Lots of end of stream complexities

	void TimerCallback();
	void ResetEndOfStreamTimer();
	HRESULT NotifyEndOfStream();
	virtual HRESULT SendEndOfStream();
	virtual HRESULT ResetEndOfStream();
	virtual HRESULT EndOfStream();

	// Rendering is based around the clock

	void SignalTimerFired();
	virtual HRESULT CancelNotification();
	virtual HRESULT ClearPendingSample();

	// Called when the filter changes state

	virtual HRESULT Active();
	virtual HRESULT Inactive();
	virtual HRESULT StartStreaming();
	virtual HRESULT StopStreaming();
	virtual HRESULT BeginFlush();
	virtual HRESULT EndFlush();

	// Deal with connections and type changes

	virtual HRESULT BreakConnect();
	virtual HRESULT SetMediaType(const CMediaType *pmt);
	virtual HRESULT CompleteConnect(IPin *pReceivePin);

	// These look after the handling of data samples

	virtual HRESULT PrepareReceive(IMediaSample *pMediaSample);
	virtual HRESULT Receive(IMediaSample *pMediaSample);
	virtual BOOL HaveCurrentSample();
	virtual IMediaSample *GetCurrentSample();
	virtual HRESULT Render(IMediaSample *pMediaSample);

	// Derived classes MUST override these
	virtual HRESULT DoRenderSample(IMediaSample *pMediaSample) PURE;
	virtual HRESULT CheckMediaType(const CMediaType *) PURE;

	// Helper
	void WaitForReceiveToComplete();
};

class CBaseVideoRenderer : public CBaseRenderer,    // Base renderer class
	public IQualProp,        // Property page guff
	public IQualityControl   // Allow throttling
{
protected:

	// Hungarian:
	//     tFoo is the time Foo in mSec (beware m_tStart from filter.h)
	//     trBar is the time Bar by the reference clock

	//******************************************************************
	// State variables to control synchronisation
	//******************************************************************

	// Control of sending Quality messages.  We need to know whether
	// we are in trouble (e.g. frames being dropped) and where the time
	// is being spent.

	// When we drop a frame we play the next one early.
	// The frame after that is likely to wait before drawing and counting this
	// wait as spare time is unfair, so we count it as a zero wait.
	// We therefore need to know whether we are playing frames early or not.

	int m_nNormal;                  // The number of consecutive frames
									// drawn at their normal time (not early)
									// -1 means we just dropped a frame.

#ifdef PERF
	BOOL m_bDrawLateFrames;         // Don't drop any frames (debug and I'm
									// not keen on people using it!)
#endif

	BOOL m_bSupplierHandlingQuality;// The response to Quality messages says
									// our supplier is handling things.
									// We will allow things to go extra late
									// before dropping frames.  We will play
									// very early after he has dropped one.

									// Control of scheduling, frame dropping etc.
									// We need to know where the time is being spent so as to tell whether
									// we should be taking action here, signalling supplier or what.
									// The variables are initialised to a mode of NOT dropping frames.
									// They will tell the truth after a few frames.
									// We typically record a start time for an event, later we get the time
									// again and subtract to get the elapsed time, and we average this over
									// a few frames.  The average is used to tell what mode we are in.

									// Although these are reference times (64 bit) they are all DIFFERENCES
									// between times which are small.  An int will go up to 214 secs before
									// overflow.  Avoiding 64 bit multiplications and divisions seems
									// worth while.



									// Audio-video throttling.  If the user has turned up audio quality
									// very high (in principle it could be any other stream, not just audio)
									// then we can receive cries for help via the graph manager.  In this case
									// we put in a wait for some time after rendering each frame.
	int m_trThrottle;

	// The time taken to render (i.e. BitBlt) frames controls which component
	// needs to degrade.  If the blt is expensive, the renderer degrades.
	// If the blt is cheap it's done anyway and the supplier degrades.
	int m_trRenderAvg;              // Time frames are taking to blt
	int m_trRenderLast;             // Time for last frame blt
	int m_tRenderStart;             // Just before we started drawing (mSec)
									// derived from timeGetTime.

									// When frames are dropped we will play the next frame as early as we can.
									// If it was a false alarm and the machine is fast we slide gently back to
									// normal timing.  To do this, we record the offset showing just how early
									// we really are.  This will normally be negative meaning early or zero.
	int m_trEarliness;

	// Target provides slow long-term feedback to try to reduce the
	// average sync offset to zero.  Whenever a frame is actually rendered
	// early we add a msec or two, whenever late we take off a few.
	// We add or take off 1/32 of the error time.
	// Eventually we should be hovering around zero.  For a really bad case
	// where we were (say) 300mSec off, it might take 100 odd frames to
	// settle down.  The rate of change of this is intended to be slower
	// than any other mechanism in Quartz, thereby avoiding hunting.
	int m_trTarget;

	// The proportion of time spent waiting for the right moment to blt
	// controls whether we bother to drop a frame or whether we reckon that
	// we're doing well enough that we can stand a one-frame glitch.
	int m_trWaitAvg;                // Average of last few wait times
									// (actually we just average how early
									// we were).  Negative here means LATE.

									// The average inter-frame time.
									// This is used to calculate the proportion of the time used by the
									// three operations (supplying us, waiting, rendering)
	int m_trFrameAvg;               // Average inter-frame time
	int m_trDuration;               // duration of last frame.

#ifdef PERF
									// Performance logging identifiers
	int m_idTimeStamp;              // MSR_id for frame time stamp
	int m_idEarliness;              // MSR_id for earliness fudge
	int m_idTarget;                 // MSR_id for Target fudge
	int m_idWaitReal;               // MSR_id for true wait time
	int m_idWait;                   // MSR_id for wait time recorded
	int m_idFrameAccuracy;          // MSR_id for time frame is late (int)
	int m_idRenderAvg;              // MSR_id for Render time recorded (int)
	int m_idSchLateTime;            // MSR_id for lateness at scheduler
	int m_idQualityRate;            // MSR_id for Quality rate requested
	int m_idQualityTime;            // MSR_id for Quality time requested
	int m_idDecision;               // MSR_id for decision code
	int m_idDuration;               // MSR_id for duration of a frame
	int m_idThrottle;               // MSR_id for audio-video throttling
									//int m_idDebug;                  // MSR_id for trace style debugging
									//int m_idSendQuality;          // MSR_id for timing the notifications per se
#endif // PERF
	REFERENCE_TIME m_trRememberStampForPerf;  // original time stamp of frame
											  // with no earliness fudges etc.
#ifdef PERF
	REFERENCE_TIME m_trRememberFrameForPerf;  // time when previous frame rendered

											  // debug...
	int m_idFrameAvg;
	int m_idWaitAvg;
#endif

	// PROPERTY PAGE
	// This has edit fields that show the user what's happening
	// These member variables hold these counts.

	int m_cFramesDropped;           // cumulative frames dropped IN THE RENDERER
	int m_cFramesDrawn;             // Frames since streaming started seen BY THE
									// RENDERER (some may be dropped upstream)

									// Next two support average sync offset and standard deviation of sync offset.
	LONGLONG m_iTotAcc;                  // Sum of accuracies in mSec
	LONGLONG m_iSumSqAcc;           // Sum of squares of (accuracies in mSec)

									// Next two allow jitter calculation.  Jitter is std deviation of frame time.
	REFERENCE_TIME m_trLastDraw;    // Time of prev frame (for inter-frame times)
	LONGLONG m_iSumSqFrameTime;     // Sum of squares of (inter-frame time in mSec)
	LONGLONG m_iSumFrameTime;            // Sum of inter-frame times in mSec

										 // To get performance statistics on frame rate, jitter etc, we need
										 // to record the lateness and inter-frame time.  What we actually need are the
										 // data above (sum, sum of squares and number of entries for each) but the data
										 // is generated just ahead of time and only later do we discover whether the
										 // frame was actually drawn or not.  So we have to hang on to the data
	int m_trLate;                   // hold onto frame lateness
	int m_trFrame;                  // hold onto inter-frame time

	int m_tStreamingStart;          // if streaming then time streaming started
									// else time of last streaming session
									// used for property page statistics
#ifdef PERF
	LONGLONG m_llTimeOffset;        // timeGetTime()*10000+m_llTimeOffset==ref time
#endif

public:


	CBaseVideoRenderer(REFCLSID RenderClass, // CLSID for this renderer
		__in_opt LPCTSTR pName,         // Debug ONLY description
		__inout_opt LPUNKNOWN pUnk,       // Aggregated owner object
		__inout HRESULT *phr);        // General OLE return code

	~CBaseVideoRenderer();

	// IQualityControl methods - Notify allows audio-video throttling

	STDMETHODIMP SetSink(IQualityControl * piqc);
	STDMETHODIMP Notify(IBaseFilter * pSelf, Quality q);

	// These provide a full video quality management implementation

	void OnRenderStart(IMediaSample *pMediaSample);
	void OnRenderEnd(IMediaSample *pMediaSample);
	void OnWaitStart();
	void OnWaitEnd();
	HRESULT OnStartStreaming();
	HRESULT OnStopStreaming();
	void ThrottleWait();

	// Handle the statistics gathering for our quality management

	void PreparePerformanceData(int trLate, int trFrame);
	virtual void RecordFrameLateness(int trLate, int trFrame);
	virtual void OnDirectRender(IMediaSample *pMediaSample);
	virtual HRESULT ResetStreamingTimes();
	BOOL ScheduleSample(IMediaSample *pMediaSample);
	HRESULT ShouldDrawSampleNow(IMediaSample *pMediaSample,
		__inout REFERENCE_TIME *ptrStart,
		__inout REFERENCE_TIME *ptrEnd);

	virtual HRESULT SendQuality(REFERENCE_TIME trLate, REFERENCE_TIME trRealStream);
	STDMETHODIMP JoinFilterGraph(__inout_opt IFilterGraph * pGraph, __in_opt LPCWSTR pName);

	//
	//  Do estimates for standard deviations for per-frame
	//  statistics
	//
	//  *piResult = (llSumSq - iTot * iTot / m_cFramesDrawn - 1) /
	//                            (m_cFramesDrawn - 2)
	//  or 0 if m_cFramesDrawn <= 3
	//
	HRESULT GetStdDev(
		int nSamples,
		__out int *piResult,
		LONGLONG llSumSq,
		LONGLONG iTot
	);
public:

	// IQualProp property page support

	STDMETHODIMP get_FramesDroppedInRenderer(__out int *cFramesDropped);
	STDMETHODIMP get_FramesDrawn(__out int *pcFramesDrawn);
	STDMETHODIMP get_AvgFrameRate(__out int *piAvgFrameRate);
	STDMETHODIMP get_Jitter(__out int *piJitter);
	STDMETHODIMP get_AvgSyncOffset(__out int *piAvg);
	STDMETHODIMP get_DevSyncOffset(__out int *piDev);

	// Implement an IUnknown interface and expose IQualProp

	DECLARE_IUNKNOWN
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out VOID **ppv);
};
