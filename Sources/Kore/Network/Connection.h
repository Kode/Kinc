#pragma once

#include <Kore/Network/Socket.h>

namespace Kore {

	class Connection {
	public:
		enum State {
			Disconnected = 0,
			Connected = 1,
		};

		State state;

		Connection(const char* url, int sendPort, int receivePort, double timeout = 10, int buffSize = 256);
		~Connection();
		void send(const u8* data, int size, bool reliable = true);
		int receive(u8* data);
	private:
		enum PaketType {
			Control = 1,
			Reliable = 2,
			Unreliable = 3
		};

		static const u32 magicID = 1346655563;
		static const int headerSize = 8;

		const int sndPort;
		const int recPort;
		const char* url;
		Kore::Socket socket;

		int buffSize;
		u32 lastSndNr;
		u8* recBuff;
		u8* sndBuff;

		double timeout;
		double lastTime;

		void send(const u8* data, int size, PaketType type);
	};
}
