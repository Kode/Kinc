#include "Http.h"

#include <kinc/network/http.h>

using namespace Kore;

void Kore::httpRequest(const char *url, const char *path, const char *data, int port, bool secure, HttpMethod method, const char *header, HttpCallback callback,
                       void *callbackdata) {
	kinc_http_request(url, path, data, port, secure, (int)method, header, callback, callbackdata);
}
