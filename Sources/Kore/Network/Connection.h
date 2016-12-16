#pragma once

#include <Kore/Network/Socket.h>

namespace Kore {

	class Connection {
	public:
		enum State {
			Disconnected = 0,
			Connecting = 1,
			Connected = 2,
		};

		int maxConns;
		int activeConns;

		// For each connected entity
		State* states;
		double* pings;
		bool* congests;

		Connection(int receivePort, int maxConns, double timeout = 10, double pngInterv = 1, double resndInterv = 0.2, double congestPing = 0.2,
		           float congestShare = 0.5, int buffSize = 256, int cacheCount = 20);
		~Connection();

		void listen();
		void connect(unsigned address, int port);
		void connect(const char* url, int port);
		void send(const u8* data, int size, int connId = -1, bool reliable = true);
		int receive(u8* data, int& fromId);

	private:
		enum ControlType { Ping = 0, Pong = 1 };

		bool acceptConns;
		const int recPort;
		Kore::Socket socket;

		// For each connected entity
		unsigned* connAdds;
		int* connPorts;
		double* lastRecs;
		u32* lastSndNrsRel;
		u32* lastSndNrsURel;
		u32* lastAckNrsRel;
		u32* lastRecNrsRel;
		u32* lastRecNrsURel;
		u32* congestBits;
		u8* recCaches;

		int buffSize;
		int cacheCount;
		u8* recBuff;
		u8* sndBuff;
		u8* sndCache;

		float congestShare;
		double timeout;
		double pngInterv;
		double resndInterv;
		double congestPing;
		double lastPng;

		int getID(unsigned int recAddr, unsigned int recPort);
		void sendPacket(const u8* data, int size, int connId, bool reliable, bool control);
		void sendPreparedBuffer(int size, bool reliable, int id);
		bool checkSeqNr(u32 next, u32 last);
		void processControlMessage(int id);
		int processMessage(int size, u8* returnBuffer);
		void reset(int id, bool decCount);
	};
}
