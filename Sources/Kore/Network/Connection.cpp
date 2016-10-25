#include "pch.h"
#include "Connection.h"
#include "Socket.h"

#include <cassert>
#include <cstring>

using namespace Kore;

Connection::Connection(const char* url, int sendPort, int receivePort, int buffSize) : url(url), sndPort(sendPort), recPort(receivePort), buffSize(buffSize) {
	socket.init();
	socket.open(receivePort);

	sndBuff = new u8[buffSize];
	recBuff = new u8[buffSize];
	// TODO: (Dis-)connection handling, especially for the server (broadcasting, control messages - client hello / ping) -> maybe split into two classes
}

Connection::~Connection() {
	delete sndBuff;
	delete recBuff;
}

void Connection::send(const u8* data, int size, bool reliable) {
	assert(size + headerSize <= buffSize);

	// TODO: Separate seq nrs for reliable and unreliable (only discarded on old)

	// Identifier
	*((u32*)(sndBuff)) = magicID;
	// Reliability via sequence numbers (wrap around)
	*((u32*)(sndBuff + 4)) = lastSndNr++;

	memcpy(sndBuff + headerSize, data, size);

	socket.send(url, sndPort, sndBuff, headerSize + size);
}

int Connection::receive(u8* data) {
	unsigned int recAddr;
	unsigned int recPort;
	
	int size = socket.receive(recBuff, buffSize, recAddr, recPort);
	assert(size < buffSize);

	if (size >= 0) {
		// Check for prefix (stray packets)
		if (*((u32*)(recBuff)) == magicID) {
			// TODO: Handle missing packets
			int recNr = *((u32*)(recBuff + 4));

			// Prepare output
			int msgSize = size - headerSize;
			memcpy(data, recBuff + headerSize, msgSize);
			
			return msgSize;
		}
	}

	return 0;
}
