#include "pch.h"
#include "Connection.h"
#include "Socket.h"

#include <cassert>
#include <cstring>

#include <Kore/System.h>

using namespace Kore;

Connection::Connection(const char* url, int sendPort, int receivePort, double timeout, int buffSize) :
		url(url),
		sndPort(sendPort),
		recPort(receivePort),
		timeout(timeout),
		buffSize(buffSize) {

	socket.init();
	socket.open(receivePort);

	sndBuff = new u8[buffSize];
	recBuff = new u8[buffSize];

	state = Disconnected;
	lastTime = System::time();
	lastSndNr = 0;
	// TODO: (Dis-)connection handling, especially for the server (broadcasting, control messages - client hello / ping) -> maybe split into two classes
}

Connection::~Connection() {
	delete sndBuff;
	delete recBuff;
}

void Connection::send(const u8* data, int size, bool reliable) {
	send(data, size, reliable ? Reliable : Unreliable);
}

void Connection::send(const u8* data, int size, PaketType type) {
	assert(size + headerSize <= buffSize);

	// TODO: Separate seq nrs for reliable and unreliable (only discarded on old)

	// Identifier
	*((u32*)(sndBuff)) = (magicID & 0xFFFFFF00) + type;
	// Reliability via sequence numbers (wrap around)
	*((u32*)(sndBuff + 4)) = lastSndNr++;

	memcpy(sndBuff + headerSize, data, size);

	socket.send(url, sndPort, sndBuff, headerSize + size);
}

// Must be called regularily as it also keeps the connection alive
int Connection::receive(u8* data) {
	unsigned int recAddr;
	unsigned int recPort;
	
	int size = socket.receive(recBuff, buffSize, recAddr, recPort);
	assert(size < buffSize);

	if (size >= 0) {
		// Check for prefix (stray packets)
		u32 header = *((u32*)(recBuff));
		if ((header & 0xFFFFFF00) == (magicID & 0xFFFFFF00)) {
			state = Connected;
			lastTime = System::time();

			PaketType type = (PaketType)(header & 0x000000FF);

			// TODO: Handle missing packets
			int recNr = *((u32*)(recBuff + 4));

			// Prepare output
			int msgSize = size - headerSize;
			memcpy(data, recBuff + headerSize, msgSize);
			
			return msgSize;
		}
	}

	if (System::time() - lastTime > timeout) {
		state = Disconnected;
	}

	return 0;
}
