/************************************************************************************

PublicHeader:   Kernel
Filename    :   OVR_RefCount.h
Content     :   Reference counting implementation headers
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2014-2016 Oculus VR, LLC All Rights reserved.

Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License"); 
you may not use the Oculus VR Rift SDK except in compliance with the License, 
which is provided at the time of installation or download, or which 
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-3.3 

Unless required by applicable law or agreed to in writing, the Oculus VR SDK 
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#ifndef OVR_RefCount_h
#define OVR_RefCount_h

#include "OVR_Types.h"
#include "OVR_Allocator.h"

namespace OVR {

//-----------------------------------------------------------------------------------
// ***** Reference Counting

// There are three types of reference counting base classes:
//
//  RefCountBase     - Provides thread-safe reference counting (Default).
//  RefCountBaseNTS  - Non Thread Safe version of reference counting.


// ***** Declared classes

template<class C>
class   RefCountBase;
template<class C>
class   RefCountBaseNTS;

class   RefCountImpl;
class   RefCountNTSImpl;


//-----------------------------------------------------------------------------------
// ***** Implementation For Reference Counting

// RefCountImplCore holds RefCount value and defines a few utility
// functions shared by all implementations.

class RefCountImplCore
{
protected:
    std::atomic<int> RefCount = { 1 };

public:
    // RefCountImpl constructor always initializes RefCount to 1 by default.
    OVR_FORCE_INLINE RefCountImplCore() { }

    // Need virtual destructor
    // This:    1. Makes sure the right destructor's called.
    //          2. Makes us have VTable, necessary if we are going to have format needed by InitNewMem()
    virtual ~RefCountImplCore();

    // Debug method only.
    int GetRefCount() const { return RefCount;  }

    // This logic is used to detect invalid 'delete' calls of reference counted
    // objects. Direct delete calls are not allowed on them unless they come in
    // internally from Release.
#ifdef OVR_BUILD_DEBUG    
    static void   OVR_CDECL  reportInvalidDelete(void *pmem);
    inline static void checkInvalidDelete(RefCountImplCore *pmem)
    {
        if (pmem->RefCount != 0)
            reportInvalidDelete(pmem);
    }
#else
    inline static void checkInvalidDelete(RefCountImplCore *) { }
#endif

    // Base class ref-count content should not be copied.
    void operator = (const RefCountImplCore &) { }  
};

class RefCountNTSImplCore
{
protected:
    mutable int RefCount;

public:
    // RefCountImpl constructor always initializes RefCount to 1 by default.
    OVR_FORCE_INLINE RefCountNTSImplCore() : RefCount(1) { }

    // Need virtual destructor
    // This:    1. Makes sure the right destructor's called.
    //          2. Makes us have VTable, necessary if we are going to have format needed by InitNewMem()
    virtual ~RefCountNTSImplCore();

    // Debug method only.
    int GetRefCount() const 
    {
        return RefCount;
    }

    // This logic is used to detect invalid 'delete' calls of reference counted
    // objects. Direct delete calls are not allowed on them unless they come in
    // internally from Release.
#ifdef OVR_BUILD_DEBUG    
    static void   OVR_CDECL  reportInvalidDelete(void *pmem);
    OVR_FORCE_INLINE static void checkInvalidDelete(RefCountNTSImplCore *pmem)
    {
        if (pmem->RefCount != 0)
            reportInvalidDelete(pmem);
    }
#else
    OVR_FORCE_INLINE static void checkInvalidDelete(RefCountNTSImplCore *) { }
#endif

    // Base class ref-count content should not be copied.
    void operator = (const RefCountNTSImplCore &) { }  
};



// RefCountImpl provides Thread-Safe implementation of reference counting, so
// it should be used by default in most places.

class RefCountImpl : public RefCountImplCore
{
public:
    // Thread-Safe Ref-Count Implementation.
    void    AddRef();
    void    Release();   
};

// RefCountVImpl provides Thread-Safe implementation of reference counting, plus,
// virtual AddRef and Release.

class RefCountVImpl : virtual public RefCountImplCore
{
public:
    // Thread-Safe Ref-Count Implementation.
    virtual void      AddRef();
    virtual void      Release();   
};


// RefCountImplNTS provides Non-Thread-Safe implementation of reference counting,
// which is slightly more efficient since it doesn't use atomics.

class RefCountNTSImpl : public RefCountNTSImplCore
{
public:
    OVR_FORCE_INLINE void    AddRef() const
    {
        RefCount++;
    }

    void    Release() const;   
};



// RefCountBaseStatImpl<> is a common class that adds new/delete override with Stat tracking
// to the reference counting implementation. Base must be one of the RefCountImpl classes.

template<class Base>
class RefCountBaseStatImpl : public Base
{
public:
    RefCountBaseStatImpl() { }
     
    // *** Override New and Delete

    // DOM-IGNORE-BEGIN
    // Undef new temporarily if it is being redefined
#ifdef OVR_DEFINE_NEW
#undef new
#endif

#ifdef OVR_BUILD_DEBUG
    // Custom check used to detect incorrect calls of 'delete' on ref-counted objects.
    #define OVR_REFCOUNTALLOC_CHECK_DELETE(class_name, p)   \
        do {if (p) Base::checkInvalidDelete((class_name*)p); } while(0)
#else
    #define OVR_REFCOUNTALLOC_CHECK_DELETE(class_name, p)
#endif

    // Redefine all new & delete operators.
    OVR_MEMORY_REDEFINE_NEW_IMPL(Base, OVR_REFCOUNTALLOC_CHECK_DELETE)

#undef OVR_REFCOUNTALLOC_CHECK_DELETE

#ifdef OVR_DEFINE_NEW
#define new OVR_DEFINE_NEW
#endif
        // OVR_BUILD_DEFINE_NEW
        // DOM-IGNORE-END
};


template<class Base>
class RefCountBaseStatVImpl : virtual public Base
{
public:
    RefCountBaseStatVImpl() { }

    // *** Override New and Delete

    // DOM-IGNORE-BEGIN
    // Undef new temporarily if it is being redefined
#ifdef OVR_DEFINE_NEW
#undef new
#endif

#define OVR_REFCOUNTALLOC_CHECK_DELETE(class_name, p)

    // Redefine all new & delete operators.
    OVR_MEMORY_REDEFINE_NEW_IMPL(Base, OVR_REFCOUNTALLOC_CHECK_DELETE)

#undef OVR_REFCOUNTALLOC_CHECK_DELETE

#ifdef OVR_DEFINE_NEW
#define new OVR_DEFINE_NEW
#endif
        // OVR_BUILD_DEFINE_NEW
        // DOM-IGNORE-END
};



//-----------------------------------------------------------------------------------
// *** End user RefCountBase<> classes


// RefCountBase is a base class for classes that require thread-safe reference
// counting; it also overrides the new and delete operators to use MemoryHeap.
//
// Reference counted objects start out with RefCount value of 1. Further lifetime
// management is done through the AddRef() and Release() methods, typically
// hidden by Ptr<>.

template<class C>
class RefCountBase : public RefCountBaseStatImpl<RefCountImpl>
{
public:    
    // Constructor.
    OVR_FORCE_INLINE RefCountBase() : RefCountBaseStatImpl<RefCountImpl>() { }    
};

// RefCountBaseV is the same as RefCountBase but with virtual AddRef/Release

template<class C>
class RefCountBaseV : virtual public RefCountBaseStatVImpl<RefCountVImpl>
{
public:    
    // Constructor.
    OVR_FORCE_INLINE RefCountBaseV() : RefCountBaseStatVImpl<RefCountVImpl>() { }    
};


// RefCountBaseNTS is a base class for classes that require Non-Thread-Safe reference
// counting; it also overrides the new and delete operators to use MemoryHeap.
// This class should only be used if all pointers to it are known to be assigned,
// destroyed and manipulated within one thread.
//
// Reference counted objects start out with RefCount value of 1. Further lifetime
// management is done through the AddRef() and Release() methods, typically
// hidden by Ptr<>.

template<class C>
class RefCountBaseNTS : public RefCountBaseStatImpl<RefCountNTSImpl>
{
public:    
    // Constructor.
    OVR_FORCE_INLINE RefCountBaseNTS() : RefCountBaseStatImpl<RefCountNTSImpl>() { }    
};




//-----------------------------------------------------------------------------------
// ***** Ref-Counted template pointer
//
// Automatically AddRefs and Releases interfaces
//
// Note: Some of the member functions take C& as opposed to C* arguments:
//    Ptr(C&)
//    const Ptr<C>& operator= (C&)
//    Ptr<C>& SetPtr(C&)
// These functions do not AddRef the assigned C& value, unlike the C* assignment 
// functions. Thus the purpose of these functions is for the Ptr instance to 
// assume ownership of a C reference count. Example usage:
//     Ptr<Widget> w = *new Widget;   // Calls the Ptr(C&) constructor. Note that the Widget constructor sets initial refcount to 1.
//

template<class C>
class Ptr
{
protected:
    C   *pObject;

public:

    // Constructors
    OVR_FORCE_INLINE Ptr() 
        : pObject(0)
    { }

    // This constructor adopts the object's existing reference count rather than increment it.
    OVR_FORCE_INLINE Ptr(C &robj) 
        : pObject(&robj)
    { }

    OVR_FORCE_INLINE Ptr(C *pobj)
    {
        if (pobj) 
            pobj->AddRef();   
        pObject = pobj;
    }

    OVR_FORCE_INLINE Ptr(const Ptr<C> &src)
    {
        if (src.pObject) 
            src.pObject->AddRef();     
        pObject = src.pObject;
    }

    template<class R>
    OVR_FORCE_INLINE Ptr(Ptr<R> &src)
    {
        if (src) 
            src->AddRef();
        pObject = src;
    }

    // Destructor
    OVR_FORCE_INLINE ~Ptr()
    {
        if (pObject)
            pObject->Release();        
    }

    // Compares
    OVR_FORCE_INLINE bool operator == (const Ptr &other) const
    {
        return pObject == other.pObject;
    }

    OVR_FORCE_INLINE bool operator != (const Ptr &other) const
    {
        return pObject != other.pObject;
    }

    OVR_FORCE_INLINE bool operator == (C *pother) const
    {
        return pObject == pother;
    }

    OVR_FORCE_INLINE bool operator != (C *pother) const
    {
        return pObject != pother;
    }

    OVR_FORCE_INLINE bool operator < (const Ptr &other) const
    {
        return pObject < other.pObject;
    }

    // Assignment
    template<class R>
    OVR_FORCE_INLINE const Ptr<C>& operator = (const Ptr<R> &src)
    {
        // By design we don't check for src == pObject, as we don't expect that to be the case the large majority of the time.
        if (src)
            src->AddRef();
        if (pObject)
            pObject->Release();        
        pObject = src;
        return *this;
    }
 
    // Specialization
    OVR_FORCE_INLINE const Ptr<C>& operator = (const Ptr<C> &src)
    {
        if (src)
            src->AddRef();
        if (pObject)
            pObject->Release();        
        pObject = src;
        return *this;
    }   
    
    OVR_FORCE_INLINE const Ptr<C>& operator = (C *psrc)
    {
        if (psrc)
            psrc->AddRef();
        if (pObject)
            pObject->Release();        
        pObject = psrc;
        return *this;
    }

    // This operator adopts the object's existing reference count rather than increment it.
    OVR_FORCE_INLINE const Ptr<C>& operator = (C &src)
    {       
        if (pObject)
            pObject->Release();        
        pObject = &src;
        return *this;
    }

    // Set Assignment
    template<class R>
    OVR_FORCE_INLINE Ptr<C>& SetPtr(const Ptr<R> &src)
    {
        if (src)
            src->AddRef();
        if (pObject)
            pObject->Release();
        pObject = src;
        return *this;
    }
    // Specialization
    OVR_FORCE_INLINE Ptr<C>& SetPtr(const Ptr<C> &src)
    {
        if (src)
            src->AddRef();
        if (pObject)
            pObject->Release();
        pObject = src;
        return *this;
    }

    OVR_FORCE_INLINE Ptr<C>& SetPtr(C *psrc)
    {
        if (psrc)
            psrc->AddRef();
        if (pObject)
            pObject->Release();
        pObject = psrc;
        return *this;
    }

    // This function adopts the object's existing reference count rather than increment it.
    OVR_FORCE_INLINE Ptr<C>& SetPtr(C &src)
    {       
        if (pObject)
            pObject->Release();
        pObject = &src;
        return *this;
    }

    // Nulls ref-counted pointer without decrement
    OVR_FORCE_INLINE void    NullWithoutRelease()    
    { 
        pObject = 0;    
    }

    // Clears the pointer to the object
    OVR_FORCE_INLINE void    Clear()
    {
        if (pObject) 
            pObject->Release();
        pObject = 0;
    }

    // Obtain pointer reference directly, for D3D interfaces
    OVR_FORCE_INLINE C*& GetRawRef()
    {
        return pObject;
    }

    // Access Operators
    OVR_FORCE_INLINE C* GetPtr() const
    {
        return pObject;
    }

    OVR_FORCE_INLINE C& operator * () const
    {
        return *pObject;
    }

    OVR_FORCE_INLINE C* operator -> ()  const
    {
        return pObject;
    }

    // Conversion                   
    OVR_FORCE_INLINE operator C* () const
    {
        return pObject;
    }

};


// LockedPtr
//
// Helper class to simplify thread-safety of the TrackingManager.
// It wraps the Ptr<> object it contains in a Lock.
template<class T>
class LockedPtr
{
public:
    LockedPtr(Lock* lock = nullptr) :
        TheLock(lock)
    {
    }

    void Set(T* value)
    {
        OVR_ASSERT(TheLock);
        TheLock->DoLock();
        Ptr<T> oldPtr = ThePtr; // Keep a reference to the old ptr
        ThePtr = value; // Change/decrement the old ptr (cannot die here due to oldPtr)
        TheLock->Unlock();

        // Release the old Ptr reference here, outside of the lock
        // so that the object will not die while TheLock is held.
    }

    template<class S>
    void Get(Ptr<S>& outputPtr) const
    {
        OVR_ASSERT(TheLock);
        TheLock->DoLock();
        Ptr<T> retval = ThePtr;
        TheLock->Unlock();
        outputPtr = retval;
    }

protected:
    mutable Lock* TheLock;
    Ptr<T> ThePtr;
};


} // OVR

#endif
