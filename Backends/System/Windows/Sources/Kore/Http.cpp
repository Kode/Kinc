#include "pch.h"

#include <Kore/Network/Http.h>
#include <Kore/Log.h>

#include <stdio.h>

using namespace Kore;

namespace {
	const wchar_t* convert(HttpMethod method) {
		switch (method) {
		case GET:
		default:
			return L"GET";
		case POST:
			return L"POST";
		case PUT:
			return L"PUT";
		case DELETE:
			return L"DELETE";
		}
	}
}

#include <Windows.h>
#include <winhttp.h>

void Kore::httpRequest(const char* url, const char* path, const char* data, int port, bool secure, HttpMethod method, const char* header, HttpCallback callback,
                       void* callbackdata) {
	// based on https://docs.microsoft.com/en-us/windows/desktop/winhttp/winhttp-sessions-overview

	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	BOOL bResults = FALSE;
	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

	hSession = WinHttpOpen(L"WinHTTP via Kore/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

	if (hSession) {
		wchar_t wurl[4096];
		MultiByteToWideChar(CP_UTF8, 0, url, -1, wurl, 4096);
		hConnect = WinHttpConnect(hSession, wurl, port, 0);
	}

	if (hConnect) {
		wchar_t wpath[4096];
		MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 4096);
		hRequest =
		    WinHttpOpenRequest(hConnect, convert(method), wpath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0);
	}

	if (hRequest) {
		wchar_t wheader[4096];
		if (header) {
			MultiByteToWideChar(CP_UTF8, 0, header, -1, wheader, 4096);
		}
		DWORD optionalLength = (data != 0 && strlen(data) > 0) ? (DWORD)strlen(data) + 1 : 0;
		bResults = WinHttpSendRequest(hRequest, header == 0 ? WINHTTP_NO_ADDITIONAL_HEADERS : wheader, header == 0 ? 0 : -1L, data == 0 ? WINHTTP_NO_REQUEST_DATA : (LPVOID)data, optionalLength, optionalLength, 0);
	}

	if (bResults) bResults = WinHttpReceiveResponse(hRequest, NULL);

	if (bResults) {
		do {
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) log(Error, "Error %d in WinHttpQueryDataAvailable.\n", GetLastError());

			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer) {
				log(Error, "Out of memory\n");
				dwSize = 0;
			}
			else {
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
					log(Error, "Error %d in WinHttpReadData.\n", GetLastError());
				else
					log(Info, "%s", pszOutBuffer);

				delete[] pszOutBuffer;
			}
		} while (dwSize > 0);
	}

	if (!bResults) log(Error, "Error %d has occurred.\n", GetLastError());

	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
}
