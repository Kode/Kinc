/**************************************************************************

Filename    :   OVR_UTF8Util.cpp
Content     :   UTF8 Unicode character encoding/decoding support
Created     :   September 19, 2012
Notes       :
Notes       :   Much useful info at "UTF-8 and Unicode FAQ"
                http://www.cl.cam.ac.uk/~mgk25/unicode.html

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

#include "OVR_UTF8Util.h"
#include <wchar.h>
#include <string.h>

// sizeof(wchar_t) in preprocessor-accessible form.
#ifndef OVR_WCHAR_SIZE
#if defined(__WCHAR_MAX__)
#if (__WCHAR_MAX__ == 127) || (__WCHAR_MAX__ == 255)
#define OVR_WCHAR_SIZE 1
#elif (__WCHAR_MAX__ == 32767) || (__WCHAR_MAX__ == 65535)
#define OVR_WCHAR_SIZE 2
#else
#define OVR_WCHAR_SIZE 4
#endif
#elif defined(OVR_OS_UNIX)
#define OVR_WCHAR_SIZE 4
#else
#define OVR_WCHAR_SIZE 2
#endif
#endif

namespace OVR {
namespace UTF8Util {

size_t Strlcpy(char* pDestUTF8, size_t destCharCount, const wchar_t* pSrcUCS, size_t sourceLength) {
  if (sourceLength == (size_t)-1)
    sourceLength = wcslen(pSrcUCS);

  size_t destLength = 0;

  size_t i;
  for (i = 0; i < sourceLength; ++i) {
    char buff[6]; // longest utf8 encoding just to be safe
    intptr_t count = 0;

    EncodeChar(buff, &count, pSrcUCS[i]);

    // If this character occupies more than the remaining space (leaving room for the trailing
    // '\0'), truncate here
    if ((destLength + count) >= destCharCount)
      break;

    memcpy(pDestUTF8 + destLength, buff, count);
    destLength += (size_t)count;
  }

  // Should be true for all cases other than destCharCount == 0.
  if (destLength < destCharCount)
    pDestUTF8[destLength] = '\0';

  // Compute the required destination size for any remaining source characters
  // Note a very long wchar string with lots of multibyte encodings can overflow destLength
  // This should be okay since we'll just treat those cases (likely to be originating
  // from an attacker) as shorter strings
  while (i < sourceLength)
    destLength += GetEncodeCharSize(pSrcUCS[i++]);

  // Return the intended strlen of pDestUTF8.
  return destLength;
}

size_t Strlcpy(wchar_t* pDestUCS, size_t destCharCount, const char* pSrcUTF8, size_t sourceLength) {
  if (sourceLength == (size_t)-1)
    sourceLength = strlen(pSrcUTF8);

  size_t destLength = 0, requiredLength = 0;

  for (const char* pSrcUTF8End = (pSrcUTF8 + sourceLength); pSrcUTF8 < pSrcUTF8End;) {
    uint32_t c = DecodeNextChar_Advance0(&pSrcUTF8);
    OVR_ASSERT_M(
        pSrcUTF8 <= (pSrcUTF8 + sourceLength), "Strlcpy sourceLength was not on a UTF8 boundary.");

#if (OVR_WCHAR_SIZE == 2)
    if (c >= 0x0000FFFF)
      c = 0x0000FFFD;
#endif

    if ((destLength + 1) < destCharCount) // If there is enough space to append a wchar_t (leaving
    // room for a trailing '\0')...
    {
      pDestUCS[destLength] = wchar_t(c);
      destLength++;
    }

    requiredLength++;
  }

  if (destLength < destCharCount)
    pDestUCS[destLength] = L'\0';

  return requiredLength; // Return the intended wcslen of pDestUCS.
}

intptr_t GetLength(const char* buf, intptr_t buflen) {
  const char* p = buf;
  intptr_t length = 0;

  if (buflen != -1) {
    while (p - buf < buflen) {
      // We should be able to have ASStrings with 0 in the middle.
      UTF8Util::DecodeNextChar_Advance0(&p);
      length++;
    }
  } else {
    while (UTF8Util::DecodeNextChar_Advance0(&p))
      length++;
  }

  return length;
}

uint32_t GetCharAt(intptr_t index, const char* putf8str, intptr_t length) {
  const char* buf = putf8str;
  uint32_t c = 0;

  if (length != -1) {
    while (buf - putf8str < length) {
      c = UTF8Util::DecodeNextChar_Advance0(&buf);
      if (index == 0)
        return c;
      index--;
    }

    return c;
  }

  do {
    c = UTF8Util::DecodeNextChar_Advance0(&buf);
    index--;

    if (c == 0) {
      // We've hit the end of the string; don't go further.
      OVR_ASSERT(index == 0);
      return c;
    }
  } while (index >= 0);

  return c;
}

intptr_t GetByteIndex(intptr_t index, const char* putf8str, intptr_t byteLength) {
  const char* buf = putf8str;

  if (byteLength >= 0) {
    const char* lastValid = putf8str;
    while ((buf - putf8str) < byteLength && index > 0) {
      lastValid = buf;
      // XXX this may read up to 5 bytes past byteLength
      UTF8Util::DecodeNextChar_Advance0(&buf);
      index--;
    }

    // check for UTF8 characters which ran past the end of the buffer
    if (buf - putf8str > byteLength)
      return lastValid - putf8str;

    return buf - putf8str;
  }

  while (index > 0) {
    uint32_t c = UTF8Util::DecodeNextChar(&buf);
    index--;

    // okay to index past null-terminator, DecodeNextChar will not advance past it
    if (c == 0)
      break;
  }

  return buf - putf8str;
}

intptr_t GetByteIndexChecked(intptr_t index, const char* putf8str, intptr_t byteLength) {
  const char* buf = putf8str;

  // before start of string?
  if (index < 0)
    return -1;

  if (byteLength >= 0) {
    while ((buf - putf8str) < byteLength && index > 0) {
      // XXX this may read up to 5 bytes past byteLength
      UTF8Util::DecodeNextChar_Advance0(&buf);
      index--;
    }

    // check for UTF8 characters which ran past the end of the buffer
    if ((buf - putf8str) > byteLength)
      return -1;
  } else {
    while (index > 0) {
      uint32_t c = UTF8Util::DecodeNextChar_Advance0(&buf);
      index--;

      // ran past null terminator
      if (c == 0)
        return -1;
    }
  }

  // ran off end of string
  if (index != 0)
    return -1;

  // at the valid index'th character-boundary offset in the string
  return buf - putf8str;
}

int GetEncodeCharSize(uint32_t ucs_character) {
  if (ucs_character <= 0x7F)
    return 1;
  else if (ucs_character <= 0x7FF)
    return 2;
  else if (ucs_character <= 0xFFFF)
    return 3;
  else if (ucs_character <= 0x1FFFFF)
    return 4;
  else if (ucs_character <= 0x3FFFFFF)
    return 5;
  else if (ucs_character <= 0x7FFFFFFF)
    return 6;
  else
    return 0;
}

uint32_t DecodeNextChar_Advance0(const char** putf8Buffer) {
  uint32_t uc;
  char c;

// Security considerations:
//
// Changed, this is now only the case for DecodeNextChar:
//  - If we hit a zero byte, we want to return 0 without stepping
//    the buffer pointer past the 0. th
//
// If we hit an "overlong sequence"; i.e. a character encoded
// in a longer multibyte string than is necessary, then we
// need to discard the character.  This is so attackers can't
// disguise dangerous characters or character sequences --
// there is only one valid encoding for each character.
//
// If we decode characters { 0xD800 .. 0xDFFF } or { 0xFFFE,
// 0xFFFF } then we ignore them; they are not valid in UTF-8.

// This isn't actually an invalid character; it's a valid char that
// looks like an inverted question mark.
#define INVALID_CHAR 0x0FFFD

#define FIRST_BYTE(mask, shift) uc = (c & (mask)) << (shift);

#define NEXT_BYTE(shift)                          \
  c = **putf8Buffer;                              \
  if (c == 0)                                     \
    return 0; /* end of buffer, do not advance */ \
  if ((c & 0xC0) != 0x80)                         \
    return INVALID_CHAR; /* standard check */     \
  (*putf8Buffer)++;                               \
  uc |= (c & 0x3F) << shift;

  c = **putf8Buffer;
  (*putf8Buffer)++;
  if (c == 0)
    return 0; // End of buffer.

  if ((c & 0x80) == 0)
    return (uint32_t)c; // Conventional 7-bit ASCII.

  // Multi-byte sequences.
  if ((c & 0xE0) == 0xC0) {
    // Two-byte sequence.
    FIRST_BYTE(0x1F, 6);
    NEXT_BYTE(0);
    if (uc < 0x80)
      return INVALID_CHAR; // overlong
    return uc;
  } else if ((c & 0xF0) == 0xE0) {
    // Three-byte sequence.
    FIRST_BYTE(0x0F, 12);
    NEXT_BYTE(6);
    NEXT_BYTE(0);
    if (uc < 0x800)
      return INVALID_CHAR; // overlong
    // Not valid ISO 10646, but Flash requires these to work
    // see AS3 test e15_5_3_2_3 for String.fromCharCode().charCodeAt(0)
    // if (uc >= 0x0D800 && uc <= 0x0DFFF) return INVALID_CHAR;
    // if (uc == 0x0FFFE || uc == 0x0FFFF) return INVALID_CHAR; // not valid ISO 10646
    return uc;
  } else if ((c & 0xF8) == 0xF0) {
    // Four-byte sequence.
    FIRST_BYTE(0x07, 18);
    NEXT_BYTE(12);
    NEXT_BYTE(6);
    NEXT_BYTE(0);
    if (uc < 0x010000)
      return INVALID_CHAR; // overlong
    return uc;
  } else if ((c & 0xFC) == 0xF8) {
    // Five-byte sequence.
    FIRST_BYTE(0x03, 24);
    NEXT_BYTE(18);
    NEXT_BYTE(12);
    NEXT_BYTE(6);
    NEXT_BYTE(0);
    if (uc < 0x0200000)
      return INVALID_CHAR; // overlong
    return uc;
  } else if ((c & 0xFE) == 0xFC) {
    // Six-byte sequence.
    FIRST_BYTE(0x01, 30);
    NEXT_BYTE(24);
    NEXT_BYTE(18);
    NEXT_BYTE(12);
    NEXT_BYTE(6);
    NEXT_BYTE(0);
    if (uc < 0x04000000)
      return INVALID_CHAR; // overlong
    return uc;
  } else {
    // Invalid.
    return INVALID_CHAR;
  }
}

void EncodeChar(char* pbuffer, intptr_t* pindex, uint32_t ucs_character) {
  if (ucs_character <= 0x7F) {
    // Plain single-byte ASCII.
    pbuffer[(*pindex)++] = (char)ucs_character;
  } else if (ucs_character <= 0x7FF) {
    // Two bytes.
    pbuffer[(*pindex)++] = 0xC0 | (char)(ucs_character >> 6);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 0) & 0x3F);
  } else if (ucs_character <= 0xFFFF) {
    // Three bytes.
    pbuffer[(*pindex)++] = 0xE0 | (char)(ucs_character >> 12);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 6) & 0x3F);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 0) & 0x3F);
  } else if (ucs_character <= 0x1FFFFF) {
    // Four bytes.
    pbuffer[(*pindex)++] = 0xF0 | (char)(ucs_character >> 18);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 12) & 0x3F);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 6) & 0x3F);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 0) & 0x3F);
  } else if (ucs_character <= 0x3FFFFFF) {
    // Five bytes.
    pbuffer[(*pindex)++] = 0xF8 | (char)(ucs_character >> 24);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 18) & 0x3F);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 12) & 0x3F);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 6) & 0x3F);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 0) & 0x3F);
  } else if (ucs_character <= 0x7FFFFFFF) {
    // Six bytes.
    pbuffer[(*pindex)++] = 0xFC | (char)(ucs_character >> 30);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 24) & 0x3F);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 18) & 0x3F);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 12) & 0x3F);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 6) & 0x3F);
    pbuffer[(*pindex)++] = 0x80 | (char)((ucs_character >> 0) & 0x3F);
  } else {
    // Invalid char; don't encode anything.
  }
}
} // namespace UTF8Util
} // namespace OVR
