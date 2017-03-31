#include "pch.h"

#include "Http.h"

#if !defined KORE_MACOS && !defined KORE_IOS

using namespace Kore;

void Kore::httpRequest(const char* url, const char* path, const char* data, int port, bool secure, HttpMethod method, HttpCallback callback,
                       void* callbackdata) {}

#endif
