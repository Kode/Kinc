#include "pch.h"

#include <Kore/Network/Http.h>

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

	wchar_t wurl[4096];
	MultiByteToWideChar(CP_UTF8, 0, url, -1, wurl, 4096);

	if (hSession) hConnect = WinHttpConnect(hSession, wurl, INTERNET_DEFAULT_HTTPS_PORT, 0);

	if (hConnect) hRequest = WinHttpOpenRequest(hConnect, convert(method), NULL, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

	if (hRequest) bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

	if (bResults) bResults = WinHttpReceiveResponse(hRequest, NULL);

	if (bResults) {
		do {
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) printf("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());

			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer) {
				printf("Out of memory\n");
				dwSize = 0;
			}
			else {
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
					printf("Error %u in WinHttpReadData.\n", GetLastError());
				else
					printf("%s", pszOutBuffer);

				delete[] pszOutBuffer;
			}
		} while (dwSize > 0);
	}

	if (!bResults) printf("Error %d has occurred.\n", GetLastError());

	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
}
