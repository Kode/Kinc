#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KINC_HTTP_GET    0 
#define KINC_HTTP_POST   1
#define KINC_HTTP_PUT    2
#define KINC_HTTP_DELETE 3

typedef void (*Kinc_HttpCallback)(int error, int response, const char* body, void* callbackdata);

void Kinc_HttpRequest(const char* url, const char* path, const char* data, int port, bool secure, int method,
	                 const char* header, Kinc_HttpCallback callback, void* callbackdata);

#ifdef __cplusplus
}
#endif
