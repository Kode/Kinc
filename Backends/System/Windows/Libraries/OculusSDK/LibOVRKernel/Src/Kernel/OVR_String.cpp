/************************************************************************************

Filename    :   OVR_String.cpp
Content     :   String UTF8 string implementation with copy-on-write semantics
                (thread-safe for assignment but not modification).
Created     :   September 19, 2012
Notes       :

Copyright   :   Copyright (c) Facebook Technologies, LLC and its affiliates. All rights reserved.

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

namespace OVR {

// Return the byte size of the UTF-8 string corresponding to pchar (not including null termination)
static size_t GetEncodeStringSize(const wchar_t* pchar, size_t length = StringIsNullTerminated) {
  size_t len = 0;
  if (length == StringIsNullTerminated) {
    for (size_t i = 0; pchar[i] != 0; ++i) {
      len += UTF8Util::GetEncodeCharSize(pchar[i]);
    }
  } else {
    for (size_t i = 0; i < length; ++i) {
      len += UTF8Util::GetEncodeCharSize(pchar[i]);
    }
  }
  return len;
}

static size_t StringStrlcpy(
    char* pDestUTF8,
    size_t destCharCountNotIncludingNull,
    const wchar_t* pSrcUCS,
    size_t sourceLength = StringIsNullTerminated) {
  // String Data[] buffers always have one extra character for null-termination
  return UTF8Util::Strlcpy(pDestUTF8, destCharCountNotIncludingNull + 1, pSrcUCS, sourceLength);
}

static size_t StringStrlcpy(
    wchar_t* pDestUCS,
    size_t destCharCountNotIncludingNull,
    const char* pSrcUTF8,
    size_t sourceLength = StringIsNullTerminated) {
  // String Data[] buffers always have one extra character for null-termination
  return UTF8Util::Strlcpy(pDestUCS, destCharCountNotIncludingNull + 1, pSrcUTF8, sourceLength);
}

// ***** String Buffer used for Building Strings

#define OVR_SBUFF_DEFAULT_GROW_SIZE 512
// Constructors / Destructor.
StringBuffer::StringBuffer()
    : pData(NULL),
      Size(0),
      BufferSize(0),
      GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE),
      LengthIsSize(false) {}

StringBuffer::StringBuffer(size_t growSize)
    : pData(NULL),
      Size(0),
      BufferSize(0),
      GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE),
      LengthIsSize(false) {
  SetGrowSize(growSize);
}

StringBuffer::StringBuffer(const char* data)
    : pData(NULL),
      Size(0),
      BufferSize(0),
      GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE),
      LengthIsSize(false) {
  AppendString(data);
}

StringBuffer::StringBuffer(const char* data, size_t dataSize)
    : pData(NULL),
      Size(0),
      BufferSize(0),
      GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE),
      LengthIsSize(false) {
  AppendString(data, dataSize);
}

StringBuffer::StringBuffer(const String& src)
    : pData(NULL),
      Size(0),
      BufferSize(0),
      GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE),
      LengthIsSize(false) {
  AppendString(src.ToCStr(), src.GetSize());
}

StringBuffer::StringBuffer(const StringBuffer& src)
    : pData(NULL),
      Size(0),
      BufferSize(0),
      GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE),
      LengthIsSize(false) {
  AppendString(src.ToCStr(), src.GetSize());
}

StringBuffer::StringBuffer(const wchar_t* data)
    : pData(NULL),
      Size(0),
      BufferSize(0),
      GrowSize(OVR_SBUFF_DEFAULT_GROW_SIZE),
      LengthIsSize(false) {
  *this = data;
}

StringBuffer::~StringBuffer() {
  if (pData)
    OVR_FREE(pData);
}

void StringBuffer::SetGrowSize(size_t growSize) {
  if (growSize <= 16)
    GrowSize = 16;
  else {
    uint8_t bits = Alg::UpperBit(uint32_t(growSize - 1));
    size_t size = (size_t)1 << bits;
    GrowSize = size == growSize ? growSize : size;
  }
}

size_t StringBuffer::GetLength() const {
  size_t length, size = GetSize();
  if (LengthIsSize)
    return size;

  length = (size_t)UTF8Util::GetLength(pData, (size_t)GetSize());

  if (length == GetSize())
    LengthIsSize = true;
  return length;
}

void StringBuffer::Reserve(size_t _size) {
  if (_size >= BufferSize) // >= because of trailing zero! (!AB)
  {
    BufferSize = (_size + 1 + GrowSize - 1) & ~(GrowSize - 1);
    if (!pData)
      pData = (char*)OVR_ALLOC(BufferSize);
    else
      pData = (char*)OVR_REALLOC(pData, BufferSize);
  }
}

void StringBuffer::Resize(size_t _size) {
  Reserve(_size);
  LengthIsSize = false;
  Size = _size;
  if (pData)
    pData[Size] = 0;
}

void StringBuffer::Clear() {
  Resize(0);
}

// Appends a character
void StringBuffer::AppendChar(uint32_t ch) {
  char buff[8];
  size_t origSize = GetSize();

  // Converts ch into UTF8 string and fills it into buff. Also increments index according to the
  // number of bytes
  // in the UTF8 string.
  intptr_t srcSize = 0;
  UTF8Util::EncodeChar(buff, &srcSize, ch);
  OVR_ASSERT(srcSize >= 0);

  size_t size = origSize + srcSize;
  Resize(size);
  OVR_ASSERT(pData != NULL);
  memcpy(pData + origSize, buff, srcSize);
}

// Append a string
void StringBuffer::AppendString(const wchar_t* pstr, size_t len) {
  if (!pstr || !len)
    return;

  size_t srcSize = GetEncodeStringSize(pstr, len);
  size_t origSize = GetSize();
  size_t size = srcSize + origSize;

  Resize(size);
  OVR_ASSERT(pData != NULL);
  StringStrlcpy(pData + origSize, srcSize, pstr, len);
}

void StringBuffer::AppendString(const char* putf8str, size_t utf8StrSz) {
  if (!putf8str || !utf8StrSz)
    return;
  if (utf8StrSz == StringIsNullTerminated)
    utf8StrSz = OVR_strlen(putf8str);

  size_t origSize = GetSize();
  size_t size = utf8StrSz + origSize;

  Resize(size);
  OVR_ASSERT(pData != NULL);
  memcpy(pData + origSize, putf8str, utf8StrSz);
}

// If pstr is NULL then the StringBuffer is cleared.
void StringBuffer::operator=(const char* pstr) {
  pstr = pstr ? pstr : "";
  size_t size = OVR_strlen(pstr);
  Resize(size);
  OVR_ASSERT((pData != NULL) || (size == 0));
  memcpy(pData, pstr, size);
}

// If pstr is NULL then the StringBuffer is cleared.
void StringBuffer::operator=(const wchar_t* pstr) {
  pstr = pstr ? pstr : L"";
  size_t size = GetEncodeStringSize(pstr);
  Resize(size);
  OVR_ASSERT((pData != NULL) || (size == 0));
  StringStrlcpy(pData, size, pstr);
}

void StringBuffer::operator=(const String& src) {
  const size_t size = src.GetSize();
  Resize(size);
  OVR_ASSERT((pData != NULL) || (size == 0));
  memcpy(pData, src.ToCStr(), size);
}

void StringBuffer::operator=(const StringBuffer& src) {
  Clear();
  AppendString(src.ToCStr(), src.GetSize());
}

// Inserts substr at posAt
void StringBuffer::Insert(const char* substr, size_t posAt, size_t len) {
  size_t oldSize = Size;
  size_t insertSize = (len == StringIsNullTerminated) ? OVR_strlen(substr) : len;
  size_t byteIndex =
      LengthIsSize ? posAt : (size_t)UTF8Util::GetByteIndex(posAt, pData, (intptr_t)Size);

  OVR_ASSERT(byteIndex <= oldSize);
  Reserve(oldSize + insertSize);

  OVR_ASSERT(pData != NULL); // pData is unilaterally written to below.
  memmove(pData + byteIndex + insertSize, pData + byteIndex, oldSize - byteIndex + 1);
  memcpy(pData + byteIndex, substr, insertSize);
  LengthIsSize = false;
  Size = oldSize + insertSize;
  pData[Size] = 0;
}

// Inserts character at posAt
size_t StringBuffer::InsertCharAt(uint32_t c, size_t posAt) {
  char buf[8];
  intptr_t len = 0;
  UTF8Util::EncodeChar(buf, &len, c);
  OVR_ASSERT(len >= 0);
  buf[(size_t)len] = 0;

  Insert(buf, posAt, len);
  return (size_t)len;
}

std::string StringVsprintf(const char* format, va_list args) {
  char buffer[512]; // We first try writing into this buffer. If it's not enough then use a string.

  va_list tmp_args;
  va_copy(tmp_args, args);
  const int requiredStrlen = vsnprintf(buffer, sizeof(buffer), format, tmp_args);
  va_end(tmp_args);

  if (requiredStrlen <
      static_cast<int>(sizeof(buffer))) { // If the entire result fits into the buffer.
    return std::string(buffer, requiredStrlen);
  }

  std::string result(requiredStrlen, '\0');
  std::vsnprintf(&result[0], result.size(), format, args);

  return result;
}

std::string& AppendSprintf(std::string& s, const char* format, ...) {
  va_list args;
  va_start(args, format);
  s += StringVsprintf(format, args);
  va_end(args);
  return s;
}

std::wstring UTF8StringToUCSString(const char* pUTF8, size_t length) {
  if (length == StringIsNullTerminated)
    length = OVR_strlen(pUTF8);

  std::wstring returnValue(length, wchar_t(0)); // We'll possibly trim this value below.

  // Note that Strlcpy doesn't handle UTF8 encoding errors.
  size_t decodedLength = StringStrlcpy(&returnValue[0], length, pUTF8, length);
  OVR_ASSERT(decodedLength <= length);

  returnValue.resize(decodedLength);

  return returnValue;
}

std::wstring UTF8StringToUCSString(const std::string& sUTF8) {
  return UTF8StringToUCSString(sUTF8.data(), sUTF8.size());
}

std::wstring OVRStringToUCSString(const String& sOVRUTF8) {
  return UTF8StringToUCSString(sOVRUTF8.ToCStr(), sOVRUTF8.GetSize());
}

std::string UCSStringToUTF8String(const wchar_t* pUCS, size_t length) {
  if (length == StringIsNullTerminated)
    length = wcslen(pUCS);

  std::string sUTF8;
  size_t size = GetEncodeStringSize(pUCS, length);
  sUTF8.resize(size);
  StringStrlcpy(&sUTF8[0], size, pUCS, length);
  return sUTF8;
}

std::string UCSStringToUTF8String(const std::wstring& sUCS) {
  return UCSStringToUTF8String(sUCS.data(), sUCS.size());
}

String UCSStringToOVRString(const wchar_t* pUCS, size_t length) {
  if (length == StringIsNullTerminated)
    length = wcslen(pUCS);

  // We use a std::string intermediate because String doesn't support resize or assignment without
  // preallocated data.
  const std::string sUTF8 = UCSStringToUTF8String(pUCS, length);
  const String sOVRUTF8(sUTF8.data(), sUTF8.size());
  return sOVRUTF8;
}

String UCSStringToOVRString(const std::wstring& sUCS) {
  return UCSStringToOVRString(sUCS.data(), sUCS.length());
}

} // namespace OVR
