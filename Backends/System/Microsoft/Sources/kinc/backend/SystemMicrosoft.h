#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef long HRESULT;

void kinc_microsoft_affirm(HRESULT result);
void kinc_microsoft_affirm_message(HRESULT result, const char *format, ...);
void kinc_microsoft_format(const char *format, va_list args, wchar_t *buffer);
void kinc_microsoft_convert_string(wchar_t *destination, const char *source, int num);

#ifdef __cplusplus
}
#endif
