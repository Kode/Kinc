#include "pch.h"

#include "Http.h"

#include <Kinc/Network/Http.h>

using namespace Kore;

void Kore::httpRequest(const char* url, const char* path, const char* data, int port, bool secure, HttpMethod method, const char* header, HttpCallback callback,
                       void* callbackdata) {
	Kinc_HttpRequest(url, path, data, port, secure, (int)method, header, callback, callbackdata);
}
