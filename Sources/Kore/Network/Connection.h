#pragma once

#include <Kore/Network/Socket.h>

namespace Kore {
	const int connBuffSize = 256;

	class Connection {
	public:
		Connection(const char* url, int sendPort, int receivePort);
		~Connection();
		void send(const unsigned char* data, int size);
		int receive(unsigned char* data);
	private:
		const int sendPort;
		const int receivePort;
		const char* url;
		Kore::Socket* socket;

		int bufferUsage;
		unsigned char buffer[connBuffSize];
	};
}
