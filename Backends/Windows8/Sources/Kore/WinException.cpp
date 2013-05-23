#include "stdafx.h"
#include "WinException.h"
#include <Windows.h>

#undef assert

using namespace Kt;

HResultException::HResultException(long result) : result(result) {

}

const char* HResultException::what() {
	/*LPTSTR errorText = nullptr;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, result, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorText, 0, nullptr);
	String str(L"HRESULT failed: ");
	if (errorText != nullptr) {
		str += String(errorText);
		LocalFree(errorText);
		errorText = nullptr;
	}*/
	//_com_error error(result);
	//return error.ErrorMessage();
	return "Unknown error";
}

void Kt::affirm(long result) {
	if (FAILED(result)) throw HResultException(result);
}