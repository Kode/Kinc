#pragma once

namespace Kore {
	class Socket {
	public:
		Socket();
		~Socket();
		void open(int port);
		void send(unsigned addr1, unsigned addr2, unsigned addr3, unsigned addr4, unsigned short port, const unsigned char* data, int size);
		int receive(unsigned char* data, int maxSize, unsigned& fromAddress, unsigned& fromPort);
	private:
		int handle;
	};
}
