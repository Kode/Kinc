#include "pch.h"

#include "http.h"

#if !defined KORE_MACOS && !defined KORE_IOS && !defined KORE_WINDOWS

void kinc_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
                       kinc_http_callback_t callback, void *callbackdata) {}

#endif
