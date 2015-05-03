#pragma once

namespace Kore {
	enum HttpMethod {
		GET,
		POST,
		PUT,
		DELETE
	};
	
	typedef void(*HttpCallback)(int error, int response, const char* body);
	
	void httpRequest(const char* url, const char* path, const char* data, int port = 80, bool secure = false, HttpMethod method = GET, HttpCallback callback = 0);
}
