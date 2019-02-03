#include "pch.h"

#include "Http.h"

#if !defined KORE_MACOS && !defined KORE_IOS && !defined KORE_WINDOWS

using namespace Kore;

void Kore::httpRequest(const char* url, const char* path, const char* data, int port, bool secure, HttpMethod method, const char* header, HttpCallback callback,
                       void* callbackdata) {}

#endif
