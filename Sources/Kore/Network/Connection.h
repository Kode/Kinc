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
		double ping;

		Connection(const char* url, int sendPort, int receivePort, double timeout = 10, double pngInterv = 1, int buffSize = 256);
		~Connection();
		void send(const u8* data, int size, bool reliable = true);
		int receive(u8* data);
	private:
		enum PaketType {
			Control = 0,
			Reliable = 1,
			Unreliable = 2
		};
		enum ControlType {
			Ping = 0,
			Pong = 1
		};

		static const u32 magicID = 1346655563;
		static const int headerSize = 8;

		const int sndPort;
		const int recPort;
		const char* url;
		Kore::Socket socket;

		int buffSize;
		u32 lastSndNrRel;
		u32 lastSndNrURel;
		u32 lastRecNrRel;
		u32 lastRecNrURel;
		u8* recBuff;
		u8* sndBuff;

		double timeout;
		double pngInterv;
		double lastRec;
		double lastPng;

		void send(const u8* data, int size, PaketType type);
	};
}
