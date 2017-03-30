/************************************************************************************

Filename    :   OVR_String.cpp
Content     :   String UTF8 string implementation with copy-on-write semantics
                (thread-safe for assignment but not modification).
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

#include "OVR_String.h"

#include <stdlib.h>
#include <ctype.h>
#include <atomic>

#ifdef OVR_OS_QNX
# include <strings.h>
#endif

namespace OVR {

// Return the byte size of the UTF-8 string corresponding to pchar (not including null termination)
static size_t GetEncodeStringSize(const wchar_t* pchar, size_t length = StringIsNullTerminated)
{
    size_t len = 0;
    if (length == StringIsNullTerminated)
    {
        for (size_t i = 0; pchar[i] != 0; ++i)
        {
            len += UTF8Util::GetEncodeCharSize(pchar[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < length; ++i)
        {
            len += UTF8Util::GetEncodeCharSize(pchar[i]);
        }
    }
    return len;
}

static size_t StringStrlcpy(char* pDestUTF8, size_t destCharCountNotIncludingNull, const wchar_t* pSrcUCS, size_t sourceLength = StringIsNullTerminated)
{
    // String Data[] buffers always have one extra character for null-termination
    return UTF8Util::Strlcpy(pDestUTF8, destCharCountNotIncludingNull + 1, pSrcUCS, sourceLength);
}

static size_t StringStrlcpy(wchar_t* pDestUCS, size_t destCharCountNotIncludingNull, const char* pSrcUTF8, size_t sourceLength = StringIsNullTerminated)
{
    // String Data[] buffers always have one extra character for null-termination
    return UTF8Util::Strlcpy(pDestUCS, destCharCountNotIncludingNull + 1, pSrcUTF8, sourceLength);
}

#define String_LengthIsSize (size_t(1) << String::Flag_LengthIsSizeShift)

String::DataDesc String::NullData = { String_LengthIsSize, {1}, {0} };

String::String()
{
    pData = &NullData;
    pData->AddRef();
};

String::String(const char* pdata)
{
    // Obtain length in bytes; it doesn't matter if _data is UTF8.
    size_t size = pdata ? OVR_strlen(pdata) : 0; 
    pData = AllocDataCopy1(size, 0, pdata, size);
};

String::String(const char* pdata1, const char* pdata2, const char* pdata3)
{
    // Obtain length in bytes; it doesn't matter if _data is UTF8.
    size_t size1 = pdata1 ? OVR_strlen(pdata1) : 0; 
    size_t size2 = pdata2 ? OVR_strlen(pdata2) : 0; 
    size_t size3 = pdata3 ? OVR_strlen(pdata3) : 0; 

    DataDesc *pdataDesc = AllocDataCopy2(size1 + size2 + size3, 0,
                                         pdata1, size1, pdata2, size2);
    memcpy(pdataDesc->Data + size1 + size2, pdata3, size3);   
    pData = pdataDesc;    
}

String::String(const char* pdata, size_t size)
{
    OVR_ASSERT((size == 0) || (pdata != 0));
    pData = AllocDataCopy1(size, 0, pdata, size);
};


String::String(const InitStruct& src, size_t size)
{
    pData = AllocData(size, 0);
    src.InitString(GetData()->Data, size);
}

String::String(const String& src)
{    
    pData = src.GetData();
    pData->AddRef();
}

String::String(const StringBuffer& src)
{
    pData = AllocDataCopy1(src.GetSize(), 0, src.ToCStr(), src.GetSize());
}

String::String(const wchar_t* data)
{
    pData = &NullData;
    pData->AddRef();
    // Simplified logic for wchar_t constructor.
    if (data)    
        *this = data;    
}


String::DataDesc* String::AllocData(size_t size, size_t lengthIsSize)
{
    String::DataDesc* pdesc;

    if (size == 0)
    {
        pdesc = &NullData;
        pdesc->AddRef();
        return pdesc;
    }

    pdesc = (DataDesc*)OVR_ALLOC(sizeof(DataDesc)+ size);
    pdesc->Data[size] = 0;
    pdesc->RefCount = 1;
    pdesc->Size     = size | lengthIsSize;  
    return pdesc;
}


String::DataDesc* String::AllocDataCopy1(size_t size, size_t lengthIsSize,
                                         const char* pdata, size_t copySize)
{
    String::DataDesc* pdesc = AllocData(size, lengthIsSize);
    memcpy(pdesc->Data, pdata, copySize);
    return pdesc;
}

String::DataDesc* String::AllocDataCopy2(size_t size, size_t lengthIsSize,
                                         const char* pdata1, size_t copySize1,
                                         const char* pdata2, size_t copySize2)
{
    String::DataDesc* pdesc = AllocData(size, lengthIsSize);
    memcpy(pdesc->Data, pdata1, copySize1);
    memcpy(pdesc->Data + copySize1, pdata2, copySize2);
    return pdesc;
}


size_t String::GetLength() const 
{
    // Optimize length accesses for non-UTF8 character strings. 
    DataDesc* pdata = GetData();
    size_t    length, size = pdata->GetSize();
    
    if (pdata->LengthIsSize())
        return size;    
    
    length = (size_t)UTF8Util::GetLength(pdata->Data, size);
    
    if (length == size)
        pdata->Size |= String_LengthIsSize;
    
    return length;
}


uint32_t String::GetCharAt(size_t index) const 
{  
    intptr_t    i = (intptr_t) index;
    DataDesc*   pdata = GetData();
    const char* buf = pdata->Data;
    uint32_t    c;
    
    if (pdata->LengthIsSize())
    {
        OVR_ASSERT(index < pdata->GetSize());
        buf += i;
        return UTF8Util::DecodeNextChar_Advance0(&buf);
    }

    c = UTF8Util::GetCharAt(index, buf, pdata->GetSize());
    return c;
}

uint32_t String::GetFirstCharAt(size_t index, const char** offset) const
{
    DataDesc*   pdata = GetData();
    intptr_t    i = (intptr_t) index;
    const char* buf = pdata->Data;
    const char* end = buf + pdata->GetSize();
    uint32_t    c;

    do 
    {
        c = UTF8Util::DecodeNextChar_Advance0(&buf);
        i--;

        if (buf >= end)
        {
            // We've hit the end of the string; don't go further.
            OVR_ASSERT(i == 0);
            return c;
        }
    } while (i >= 0);

    *offset = buf;

    return c;
}

uint32_t String::GetNextChar(const char** offset) const
{
    return UTF8Util::DecodeNextChar(offset);
}



void String::AppendChar(uint32_t ch)
{
    DataDesc*   pdata = GetData();
    size_t      size = pdata->GetSize();
    char        buff[8];
    intptr_t    encodeSize = 0;

    // Converts ch into UTF8 string and fills it into buff.   
    UTF8Util::EncodeChar(buff, &encodeSize, ch);
    OVR_ASSERT(encodeSize >= 0);

    SetData(AllocDataCopy2(size + (size_t)encodeSize, 0,
                           pdata->Data, size, buff, (size_t)encodeSize));
    pdata->Release();
}


void String::AppendString(const wchar_t* pstr, size_t len)
{
    if (!pstr)
        return;

    size_t encodeSize = GetEncodeStringSize(pstr, len);
    if (encodeSize == 0)
        return;

    DataDesc*   pdata = GetData();
    size_t      oldSize = pdata->GetSize();    
    DataDesc*   pnewData = AllocDataCopy1(oldSize + encodeSize, 0, pdata->Data, oldSize);

    StringStrlcpy(pnewData->Data + oldSize, encodeSize, pstr, len);

    SetData(pnewData);
    pdata->Release();
}


void String::AppendString(const char* putf8str, size_t utf8StrSz)
{
    if (!putf8str || !utf8StrSz)
        return;
    if (utf8StrSz == StringIsNullTerminated)
        utf8StrSz = OVR_strlen(putf8str);

    DataDesc*   pdata = GetData();
    size_t      oldSize = pdata->GetSize();

    SetData(AllocDataCopy2(oldSize + utf8StrSz, 0, pdata->Data, oldSize, putf8str, utf8StrSz));
    pdata->Release();
}

void    String::AssignString(const InitStruct& src, size_t size)
{
    DataDesc*   poldData = GetData();
    DataDesc*   pnewData = AllocData(size, 0);
    src.InitString(pnewData->Data, size);
    SetData(pnewData);
    poldData->Release();
}

void    String::AssignString(const char* putf8str, size_t size)
{
    DataDesc* poldData = GetData();
    SetData(AllocDataCopy1(size, 0, putf8str, size));
    poldData->Release();
}

void    String::operator = (const char* pstr)
{
    AssignString(pstr, pstr ? OVR_strlen(pstr) : 0);
}

void    String::operator = (const wchar_t* pwstr)
{
    pwstr = pwstr ? pwstr : L"";
    size_t size = GetEncodeStringSize(pwstr);
    if (size == 0)
    {
        Clear();
        return;
    }

    DataDesc*   poldData = GetData();
    DataDesc*   pnewData = AllocData(size, 0);

    StringStrlcpy(pnewData->Data, size, pwstr);

    SetData(pnewData);
    poldData->Release();
}


void    String::operator = (const String& src)
{     
    DataDesc*    psdata = src.GetData();
    DataDesc*    pdata = GetData();    

    SetData(psdata);
    psdata->AddRef();
    pdata->Release();
}


void    String::operator = (const StringBuffer& src)
{ 
    DataDesc* polddata = GetData();    
    SetData(AllocDataCopy1(src.GetSize(), 0, src.ToCStr(), src.GetSize()));
    polddata->Release();
}

void    String::operator += (const String& src)
{
    DataDesc   *pourData = GetData(),
               *psrcData = src.GetData();
    size_t      ourSize  = pourData->GetSize(),
                srcSize  = psrcData->GetSize();
    size_t      lflag    = pourData->GetLengthFlag() & psrcData->GetLengthFlag();

    SetData(AllocDataCopy2(ourSize + srcSize, lflag,
                           pourData->Data, ourSize, psrcData->Data, srcSize));
    pourData->Release();
}


String   String::operator + (const char* str) const
{   
    String tmp1(*this);
    tmp1 += (str ? str : "");
    return tmp1;
}

String   String::operator + (const String& src) const
{ 
    String tmp1(*this);
    tmp1 += src;
    return tmp1;
}

void    String::Remove(size_t posAt, size_t removeLength)
{
    DataDesc*   pdata = GetData();
    size_t      oldSize = pdata->GetSize();    
    // Length indicates the number of characters to remove. 
    size_t      length = GetLength();

    // If index is past the string, nothing to remove.
    if (posAt >= length)
        return;
    // Otherwise, cap removeLength to the length of the string.
    if ((posAt + removeLength) > length)
        removeLength = length - posAt;

    // Get the byte position of the UTF8 char at position posAt.
    intptr_t bytePos    = UTF8Util::GetByteIndex(posAt, pdata->Data, oldSize);
    intptr_t removeSize = UTF8Util::GetByteIndex(removeLength, pdata->Data + bytePos, oldSize - bytePos);

    SetData(AllocDataCopy2(oldSize - removeSize, pdata->GetLengthFlag(),
                           pdata->Data, bytePos,
                           pData->Data + bytePos + removeSize, (oldSize - bytePos - removeSize)));
    pdata->Release();
}


String   String::Substring(size_t start, size_t end) const
{
    size_t length = GetLength();
    if ((start >= length) || (start >= end))
        return String();   
    if (end > length)
        end = length;

    DataDesc* pdata = GetData();
    
    // If size matches, we know the exact index range.
    if (pdata->LengthIsSize())
        return String(pdata->Data + start, end - start);
    
    // Get position of starting character and size
    intptr_t byteStart = UTF8Util::GetByteIndex(start, pdata->Data, pdata->GetSize());
    intptr_t byteSize  = UTF8Util::GetByteIndex(end - start, pdata->Data + byteStart, pdata->GetSize() - byteStart);

    OVR_ASSERT((byteStart >= 0) && (byteSize >= 0));

    return String(pdata->Data + byteStart, (size_t)byteSize);
}

void String::Clear()
{   
    NullData.AddRef();
    GetData()->Release();
    SetData(&NullData);
}


String   String::ToUpper() const 
{       
    uint32_t    c;
    const char* psource = GetData()->Data;
    const char* pend = psource + GetData()->GetSize();
    String      str;
    intptr_t    bufferOffset = 0;
    char        buffer[512];
    
    while(psource < pend)
    {
        do {            
            c = UTF8Util::DecodeNextChar_Advance0(&psource);
            UTF8Util::EncodeChar(buffer, &bufferOffset, OVR_towupper(wchar_t(c)));
        } while ((psource < pend) && (bufferOffset < intptr_t(sizeof(buffer)-8)));

        // Append string a piece at a time.
        str.AppendString(buffer, bufferOffset);
        bufferOffset = 0;
    }

    return str;
}

String   String::ToLower() const 
{
    uint32_t    c;
    const char* psource = GetData()->Data;
    const char* pend = psource + GetData()->GetSize();
    String      str;
    intptr_t    bufferOffset = 0;
    char        buffer[512];

    while(psource < pend)
    {
        do {
            c = UTF8Util::DecodeNextChar_Advance0(&psource);
            UTF8Util::EncodeChar(buffer, &bufferOffset, OVR_towlower(wchar_t(c)));
        } while ((psource < pend) && (bufferOffset < intptr_t(sizeof(buffer)-8)));

        // Append string a piece at a time.
        str.AppendString(buffer, bufferOffset);
        bufferOffset = 0;
    }

    return str;
}



String& String::Insert(const char* substr, size_t posAt, size_t strSize)
{
    DataDesc* poldData   = GetData();
    size_t    oldSize    = poldData->GetSize();
    size_t    insertSize = (strSize == StringIsNullTerminated) ? OVR_strlen(substr) : strSize;
    size_t    byteIndex  =  (poldData->LengthIsSize()) ?
                            posAt : (size_t)UTF8Util::GetByteIndex(posAt, poldData->Data, oldSize);

    // Insert past end of string degrades into AppendString to match UTF8Util::GetByteIndex case
    if (byteIndex > oldSize)
        byteIndex = oldSize;
    
    DataDesc* pnewData = AllocDataCopy2(oldSize + insertSize, 0,
                                        poldData->Data, byteIndex, substr, insertSize);
    memcpy(pnewData->Data + byteIndex + insertSize,
           poldData->Data + byteIndex, oldSize - byteIndex);
    SetData(pnewData);
    poldData->Release();
    return *this;
}

size_t String::InsertCharAt(uint32_t c, size_t posAt)
{
    char      buf[8];
    intptr_t  index = 0;
    UTF8Util::EncodeChar(buf, &index, c);
    OVR_ASSERT(index >= 0);
    buf[(size_t)index] = 0;

    Insert(buf, posAt, index);
    return (size_t)index;
}

// ***** Implement hash static functions

// Hash function
size_t String::BernsteinHashFunction(const void* pdataIn, size_t size, size_t seed)
{
    const uint8_t*    pdata   = (const uint8_t*) pdataIn;
    size_t          h       = seed;
    while (size > 0)
    {
        size--;
        h = ((h << 5) + h) ^ (unsigned) pdata[size];
    }

    return h;
}

// Hash function, case-insensitive
size_t String::BernsteinHashFunctionCIS(const void* pdataIn, size_t size, size_t seed)
{
    const uint8_t*    pdata = (const uint8_t*) pdataIn;
    size_t          h = seed;
    while (size > 0)
    {
        size--;
        h = ((h << 5) + h) ^ OVR_tolower(pdata[size]);
    }

    // Alternative: "sdbm" hash function, suggested at same web page above.
    // h = 0;
    // for bytes { h = (h << 16) + (h << 6) - hash + *p; }
    return h;
}



// ***** String Buffer used for Building Strings


#define OVR_SBUFF_DEFAULT_GROW_SIZE 512
// Constructors / Destructor.
StringBuffer::StringBuffer()
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
}

StringBuffer::StringBuffer(size_t growSize)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    SetGrowSize(growSize);
}

StringBuffer::StringBuffer(const char* data)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    AppendString(data);
}

StringBuffer::StringBuffer(const char* data, size_t dataSize)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    AppendString(data, dataSize);
}

StringBuffer::StringBuffer(const String& src)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    AppendString(src.ToCStr(), src.GetSize());
}

StringBuffer::StringBuffer(const StringBuffer& src)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    AppendString(src.ToCStr(), src.GetSize());
}

StringBuffer::StringBuffer(const wchar_t* data)
    : pData(NULL), Size(0), BufferSize(0), GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE), LengthIsSize(false)
{
    *this = data;
}

StringBuffer::~StringBuffer()
{
    if (pData)
        OVR_FREE(pData);
}

void StringBuffer::SetGrowSize(size_t growSize) 
{ 
    if (growSize <= 16)
        GrowSize = 16;
    else
    {
        uint8_t bits = Alg::UpperBit(uint32_t(growSize - 1));
        size_t size = (size_t)1 << bits;
        GrowSize = size == growSize ? growSize : size;
    }
}

size_t StringBuffer::GetLength() const
{
    size_t length, size = GetSize();
    if (LengthIsSize)
        return size;

    length = (size_t)UTF8Util::GetLength(pData, (size_t)GetSize());

    if (length == GetSize())
        LengthIsSize = true;
    return length;
}

void    StringBuffer::Reserve(size_t _size)
{
    if (_size >= BufferSize) // >= because of trailing zero! (!AB)
    {
        BufferSize = (_size + 1 + GrowSize - 1) & ~(GrowSize - 1);
        if (!pData)
            pData = (char*)OVR_ALLOC(BufferSize);
        else 
            pData = (char*)OVR_REALLOC(pData, BufferSize);
    }
}

void    StringBuffer::Resize(size_t _size)
{
    Reserve(_size);
    LengthIsSize = false;
    Size = _size;
    if (pData)
        pData[Size] = 0;
}

void StringBuffer::Clear()
{
    Resize(0);
}

// Appends a character
void     StringBuffer::AppendChar(uint32_t ch)
{
    char    buff[8];
    size_t  origSize = GetSize();

    // Converts ch into UTF8 string and fills it into buff. Also increments index according to the number of bytes
    // in the UTF8 string.
    intptr_t   srcSize = 0;
    UTF8Util::EncodeChar(buff, &srcSize, ch);
    OVR_ASSERT(srcSize >= 0);
    
    size_t size = origSize + srcSize;
    Resize(size);
    OVR_ASSERT(pData != NULL);
    memcpy(pData + origSize, buff, srcSize);
}

// Append a string
void     StringBuffer::AppendString(const wchar_t* pstr, size_t len)
{
    if (!pstr || !len)
        return;

    size_t srcSize  = GetEncodeStringSize(pstr, len);
    size_t origSize = GetSize();
    size_t size     = srcSize + origSize;

    Resize(size);
    OVR_ASSERT(pData != NULL);
    StringStrlcpy(pData + origSize, srcSize, pstr, len);
}

void      StringBuffer::AppendString(const char* putf8str, size_t utf8StrSz)
{
    if (!putf8str || !utf8StrSz)
        return;
    if (utf8StrSz == StringIsNullTerminated)
        utf8StrSz = OVR_strlen(putf8str);

    size_t  origSize = GetSize();
    size_t  size     = utf8StrSz + origSize;

    Resize(size);
    OVR_ASSERT(pData != NULL);
    memcpy(pData + origSize, putf8str, utf8StrSz);
}

// If pstr is NULL then the StringBuffer is cleared.
void      StringBuffer::operator = (const char* pstr)
{
    pstr = pstr ? pstr : "";
    size_t size = OVR_strlen(pstr);
    Resize(size);
    OVR_ASSERT((pData != NULL) || (size == 0));
    memcpy(pData, pstr, size);
}

// If pstr is NULL then the StringBuffer is cleared.
void      StringBuffer::operator = (const wchar_t* pstr)
{
    pstr = pstr ? pstr : L"";
    size_t size = GetEncodeStringSize(pstr);
    Resize(size);
    OVR_ASSERT((pData != NULL) || (size == 0));
    StringStrlcpy(pData, size, pstr);
}

void      StringBuffer::operator = (const String& src)
{
    const size_t size = src.GetSize();
    Resize(size);
    OVR_ASSERT((pData != NULL) || (size == 0));
    memcpy(pData, src.ToCStr(), size);
}

void      StringBuffer::operator = (const StringBuffer& src)
{
    Clear();
    AppendString(src.ToCStr(), src.GetSize());
}


// Inserts substr at posAt
void      StringBuffer::Insert(const char* substr, size_t posAt, size_t len)
{
    size_t    oldSize    = Size;
    size_t    insertSize = (len == StringIsNullTerminated) ? OVR_strlen(substr) : len;
    size_t    byteIndex  = LengthIsSize ? posAt : 
                           (size_t)UTF8Util::GetByteIndex(posAt, pData, (intptr_t)Size);

    OVR_ASSERT(byteIndex <= oldSize);
    Reserve(oldSize + insertSize);

    OVR_ASSERT(pData != NULL); // pData is unilaterally written to below.
    memmove(pData + byteIndex + insertSize, pData + byteIndex, oldSize - byteIndex + 1);
    memcpy (pData + byteIndex, substr, insertSize);
    LengthIsSize = false;
    Size = oldSize + insertSize;
    pData[Size] = 0;
}

// Inserts character at posAt
size_t    StringBuffer::InsertCharAt(uint32_t c, size_t posAt)
{
    char buf[8];
    intptr_t len = 0;
    UTF8Util::EncodeChar(buf, &len, c);
    OVR_ASSERT(len >= 0);
    buf[(size_t)len] = 0;

    Insert(buf, posAt, len);
    return (size_t)len;
}






std::wstring UTF8StringToUCSString(const char* pUTF8, size_t length)
{
    if (length == StringIsNullTerminated)
        length = OVR_strlen(pUTF8);

    std::wstring returnValue(length, wchar_t(0)); // We'll possibly trim this value below.

    // Note that Strlcpy doesn't handle UTF8 encoding errors.
    size_t decodedLength = StringStrlcpy(&returnValue[0], length, pUTF8, length);
    OVR_ASSERT(decodedLength <= length);

    returnValue.resize(decodedLength);

    return returnValue;
}

std::wstring UTF8StringToUCSString(const std::string& sUTF8)
{
    return UTF8StringToUCSString(sUTF8.data(), sUTF8.size());
}


std::wstring OVRStringToUCSString(const String& sOVRUTF8)
{
    return UTF8StringToUCSString(sOVRUTF8.ToCStr(), sOVRUTF8.GetSize());
}

std::string UCSStringToUTF8String(const wchar_t* pUCS, size_t length)
{
    if (length == StringIsNullTerminated)
        length = wcslen(pUCS);

    std::string sUTF8;
    size_t size = GetEncodeStringSize(pUCS, length);
    sUTF8.resize(size);
    StringStrlcpy(&sUTF8[0], size, pUCS, length);
    return sUTF8;
}

std::string UCSStringToUTF8String(const std::wstring& sUCS)
{
    return UCSStringToUTF8String(sUCS.data(), sUCS.size());
}

String UCSStringToOVRString(const wchar_t* pUCS, size_t length)
{
    if (length == StringIsNullTerminated)
        length = wcslen(pUCS);

    // We use a std::string intermediate because String doesn't support resize or assignment without preallocated data.
    const std::string sUTF8 = UCSStringToUTF8String(pUCS, length); 
    const String sOVRUTF8(sUTF8.data(), sUTF8.size());
    return sOVRUTF8;
}

String UCSStringToOVRString(const std::wstring& sUCS)
{
    return UCSStringToOVRString(sUCS.data(), sUCS.length());
}

} // OVR
