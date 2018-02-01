/************************************************************************************

Filename    :   OVR_UTF8Util.h
Content     :   UTF8 Unicode character encoding/decoding support
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

#ifndef OVR_UTF8Util_h
#define OVR_UTF8Util_h

#include "OVR_Types.h"

namespace OVR {
namespace UTF8Util {

// *** wchar_t / UTF8 Unicode string conversion

// Same behavior as strlcpy except it does an encoding conversion while doing the copy.
// length refers to wcslen(pSrcUCS).
// Returns the intended strlen of the destination.
//
// Example usage:
//     wchar_t* strW = L"abcdefgh";
//     char     buffer[4];
//     size_t   requiredStrlen = Strlcpy(buffer, OVR_ARRAY_COUNT(buffer), strW);
//
//     if(requiredStrlen >= OVR_ARRAY_COUNT(buffer)) // If not enough space...
//     {
//         char* pBuffer = new char[requiredStrlen + 1];
//         Strlcpy(pBuffer, OVR_ARRAY_COUNT(requiredStrlen + 1), strW);
//         ...
//     }
//
size_t Strlcpy(
    char* pDestUTF8,
    size_t destCharCount,
    const wchar_t* pSrcUCS,
    size_t sourceLength = (size_t)-1);

// Same behavior as strlcpy except it does an encoding conversion while doing the copy.
// length refers to strlen(pSrcUTF8).
// Returns the intended wcslen of the destination.
//
// Example usage:
//     char*   str8 = "abcdefgh";
//     wchar_t buffer[4];
//     size_t  requiredStrlen = Strlcpy(buffer, OVR_ARRAY_COUNT(buffer), str8);
//
//     if(requiredStrlen >= OVR_ARRAY_COUNT(buffer)) // If not enough space...
//     {
//         wchar_t* pBuffer = new wchar_t[requiredStrlen + 1];
//         Strlcpy(pBuffer, OVR_ARRAY_COUNT(requiredStrlen + 1), str8);
//         ...
//     }
//
size_t Strlcpy(
    wchar_t* pDestUCS,
    size_t destCharCount,
    const char* pSrcUTF8,
    size_t sourceLength = (size_t)-1);

// *** UTF8 string length and indexing.

// Determines the length of UTF8 string in characters.
// If source length is specified (in bytes), null 0 character is counted properly.
intptr_t GetLength(const char* putf8str, intptr_t length = -1);

// Gets a decoded UTF8 character at index; you can access up to the index returned
// by GetLength. 0 will be returned for out of bounds access.
uint32_t GetCharAt(intptr_t index, const char* putf8str, intptr_t length = -1);

// Converts UTF8 character index into byte offset.
// A valid offset is always returned, either to the start of the string if index < 0,
// or to the terminating null character/end of string if the index'th character is past byteLength.
// Characters which straddle byteLength are truncated.
intptr_t GetByteIndex(intptr_t index, const char* putf8str, intptr_t byteLength = -1);

// Converts UTF8 character index into byte offset.
// -1 is returned if index was out of bounds or if the final character straddles byteLength.
intptr_t GetByteIndexChecked(intptr_t index, const char* putf8str, intptr_t byteLength = -1);

// *** Individual character Encoding/Decoding.

// Determined the number of bytes necessary to encode a UCS character.
int GetEncodeCharSize(uint32_t ucsCharacter);

// Encodes the given UCS character into the given UTF-8 buffer.
// Writes the data starting at buffer[offset], and
// increments offset by the number of bytes written.
// May write up to 6 bytes, so make sure there's room in the buffer
void EncodeChar(char* pbuffer, intptr_t* poffset, uint32_t ucsCharacter);

// Return the next Unicode character in the UTF-8 encoded buffer.
// Invalid UTF-8 sequences produce a U+FFFD character as output.
// Advances *utf8_buffer past the character returned. Pointer advance
// occurs even if the terminating 0 character is hit, since that allows
// strings with middle '\0' characters to be supported.
uint32_t DecodeNextChar_Advance0(const char** putf8Buffer);

// Safer version of DecodeNextChar, which doesn't advance pointer if
// null character is hit.
inline uint32_t DecodeNextChar(const char** putf8Buffer) {
  uint32_t ch = DecodeNextChar_Advance0(putf8Buffer);
  if (ch == 0)
    (*putf8Buffer)--;
  return ch;
}
} // namespace UTF8Util
} // namespace OVR

#endif
