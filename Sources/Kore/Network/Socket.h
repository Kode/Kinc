#pragma once

namespace Kore {
	class Socket {
	public:
		Socket();
		~Socket();
		void open(int port);
		void send(const char* url, int port, const unsigned char* data, int size);
		int receive(unsigned char* data, int maxSize, unsigned& fromAddress, unsigned& fromPort);
	private:
		int handle;
	};
}
