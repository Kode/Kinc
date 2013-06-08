#include "pch.h"
#include "WinError.h"

#define S_OK    ((HRESULT)0L)

void Kore::affirm(HRESULT result) {
	affirm(result == S_OK);
}

void Kore::affirm(HRESULT result, const char* message) {
	affirm(result == S_OK, message);
}
