#pragma once

#include <Kore/Network/Socket.h>

namespace Kore {

	class Connection {
	public:
		Connection(const char* url, int sendPort, int receivePort, int buffSize = 256);
		~Connection();
		void send(const u8* data, int size, bool reliable = true);
		int receive(u8* data);
	private:
		static const u32 magicID = 19285;
		static const int headerSize = 8;

		const int sndPort;
		const int recPort;
		const char* url;
		Kore::Socket socket;

		int buffSize;
		u32 lastSndNr;
		u8* recBuff;
		u8* sndBuff;
	};
}
