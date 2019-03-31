#pragma once

#ifdef KORE_WINDOWS
#if defined(_WIN64)
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;
#else
#if !defined _W64
#define _W64
#endif
typedef _W64 unsigned int UINT_PTR, *PUINT_PTR;
#endif
typedef UINT_PTR SOCKET;
#endif

namespace Kore {
	class Socket {
	public:
		Socket();
		~Socket();
		void init();
		void open(int port);

		unsigned urlToInt(const char* url, int port);
		void setBroadcastEnabled(bool enabled);
		void send(unsigned address, int port, const unsigned char* data, int size);
		void send(const char* url, int port, const unsigned char* data, int size);
		int receive(unsigned char* data, int maxSize, unsigned& fromAddress, unsigned& fromPort);

	private:
#ifdef KORE_WINDOWS
		SOCKET handle;
#else
		int handle;
#endif
	};
}
