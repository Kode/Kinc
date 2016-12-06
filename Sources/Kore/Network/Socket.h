#pragma once

namespace Kore {
	class Socket {
	public:
		Socket();
		~Socket();
		void init();
		void open(int port);

		unsigned urlToInt(const char* url, int port);
		void send(unsigned address, int port, const unsigned char* data, int size);
		void send(const char* url, int port, const unsigned char* data, int size);
		int receive(unsigned char* data, int maxSize, unsigned& fromAddress, unsigned& fromPort);

	private:
		int handle;
	};
}
