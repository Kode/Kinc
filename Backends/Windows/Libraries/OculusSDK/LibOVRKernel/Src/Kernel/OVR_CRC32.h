/************************************************************************************

PublicHeader:   OVR
Filename    :   OVR_CRC32.h
Content     :   CRC-32
Created     :   June 20, 2014
Author      :   Chris Taylor

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

#ifndef OVR_CRC32_h
#define OVR_CRC32_h

#include "OVR_Types.h"
#include "OVR_Compiler.h"

namespace OVR {


//-----------------------------------------------------------------------------------
// ***** Oculus Camera CRC-32

// This is a proprietary CRC we have been using for camera EEPROM data.
uint32_t OculusCamera_CRC32(const void* data, int bytes, uint32_t prevCRC = 0);


//-----------------------------------------------------------------------------------
// ***** CRC-32 Internal Details

// Choose one of these tables for calculating the CRC:

// polynomial 0x04C11DB7 - PKZIP, PNG [older]
extern const uint32_t CRC32_Table_CRC32[256];

// polynomial 0x1EDC6F41 - CRC32-C (Castagnoli): SSE4.2 [newer]
extern const uint32_t CRC32_Table_CRC32_C[256];

// CRC32 core algorithm
uint32_t CalculateCRC32(const uint32_t* OVR_RESTRICT table, const void* OVR_RESTRICT data, int bytes, uint32_t crc);


//-----------------------------------------------------------------------------------
// ***** CRC-32 Standards

// This is the version you probably want to call.  It's the same one used in PKZIP.
inline uint32_t Standard_CRC32(const void* data, int bytes)
{
    return CalculateCRC32(CRC32_Table_CRC32, data, bytes, 0);
}

// This is a version that can be hardware accelerated with SSE4.2 and may be suitable
// for large data in the future.
inline uint32_t Castagnoli_CRC32(const void* data, int bytes)
{
    return CalculateCRC32(CRC32_Table_CRC32_C, data, bytes, 0);
}


} // namespace OVR

#endif
