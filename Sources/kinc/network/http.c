#include "pch.h"

#include "Http.h"

#if !defined KORE_MACOS && !defined KORE_IOS && !defined KORE_WINDOWS

using namespace Kore;

void Kinc_HttpRequest(const char* url, const char* path, const char* data, int port, bool secure, int method, const char* header, Kinc_HttpCallback callback,
                       void* callbackdata) {}

#endif
