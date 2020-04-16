#pragma once

#include <kinc/network/socket.h>

namespace Kore {
	class Socket {
	public:
		Socket();
		~Socket();
		void init();
		bool open(int port);

		unsigned urlToInt(const char* url, int port);
		void setBroadcastEnabled(bool enabled);
		void send(unsigned address, int port, const unsigned char* data, int size);
		void send(const char* url, int port, const unsigned char* data, int size);
		int receive(unsigned char* data, int maxSize, unsigned& fromAddress, unsigned& fromPort);
	private:
		kinc_socket_t sock;
	};
}
