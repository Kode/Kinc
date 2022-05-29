#pragma once

#include <kinc/global.h>

#include <stdbool.h>

/*! \file http.h
    \brief Provides a simple http-API.
*/

#ifdef __cplusplus
extern "C" {
#endif

#define KINC_HTTP_GET 0
#define KINC_HTTP_POST 1
#define KINC_HTTP_PUT 2
#define KINC_HTTP_DELETE 3

typedef void (*kinc_http_callback_t)(int error, int response, const char *body, void *callbackdata);

/// <summary>
/// Fires off an http request.
/// </summary>
KINC_FUNC void kinc_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
                                 kinc_http_callback_t callback, void *callbackdata);

#ifdef __cplusplus
}
#endif
