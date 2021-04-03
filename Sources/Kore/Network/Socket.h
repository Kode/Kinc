#pragma once

#include <kinc/network/socket.h>

namespace Kore {
	class Socket {
	public:
		Socket();
		~Socket();
		void init();
		bool open(kinc_socket_protocol_t protocol, int port, bool blocking);

		unsigned urlToInt(const char* url, int port);
		void setBroadcastEnabled(bool enabled);
		bool listen(int backlog);
		bool accept(Socket* newSocket, unsigned* remoteAddress, unsigned* remotePort);
		bool connect(unsigned address, int port);
		void send(unsigned address, int port, const unsigned char* data, int size);
		void send(const char* url, int port, const unsigned char* data, int size);
		void send(const unsigned char* data, int size);
		int receive(unsigned char* data, int maxSize, unsigned& fromAddress, unsigned& fromPort);
		int receive(unsigned char* data, int maxSize);

	private:
		kinc_socket_t sock;
	};
}
