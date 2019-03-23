#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef long HRESULT;

void Kinc_Microsoft_Affirm(HRESULT result);
void Kinc_Microsoft_AffirmMessage(HRESULT result, const char *format, ...);
void Kinc_Microsoft_Format(const char *format, va_list args, wchar_t *buffer);

#ifdef __cplusplus
}
#endif
