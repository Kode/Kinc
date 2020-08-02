/************************************************************************************

Filename    :   OVR_String.h
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

#ifndef OVR_String_h
#define OVR_String_h

#include "OVR_Types.h"
#include "OVR_Allocator.h"
#include "OVR_UTF8Util.h"
#include "OVR_Atomic.h"
#include "OVR_Std.h"
#include "OVR_Alg.h"
#include <string>
#include <stdarg.h>

namespace OVR {

class String;
class StringBuffer;

// Special/default null-terminated length argument
const size_t StringIsNullTerminated = size_t(-1);

//-----------------------------------------------------------------------------------
// ***** String Class

// String is UTF8 based string class with copy-on-write implementation
// for assignment.

class String : public std::string {
 public:
  typedef std::string inherited;

  // Constructors / Destructors.
  String() {}

  String(const char* data) {
    if (data)
      inherited::assign(data);
  }

  String(const char* data1, const char* pdata2, const char* pdata3 = nullptr) {
    if (data1)
      inherited::append(data1);
    if (pdata2)
      inherited::append(pdata2);
    if (pdata3)
      inherited::append(pdata3);
  }

  String(const char* data, size_t buflen) {
    if (data)
      inherited::assign(data, buflen);
  }

  String(const String& src) {
    inherited::assign(src.data(), src.length());
  }

  String(const std::string& src) {
    inherited::assign(src.data(), src.length());
  }

  explicit String(const wchar_t* data) {
    if (data)
      String::operator=(data); // Need to do UCS2->UTF8 conversion
  }

  void Clear() {
    inherited::clear();
  }

  operator const char*() const {
    return inherited::c_str();
  }

  const char* ToCStr() const {
    return inherited::c_str();
  }

  // Byte length.
  size_t GetSize() const {
    return inherited::size();
  }

  bool IsEmpty() const {
    return inherited::empty();
  }

  // Returns number of Unicode codepoints (not byte length).
  size_t GetLength() const {
    return (size_t)UTF8Util::GetLength(inherited::data(), inherited::size());
  }

  // Return Unicode codepoint count.
  int GetLengthI() const {
    return (int)UTF8Util::GetLength(inherited::data(), inherited::size());
  }

  // Return Unicode codepoint at the specified codepoint index.
  uint32_t GetCharAt(size_t index) const {
    return UTF8Util::GetCharAt(index, inherited::data(), inherited::size());
  }

  // Get first Unicode codepoint.
  uint32_t Front() const {
    return UTF8Util::GetCharAt(0, inherited::data(), inherited::size());
  }

  // Get last Unicode codepoint.
  uint32_t Back() const {
    OVR_ASSERT(!inherited::empty());
    return GetCharAt(GetSize() - 1);
  }

  // Append a Unicode codepoint.
  void AppendChar(uint32_t ch) {
    char buff[8]; // Max possible UTF8 sequence is 6 bytes.
    intptr_t encodeSize = 0;
    UTF8Util::EncodeChar(buff, &encodeSize, ch);
    inherited::append(buff, encodeSize);
  }

  // Append Unicode codepoints
  void AppendString(const wchar_t* pstr, size_t len = StringIsNullTerminated) {
    if (pstr) {
      if (len == StringIsNullTerminated)
        len = wcslen(pstr);

      while (len-- > 0)
        AppendChar(*pstr++);
    }
  }

  // Append UTF8 stirng of a given byte size.
  void AppendString(const char* putf8str, size_t utf8StrSz = StringIsNullTerminated) {
    if (putf8str) {
      if (utf8StrSz == StringIsNullTerminated)
        inherited::append(putf8str);
      else
        inherited::append(putf8str, utf8StrSz);
    }
  }

  // Assigns string with known byte size.
  void AssignString(const char* putf8str, size_t size) {
    if (putf8str || (size == 0))
      inherited::assign(putf8str, size);
  }

  // Remove 'removeLength' Unicode codepoints starting at the 'posAt' Unicode codepoint.
  void Remove(size_t posAt, size_t removeLength = 1) {
    size_t length = GetLength(); // Unicode size.

    if (posAt < length) {
      size_t oldSize = inherited::size(); // Byte size.

      if ((posAt + removeLength) > length)
        removeLength = (length - posAt);

      intptr_t bytePos = UTF8Util::GetByteIndex(posAt, inherited::data(), oldSize);

      intptr_t removeSize =
          UTF8Util::GetByteIndex(removeLength, inherited::data() + bytePos, oldSize - bytePos);

      inherited::erase(bytePos, removeSize);
    }
  }

  // Removes the last Unicode codepoint.
  void PopBack() {
    if (!inherited::empty())
      Remove(GetLength() - 1, 1);
  }

  // Returns a String that's a substring of this.
  //  start is the index of the first Unicode codepoint you want to include.
  //  end is the index one past the last Unicode codepoint you want to include.
  String Substring(size_t start, size_t end) const {
    size_t length = GetLength();

    if ((start >= length) || (start >= end))
      return String();

    if (end > length)
      end = length;

    // Get position of starting character and size
    intptr_t byteStart = UTF8Util::GetByteIndex(start, inherited::data(), inherited::size());
    intptr_t byteSize = UTF8Util::GetByteIndex(
        end - start, inherited::data() + byteStart, inherited::size() - byteStart);

    return String(inherited::data() + byteStart, (size_t)byteSize);
  }

  // Insert a UTF8 string at the Unicode codepoint posAt.
  String& Insert(const char* substr, size_t posAt, size_t strSize = StringIsNullTerminated) {
    if (substr) {
      if (strSize == StringIsNullTerminated)
        strSize = strlen(substr);

      size_t byteIndex = UTF8Util::GetByteIndex(posAt, inherited::c_str(), inherited::size());

      if (byteIndex > inherited::size())
        byteIndex = inherited::size();

      inherited::insert(byteIndex, substr, strSize);
    }

    return *this;
  }

  // UTF8 compare
  static int CompareNoCase(const char* a, const char* b) {
    return OVR_stricmp(a, b);
  }

  // UTF8 compare
  static int CompareNoCase(const char* a, const char* b, size_t byteSize) {
    return OVR_strnicmp(a, b, byteSize);
  }

  // Hash function, case-insensitive
  static size_t BernsteinHashFunctionCIS(const void* pdataIn, size_t size, size_t seed = 5381) {
    const uint8_t* pdata = (const uint8_t*)pdataIn;
    size_t h = seed;
    while (size > 0) {
      size--;
      h = ((h << 5) + h) ^ OVR_tolower(pdata[size]);
    }
    return h;
  }

  // Hash function, case-sensitive
  static size_t BernsteinHashFunction(const void* pdataIn, size_t size, size_t seed = 5381) {
    const uint8_t* pdata = (const uint8_t*)pdataIn;
    size_t h = seed;
    while (size > 0) {
      size--;
      h = ((h << 5) + h) ^ (unsigned)pdata[size];
    }
    return h;
  }

  // Absolute paths can star with:
  //  - protocols:        'file://', 'http://'
  //  - windows drive:    'c:\'
  //  - UNC share name:   '\\share'
  //  - unix root         '/'
  static bool HasAbsolutePath(const char* path);

  static bool HasExtension(const char* path);

  static bool HasProtocol(const char* path);

  bool HasAbsolutePath() const {
    return HasAbsolutePath(inherited::c_str());
  }

  bool HasExtension() const {
    return HasExtension(inherited::c_str());
  }

  bool HasProtocol() const {
    return HasProtocol(inherited::c_str());
  }

  String GetProtocol() const; // Returns protocol, if any, with trailing '://'.

  String GetPath() const; // Returns path with trailing '/'.

  String GetFilename() const; // Returns filename, including extension.

  String GetExtension() const; // Returns extension with a dot.

  void StripProtocol(); // Strips front protocol, if any, from the string.

  void StripExtension(); // Strips off trailing extension.

  void operator=(const char* str) {
    if (str)
      inherited::assign(str);
    else
      inherited::clear();
  }

  void operator=(const wchar_t* str) {
    inherited::clear();

    while (str && *str)
      AppendChar(*str++);
  }

  void operator=(const String& src) {
    inherited::assign(src.data(), src.size());
  }

  void operator+=(const String& src) {
    inherited::append(src.data(), src.size());
  }

  void operator+=(const char* psrc) {
    if (psrc)
      inherited::append(psrc);
  }

  void operator+=(const wchar_t* psrc) {
    if (psrc)
      AppendString(psrc);
  }

  // Append a Unicode codepoint.
  // Note that this function seems amiss by taking char as an argument instead of uint32_t or
  // wchar_t.
  void operator+=(char ch) {
    AppendChar(ch);
  }

  String operator+(const char* str) const {
    String temp(*this);
    if (str)
      temp += str;
    return temp;
  }

  String operator+(const String& src) const {
    String temp(*this);
    temp += src;
    return temp;
  }

  bool operator==(const String& str) const {
    return (OVR_strcmp(inherited::c_str(), str.c_str()) == 0);
  }

  bool operator!=(const String& str) const {
    return !operator==(str);
  }

  bool operator==(const char* str) const {
    return OVR_strcmp(inherited::c_str(), (str ? str : "")) == 0;
  }

  bool operator!=(const char* str) const {
    return !operator==(str);
  }

  bool operator<(const char* pstr) const {
    return OVR_strcmp(inherited::c_str(), (pstr ? pstr : "")) < 0;
  }

  bool operator<(const String& str) const {
    return *this < str.c_str();
  }

  bool operator>(const char* pstr) const {
    return OVR_strcmp(inherited::c_str(), (pstr ? pstr : "")) > 0;
  }

  bool operator>(const String& str) const {
    return *this > str.c_str();
  }

  int CompareNoCase(const char* pstr) const {
    return CompareNoCase(inherited::c_str(), (pstr ? pstr : ""));
  }

  int CompareNoCase(const String& str) const {
    return CompareNoCase(inherited::c_str(), str.c_str());
  }

  int CompareNoCaseStartsWith(const String& str) const {
    // Problem: the original version of this used GetLength, which seems like a bug because
    // CompareNoCase takes a byte length. Need to look back in the OVR_String.h history to see if
    // the bug has always been there.
    return CompareNoCase(inherited::c_str(), str.c_str(), str.size());
  }

  // Accesses raw bytes
  const char& operator[](int index) const {
    OVR_ASSERT(index >= 0 && (size_t)index < inherited::size());
    return inherited::operator[](index);
  }

  const char& operator[](size_t index) const {
    OVR_ASSERT(index < inherited::size());
    return inherited::operator[](index);
  }

  struct NoCaseKey {
    const String* pStr;
    NoCaseKey(const String& str) : pStr(&str){};
  };

  bool operator==(const NoCaseKey& strKey) const {
    return (CompareNoCase(ToCStr(), strKey.pStr->ToCStr()) == 0);
  }

  bool operator!=(const NoCaseKey& strKey) const {
    return !(CompareNoCase(ToCStr(), strKey.pStr->ToCStr()) == 0);
  }

  // Hash functor used for strings.
  struct HashFunctor {
    size_t operator()(const String& str) const {
      return String::BernsteinHashFunction(str.data(), str.size());
    }
  };

  // Case-insensitive hash functor used for strings. Supports additional
  // lookup based on NoCaseKey.
  struct NoCaseHashFunctor {
    size_t operator()(const String& str) const {
      return String::BernsteinHashFunctionCIS(str.data(), str.size());
    }
    size_t operator()(const NoCaseKey& key) const {
      return String::BernsteinHashFunctionCIS(key.pStr->c_str(), key.pStr->size());
    }
  };
};

//-----------------------------------------------------------------------------------
// ***** String Buffer used for Building Strings

class StringBuffer {
  char* pData;
  size_t Size;
  size_t BufferSize;
  size_t GrowSize;
  mutable bool LengthIsSize;

 public:
  // Constructors / Destructor.
  StringBuffer();
  explicit StringBuffer(size_t growSize);
  StringBuffer(const char* data);
  StringBuffer(const char* data, size_t buflen);
  StringBuffer(const String& src);
  StringBuffer(const StringBuffer& src);
  explicit StringBuffer(const wchar_t* data);
  ~StringBuffer();

  // Modify grow size used for growing/shrinking the buffer.
  size_t GetGrowSize() const {
    return GrowSize;
  }
  void SetGrowSize(size_t growSize);

  // *** General Functions
  // Does not release memory, just sets Size to 0
  void Clear();

  // For casting to a pointer to char.
  operator const char*() const {
    return (pData) ? pData : "";
  }
  // Pointer to raw buffer.
  const char* ToCStr() const {
    return (pData) ? pData : "";
  }

  // Returns number of bytes.
  size_t GetSize() const {
    return Size;
  }
  // Tells whether or not the string is empty.
  bool IsEmpty() const {
    return GetSize() == 0;
  }

  // Returns  number of characters
  size_t GetLength() const;

  // Returns  character at the specified index
  uint32_t GetCharAt(size_t index) const;
  uint32_t GetFirstCharAt(size_t index, const char** offset) const;
  uint32_t GetNextChar(const char** offset) const;
  uint32_t Front() const {
    return GetCharAt(0);
  }
  uint32_t Back() const {
    return GetCharAt(GetSize() - 1);
  }

  //  Resize the string to the new size
  void Resize(size_t _size);
  void Reserve(size_t _size);

  // Appends a character
  void AppendChar(uint32_t ch);

  // Append a string
  void AppendString(const wchar_t* pstr, size_t len = StringIsNullTerminated);
  void AppendString(const char* putf8str, size_t utf8StrSz = StringIsNullTerminated);
  void AppendFormatV(const char* format, va_list argList);
  void AppendFormat(const char* format, ...);

  // Assigned a string with dynamic data (copied through initializer).
  // void        AssignString(const InitStruct& src, size_t size);

  // Inserts substr at posAt
  void Insert(const char* substr, size_t posAt, size_t len = StringIsNullTerminated);
  // Inserts character at posAt
  size_t InsertCharAt(uint32_t c, size_t posAt);

  // Assignment
  void operator=(const char* str);
  void operator=(const wchar_t* str);
  void operator=(const String& src);
  void operator=(const StringBuffer& src);

  // Addition
  void operator+=(const String& src) {
    AppendString(src.ToCStr(), src.GetSize());
  }
  void operator+=(const char* psrc) {
    AppendString(psrc);
  }
  void operator+=(const wchar_t* psrc) {
    AppendString(psrc);
  }
  void operator+=(char ch) {
    AppendChar(ch);
  }
  // String   operator +  (const char* str) const ;
  // String   operator +  (const String& src)  const ;

  // Accesses raw bytes
  char& operator[](size_t index) {
    OVR_ASSERT(index < GetSize());
    return pData[index];
  }

  const char& operator[](size_t index) const {
    OVR_ASSERT(index < GetSize());
    return pData[index];
  }
};

// Returns a std::string that was initialized via printf-style formatting.
// The behavior is undefined if the specified format or arguments are invalid.
// Example usage:
//     std::string s = StringVsprintf("Hello %s", "world");
std::string StringVsprintf(const char* format, va_list args);

// Returns a std::string that was appended to via printf-style formatting.
// The behavior is undefined if the specified format or arguments are invalid.
// Example usage:
//     AppendSprintf(s, "appended %s", "hello world");
std::string& AppendSprintf(std::string& s, const char* format, ...);

// Convert a UTF8 String object to a wchar_t UCS (Unicode) std::basic_string object.
// The C++11 Standard Library has similar functionality, but it's not supported by earlier
// versions of Visual Studio. To consider: Add support for this when available.
// length is the strlen of pUTF8. If not specified then it is calculated automatically.
// Returns an empty string in the case that the UTF8 is malformed.
std::wstring UTF8StringToUCSString(const char* pUTF8, size_t length = StringIsNullTerminated);
std::wstring UTF8StringToUCSString(const std::string& sUTF8);
std::wstring OVRStringToUCSString(const String& sOVRUTF8);

// Convert a wchar_t UCS (Unicode) std::basic_string object to a UTF8 std::basic_string object.
// The C++11 Standard Library has similar functionality, but it's not supported by earlier
// versions of Visual Studio. To consider: Add support for this when available.
// length is the strlen of pUCS. If not specified then it is calculated automatically.
// Returns an empty string in the case that the UTF8 is malformed.
std::string UCSStringToUTF8String(const wchar_t* pUCS, size_t length = StringIsNullTerminated);
std::string UCSStringToUTF8String(const std::wstring& sUCS);
String UCSStringToOVRString(const wchar_t* pUCS, size_t length = StringIsNullTerminated);
String UCSStringToOVRString(const std::wstring& sUCS);

} // namespace OVR

#endif
