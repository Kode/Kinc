#pragma once

#include <Kore/Network/Socket.h>

namespace Kore {

	class Connection {
	public:
		Connection(const char* url, int sendPort, int receivePort);
		~Connection();
		void send(const unsigned char* data, int size, bool reliable = true);
		int receive(unsigned char* data);
	private:
		static const unsigned int magicID = 19285;
		static const int headerSize = 4;
		static const int sndBuffSize = 128;
		static const int recBuffSize = 256;

		const int sndPort;
		const int recPort;
		const char* url;
		Kore::Socket* socket;

		unsigned char recBuff[recBuffSize];
		unsigned int lastSndNr;
		unsigned char sndBuff[sndBuffSize];
	};
}
