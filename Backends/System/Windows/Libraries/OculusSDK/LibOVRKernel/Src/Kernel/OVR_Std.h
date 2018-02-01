/************************************************************************************

Filename    :   OVR_Std.h
Content     :   Standard C function interface
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

#ifndef OVR_Std_h
#define OVR_Std_h

#include "OVR_Types.h"
#include <stdarg.h> // for va_list args
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(OVR_CC_MSVC) && (OVR_CC_MSVC >= 1400)
#define OVR_MSVC_SAFESTRING
#include <errno.h>
#endif

// Wide-char funcs
#include <wchar.h>
#include <wctype.h>

namespace OVR {

// Has the same behavior as itoa aside from also having a dest size argument.
// Return value: Pointer to the resulting null-terminated string, same as parameter str.
#if defined(OVR_OS_MS)
inline char* OVR_CDECL OVR_itoa(int val, char* dest, size_t destsize, int radix) {
#if defined(OVR_MSVC_SAFESTRING)
  _itoa_s(val, dest, destsize, radix);
  return dest;
#else
  OVR_UNUSED(destsize);
  return itoa(val, dest, radix);
#endif
}
#else // OVR_OS_MS
inline char* OVR_itoa(int val, char* dest, size_t len, int radix) {
  if (val == 0) {
    if (len > 1) {
      dest[0] = '0';
      dest[1] = '\0';
    } else if (len > 0)
      dest[0] = '\0';
    return dest;
  }

  // FIXME: Fix the following code to avoid memory write overruns when len is in sufficient.
  int cur = val;
  size_t i = 0;
  size_t sign = 0;

  if (val < 0) {
    val = -val;
    sign = 1;
  }

  while ((val != 0) && (i < (len - 1 - sign))) {
    cur = val % radix;
    val /= radix;

    if (radix == 16) {
      switch (cur) {
        case 10:
          dest[i] = 'a';
          break;
        case 11:
          dest[i] = 'b';
          break;
        case 12:
          dest[i] = 'c';
          break;
        case 13:
          dest[i] = 'd';
          break;
        case 14:
          dest[i] = 'e';
          break;
        case 15:
          dest[i] = 'f';
          break;
        default:
          dest[i] = (char)('0' + cur);
          break;
      }
    } else {
      dest[i] = (char)('0' + cur);
    }
    ++i;
  }

  if (sign) {
    dest[i++] = '-';
  }

  for (size_t j = 0; j < i / 2; ++j) {
    char tmp = dest[j];
    dest[j] = dest[i - 1 - j];
    dest[i - 1 - j] = tmp;
  }
  dest[i] = '\0';

  return dest;
}

#endif

// String functions

inline size_t OVR_CDECL OVR_strlen(const char* str) {
  return strlen(str);
}

inline size_t OVR_CDECL OVR_strlen(const wchar_t* str) {
  return wcslen(str);
}

inline char* OVR_CDECL OVR_strcpy(char* dest, size_t destsize, const char* src) {
#if defined(OVR_MSVC_SAFESTRING)
  // Using strncpy_s() instead of strcpy_s() since strcpy_s will invoke the
  // invalid parameter exception handler (now in google breakpad) and will
  // cause the server to crash, and even if it returns the data may not be
  // copied truncated.  The intent of all users surveyed has been to truncate
  // so we specify strncpy_s with the truncate option instead.
  strncpy_s(dest, destsize, src, _TRUNCATE);
  return dest;
#else
  // FIXME: This should be a safer implementation
  OVR_UNUSED(destsize);
  return strcpy(dest, src);
#endif
}

inline wchar_t* OVR_CDECL OVR_strcpy(wchar_t* dest, size_t destsize, const wchar_t* src) {
#if defined(OVR_MSVC_SAFESTRING)
  wcscpy_s(dest, destsize, src);
  return dest;
#else
  // FIXME: This should be a safer implementation
  OVR_UNUSED(destsize);
  return wcscpy(dest, src);
#endif
}

// Acts the same as the strlcpy function.
// Copies src to dest, 0-terminating even if it involves truncating the write.
// Returns the required strlen of dest (which is one less than the required size of dest).
// strlcpy is a safer alternative to strcpy and strncpy and provides size information.
// However, it still may result in an incomplete copy.
//
// Example usage:
//     char buffer[256];
//     if(OVR_strlcpy(buffer, "hello world", sizeof(buffer)) < sizeof(buffer))
//         { there was enough space }
//     else
//         { need a larger buffer }
//
size_t OVR_CDECL OVR_strlcpy(char* dest, const char* src, size_t destsize);
size_t OVR_CDECL OVR_strlcpy(wchar_t* dest, const wchar_t* src, size_t destsize);

// Acts the same as the strlcat function.
// Appends src to dest, 0-terminating even if it involves an incomplete write.
// Doesn't 0-terminate in the case that destsize is 0.
// Returns the required strlen of dest (which is one less than the required size of dest).
// The terminating 0 char of dest is overwritten by the first
// character of src, and a new 0 char is appended to dest. The required capacity
// of the destination is (strlen(src) + strlen(dest) + 1).
// strlcat is a safer alternative to strcat and provides size information.
// However, it still may result in an incomplete copy.
//
// Example usage:
//     char buffer[256] = "hello ";
//     if(OVR_strlcat(buffer, "world", sizeof(buffer)) < sizeof(buffer))
//         { there was enough space }
//     else
//         { need a larger buffer }
//
size_t OVR_CDECL OVR_strlcat(char* dest, const char* src, size_t destsize);
size_t OVR_CDECL OVR_strlcat(wchar_t* dest, const wchar_t* src, size_t destsize);

inline char* OVR_CDECL OVR_strncpy(char* dest, size_t destsize, const char* src, size_t count) {
#if defined(OVR_MSVC_SAFESTRING)
  strncpy_s(dest, destsize, src, count);
  return dest;
#else
  // FIXME: This should be a safer implementation
  OVR_UNUSED(destsize);
  return strncpy(dest, src, count);
#endif
}

inline char* OVR_CDECL OVR_strcat(char* dest, size_t destsize, const char* src) {
#if defined(OVR_MSVC_SAFESTRING)
  strcat_s(dest, destsize, src);
  return dest;
#else
  // FIXME: This should be a safer implementation
  OVR_UNUSED(destsize);
  return strcat(dest, src);
#endif
}

inline int OVR_CDECL OVR_strcmp(const char* dest, const char* src) {
  return strcmp(dest, src);
}

inline const char* OVR_CDECL OVR_strchr(const char* str, char c) {
  return strchr(str, c);
}

inline char* OVR_CDECL OVR_strchr(char* str, char c) {
  return strchr(str, c);
}

const char* OVR_CDECL OVR_strrchr(const char* pString, int c);
inline char* OVR_CDECL OVR_strrchr(char* pString, int c) {
  return (char*)OVR_strrchr((const char*)pString, c);
}

char* OVR_CDECL OVR_stristr(const char* s1, const char* s2);

inline const uint8_t* OVR_CDECL OVR_memrchr(const uint8_t* str, size_t size, uint8_t c) {
  for (intptr_t i = (intptr_t)size - 1; i >= 0; i--) {
    if (str[i] == c)
      return str + i;
  }
  return 0;
}

double OVR_CDECL OVR_strtod(const char* string, char** tailptr);

inline long OVR_CDECL OVR_strtol(const char* string, char** tailptr, int radix) {
  return strtol(string, tailptr, radix);
}

inline unsigned long OVR_CDECL OVR_strtoul(const char* string, char** tailptr, int radix) {
  return strtoul(string, tailptr, radix);
}

inline int OVR_CDECL OVR_strncmp(const char* ws1, const char* ws2, size_t size) {
  return strncmp(ws1, ws2, size);
}

inline uint64_t OVR_CDECL OVR_strtouq(const char* nptr, char** endptr, int base) {
#if defined(OVR_CC_MSVC)
  return _strtoui64(nptr, endptr, base);
#else
  return strtoull(nptr, endptr, base);
#endif
}

inline int64_t OVR_CDECL OVR_strtoq(const char* nptr, char** endptr, int base) {
#if defined(OVR_CC_MSVC)
  return _strtoi64(nptr, endptr, base);
#else
  return strtoll(nptr, endptr, base);
#endif
}

inline int64_t OVR_CDECL OVR_atoq(const char* string) {
#if defined(OVR_CC_MSVC)
  return _atoi64(string);
#else
  return atoll(string);
#endif
}

inline uint64_t OVR_CDECL OVR_atouq(const char* string) {
  return OVR_strtouq(string, NULL, 10);
}

// Implemented in OVR_Std.cpp in platform-specific manner.
int OVR_CDECL OVR_stricmp(const char* dest, const char* src);
int OVR_CDECL OVR_strnicmp(const char* dest, const char* src, size_t count);

// This is like vsprintf but with a destination buffer size argument. However, the behavior is
// different
// from vsnprintf in that the return value semantics are like vsprintf (which returns -1 on capacity
// overflow) and
// not like vsnprintf (which returns intended strlen on capacity overflow).
// Return value:
//    On success, the total number of characters written is returned.
//    On failure, a negative number is returned.
inline size_t OVR_CDECL
OVR_vsprintf(char* dest, size_t destsize, const char* format, va_list argList) {
  size_t ret;
#if defined(OVR_CC_MSVC)
#if defined(OVR_MSVC_SAFESTRING)
  dest[0] = '\0';
  int rv = vsnprintf_s(dest, destsize, _TRUNCATE, format, argList);
  if (rv == -1) {
    dest[destsize - 1] = '\0';
    ret = destsize - 1;
  } else
    ret = (size_t)rv;
#else
  OVR_UNUSED(destsize);
  int rv = _vsnprintf(dest, destsize - 1, format, argList);
  OVR_ASSERT(rv != -1);
  ret = (size_t)rv;
  dest[destsize - 1] = 0;
#endif
#else
  // FIXME: This should be a safer implementation
  OVR_UNUSED(destsize);
  ret = (size_t)vsprintf(dest, format, argList);
  OVR_ASSERT(ret < destsize);
#endif
  return ret;
}

// Returns the strlen of the resulting formatted string, or a negative value if the format is
// invalid.
// Note: If you are planning on printing a string then it's more efficient to just use vsnprintf and
// look at the return value and handle the uncommon case that there wasn't enough space.
inline int OVR_CDECL OVR_vscprintf(const char* format, va_list argList) {
  int ret;
#if defined(OVR_CC_MSVC)
  ret = _vscprintf(format, argList);
#else
  ret = vsnprintf(NULL, 0, format, argList);
#endif
  return ret;
}

wchar_t* OVR_CDECL OVR_wcscpy(wchar_t* dest, size_t destsize, const wchar_t* src);
wchar_t* OVR_CDECL OVR_wcsncpy(wchar_t* dest, size_t destsize, const wchar_t* src, size_t count);
wchar_t* OVR_CDECL OVR_wcscat(wchar_t* dest, size_t destsize, const wchar_t* src);
size_t OVR_CDECL OVR_wcslen(const wchar_t* str);
int OVR_CDECL OVR_wcscmp(const wchar_t* a, const wchar_t* b);
int OVR_CDECL OVR_wcsicmp(const wchar_t* a, const wchar_t* b);

inline int OVR_CDECL OVR_wcsicoll(const wchar_t* a, const wchar_t* b) {
#if defined(OVR_OS_MS)
#if defined(OVR_CC_MSVC) && (OVR_CC_MSVC >= 1400)
  return ::_wcsicoll(a, b);
#else
  return ::wcsicoll(a, b);
#endif
#else
  // not supported, use regular wcsicmp
  return OVR_wcsicmp(a, b);
#endif
}

inline int OVR_CDECL OVR_wcscoll(const wchar_t* a, const wchar_t* b) {
#if defined(OVR_OS_MS) || defined(OVR_OS_LINUX)
  return wcscoll(a, b);
#else
  // not supported, use regular wcscmp
  return OVR_wcscmp(a, b);
#endif
}

#ifndef OVR_NO_WCTYPE

inline int OVR_CDECL UnicodeCharIs(const uint16_t* table, wchar_t charCode) {
  unsigned offset = table[charCode >> 8];
  if (offset == 0)
    return 0;
  if (offset == 1)
    return 1;
  return (table[offset + ((charCode >> 4) & 15)] & (1 << (charCode & 15))) != 0;
}

extern const uint16_t UnicodeAlnumBits[];
extern const uint16_t UnicodeAlphaBits[];
extern const uint16_t UnicodeDigitBits[];
extern const uint16_t UnicodeSpaceBits[];
extern const uint16_t UnicodeXDigitBits[];

// Uncomment if necessary
// extern const uint16_t UnicodeCntrlBits[];
// extern const uint16_t UnicodeGraphBits[];
// extern const uint16_t UnicodeLowerBits[];
// extern const uint16_t UnicodePrintBits[];
// extern const uint16_t UnicodePunctBits[];
// extern const uint16_t UnicodeUpperBits[];

inline int OVR_CDECL OVR_iswalnum(wchar_t charCode) {
  return UnicodeCharIs(UnicodeAlnumBits, charCode);
}
inline int OVR_CDECL OVR_iswalpha(wchar_t charCode) {
  return UnicodeCharIs(UnicodeAlphaBits, charCode);
}
inline int OVR_CDECL OVR_iswdigit(wchar_t charCode) {
  return UnicodeCharIs(UnicodeDigitBits, charCode);
}
inline int OVR_CDECL OVR_iswspace(wchar_t charCode) {
  return UnicodeCharIs(UnicodeSpaceBits, charCode);
}
inline int OVR_CDECL OVR_iswxdigit(wchar_t charCode) {
  return UnicodeCharIs(UnicodeXDigitBits, charCode);
}

// Uncomment if necessary
// inline int OVR_CDECL OVR_iswcntrl (wchar_t charCode) { return UnicodeCharIs(UnicodeCntrlBits,
// charCode); }
// inline int OVR_CDECL OVR_iswgraph (wchar_t charCode) { return UnicodeCharIs(UnicodeGraphBits,
// charCode); }
// inline int OVR_CDECL OVR_iswlower (wchar_t charCode) { return UnicodeCharIs(UnicodeLowerBits,
// charCode); }
// inline int OVR_CDECL OVR_iswprint (wchar_t charCode) { return UnicodeCharIs(UnicodePrintBits,
// charCode); }
// inline int OVR_CDECL OVR_iswpunct (wchar_t charCode) { return UnicodeCharIs(UnicodePunctBits,
// charCode); }
// inline int OVR_CDECL OVR_iswupper (wchar_t charCode) { return UnicodeCharIs(UnicodeUpperBits,
// charCode); }

int OVR_CDECL OVR_towupper(wchar_t charCode);
int OVR_CDECL OVR_towlower(wchar_t charCode);

#else // OVR_NO_WCTYPE

inline int OVR_CDECL OVR_iswspace(wchar_t c) {
  return iswspace(c);
}

inline int OVR_CDECL OVR_iswdigit(wchar_t c) {
  return iswdigit(c);
}

inline int OVR_CDECL OVR_iswxdigit(wchar_t c) {
  return iswxdigit(c);
}

inline int OVR_CDECL OVR_iswalpha(wchar_t c) {
  return iswalpha(c);
}

inline int OVR_CDECL OVR_iswalnum(wchar_t c) {
  return iswalnum(c);
}

inline wchar_t OVR_CDECL OVR_towlower(wchar_t c) {
  return (wchar_t)towlower(c);
}

inline wchar_t OVR_towupper(wchar_t c) {
  return (wchar_t)towupper(c);
}

#endif // OVR_NO_WCTYPE

// ASCII versions of tolower and toupper. Don't use "char"
inline int OVR_CDECL OVR_tolower(int c) {
  return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

inline int OVR_CDECL OVR_toupper(int c) {
  return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}

inline double OVR_CDECL OVR_wcstod(const wchar_t* string, wchar_t** tailptr) {
#if defined(OVR_OS_OTHER)
  OVR_UNUSED(tailptr);
  char buffer[64];
  char* tp = NULL;
  size_t max = OVR_wcslen(string);
  if (max > 63)
    max = 63;
  unsigned char c = 0;
  for (size_t i = 0; i < max; i++) {
    c = (unsigned char)string[i];
    buffer[i] = ((c) < 128 ? (char)c : '!');
  }
  buffer[max] = 0;
  return OVR_strtod(buffer, &tp);
#else
  return wcstod(string, tailptr);
#endif
}

inline long OVR_CDECL OVR_wcstol(const wchar_t* string, wchar_t** tailptr, int radix) {
#if defined(OVR_OS_OTHER)
  OVR_UNUSED(tailptr);
  char buffer[64];
  char* tp = NULL;
  size_t max = OVR_wcslen(string);
  if (max > 63)
    max = 63;
  unsigned char c = 0;
  for (size_t i = 0; i < max; i++) {
    c = (unsigned char)string[i];
    buffer[i] = ((c) < 128 ? (char)c : '!');
  }
  buffer[max] = 0;
  return strtol(buffer, &tp, radix);
#else
  return wcstol(string, tailptr, radix);
#endif
}

} // namespace OVR

#endif // OVR_Std_h
