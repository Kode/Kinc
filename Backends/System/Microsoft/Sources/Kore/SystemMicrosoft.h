#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef long HRESULT;

void Kore_Microsoft_affirm(HRESULT result);
void Kore_Microsoft_affirmMessage(HRESULT result, const char* format, ...);
void Kore_Microsoft_format(const char* format, va_list args, wchar_t* buffer);

#ifdef __cplusplus
}
#endif
