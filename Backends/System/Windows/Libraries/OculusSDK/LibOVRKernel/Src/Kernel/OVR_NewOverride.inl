/************************************************************************************

Filename    :   OVR_NewOverride.inl
Content     :   New override for LibOVR Allocator
Created     :   April 7, 2015
Authors     :   Paul Pedriana, Chris Taylor

Copyright   :   Copyright 2015-2016 Oculus VR, LLC All Rights reserved.

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

#ifndef OVR_NewOverride_inl
#define OVR_NewOverride_inl

#include "OVR_Allocator.h"

#include <new>

#if defined(_MSC_VER)
    #pragma warning(push, 0)
    #include <malloc.h>
    #include <crtdbg.h>

    #include <math.h>       // Work around VS header bug by #including math.h then intrin.h.
    #if (_MSC_VER >= 1500)
        #include <intrin.h>
    #endif
    #pragma warning(pop)
 
    #if (_MSC_VER >= 1600) // VS2010+
        #pragma warning(disable : 4986) // 'operator delete[]': exception specification does not match previous declaration.
    #endif
#endif



#if defined(_MSC_VER)

#if (_MSC_VER <= 1900)
    #define OVR_THROW_SPEC_NEW(X)        __pragma(warning(push)) __pragma(warning(disable: 4290 4987)) _THROW(,X) __pragma(warning(pop))
    #define OVR_THROW_SPEC_NEW_NONE()    _THROW(,)
    #define OVR_THROW_SPEC_DELETE_NONE() _THROW(,)
#else
	#define OVR_THROW_SPEC_NEW(x)        __pragma(warning(push)) __pragma(warning(disable: 4290 4987)) noexcept(false) __pragma(warning(pop))
	#define OVR_THROW_SPEC_NEW_NONE()    noexcept
	#define OVR_THROW_SPEC_DELETE_NONE() noexcept
#endif

#else
    #define OVR_THROW_SPEC_NEW(x)        throw(x)
    #define OVR_THROW_SPEC_NEW_NONE()    throw()
    #define OVR_THROW_SPEC_DELETE_NONE() throw()
#endif


// Add common decorators here as neeeded.
#define OVR_NEW_OVERRIDE_INLINE


OVR_NEW_OVERRIDE_INLINE void* operator new(size_t size) OVR_THROW_SPEC_NEW(std::bad_alloc)
{
    void* p = OVR_ALLOC(size);
 
    #if !defined(OVR_CPP_NO_EXCEPTIONS)
        if(!p)
            throw std::bad_alloc();
    #endif
 
    return p;
}
 
OVR_NEW_OVERRIDE_INLINE void* operator new[](size_t size) OVR_THROW_SPEC_NEW(std::bad_alloc)
{
    void* p = OVR_ALLOC(size);
 
    #if !defined(OVR_CPP_NO_EXCEPTIONS)
        if(!p)
            throw std::bad_alloc();
    #endif
 
    return p;
}


OVR_NEW_OVERRIDE_INLINE void* operator new(size_t size, std::nothrow_t&) OVR_THROW_SPEC_NEW_NONE()
{
    return OVR_ALLOC(size);
}
 
OVR_NEW_OVERRIDE_INLINE void* operator new[](size_t size, std::nothrow_t&) OVR_THROW_SPEC_NEW_NONE()
{
    return OVR_ALLOC(size);
}
 


OVR_NEW_OVERRIDE_INLINE void* operator new(size_t size, const std::nothrow_t&) OVR_THROW_SPEC_NEW_NONE()
{
    return OVR_ALLOC(size);
}
  
OVR_NEW_OVERRIDE_INLINE void* operator new[](size_t size, const std::nothrow_t&) OVR_THROW_SPEC_NEW_NONE()
{
    return OVR_ALLOC(size);
}


OVR_NEW_OVERRIDE_INLINE void operator delete(void* p) OVR_THROW_SPEC_DELETE_NONE()
{
    OVR_FREE(p);
}
 
OVR_NEW_OVERRIDE_INLINE void operator delete[](void* p) OVR_THROW_SPEC_DELETE_NONE()
{
    OVR_FREE(p);
}
 

OVR_NEW_OVERRIDE_INLINE void operator delete(void* p, std::nothrow_t&) OVR_THROW_SPEC_DELETE_NONE()
{
    OVR_FREE(p);
}
 
OVR_NEW_OVERRIDE_INLINE void operator delete[](void* p, std::nothrow_t&) OVR_THROW_SPEC_DELETE_NONE()
{
    OVR_FREE(p);
}
 

OVR_NEW_OVERRIDE_INLINE void operator delete(void* p, const std::nothrow_t&) OVR_THROW_SPEC_DELETE_NONE()
{
    OVR_FREE(p);
}
 
OVR_NEW_OVERRIDE_INLINE void operator delete[](void* p, const std::nothrow_t&) OVR_THROW_SPEC_DELETE_NONE()
{
    OVR_FREE(p);
}
 
 
 
// The following new/delete overrides are required under VC++ because it defines the following operator new versions of its own.
#if defined(_MSC_VER)

OVR_NEW_OVERRIDE_INLINE void* operator new(size_t n, const char* /*fileName*/, int /*line*/)
{
    return ::operator new(n);
}
 
OVR_NEW_OVERRIDE_INLINE void* operator new[](size_t n, const char* /*fileName*/, int /*line*/)
{
    return ::operator new[](n);
}
 
OVR_NEW_OVERRIDE_INLINE void operator delete(void* p, const char* /*fileName*/, int /*line*/)
{
    ::operator delete(p);
}
 
OVR_NEW_OVERRIDE_INLINE void operator delete[](void* p, const char* /*fileName*/, int /*line*/)
{
    ::operator delete[](p);
}

OVR_NEW_OVERRIDE_INLINE void* operator new(size_t n, int /*debug*/, const char* /*fileName*/, int /*line*/)
{
    return ::operator new (n);
}

OVR_NEW_OVERRIDE_INLINE void* operator new[](size_t n, int /*debug*/, const char* /*fileName*/, int /*line*/)
{
    return ::operator new[](n);
}

OVR_NEW_OVERRIDE_INLINE void __CRTDECL
operator delete(void* p, int /*debug*/, const char* /*fileName*/, int /*line*/)
    OVR_THROW_SPEC_DELETE_NONE() {
    ::operator delete(p);
}

OVR_NEW_OVERRIDE_INLINE void __CRTDECL
operator delete[](void* p, int /*debug*/, const char* /*fileName*/, int /*line*/)
    OVR_THROW_SPEC_DELETE_NONE() {
    ::operator delete[](p);
}

#endif



#endif // OVR_NewOverride_inl
