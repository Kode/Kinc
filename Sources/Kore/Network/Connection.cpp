#include "pch.h"
#include "Connection.h"
#include "Socket.h"

#include <cassert>
#include <cstring>

using namespace Kore;

Connection::Connection(const char* url, int sendPort, int receivePort) : url(url), sndPort(sendPort), recPort(receivePort) {
	socket = new Socket();
	socket->open(receivePort);

	// TODO: (Dis-)connection handling, especially for the server
	// TODO: Broadcasting and specific clients (discuss with RK)
}

Connection::~Connection() {
	delete socket;
}

void Connection::send(const unsigned char* data, int size, bool reliable) {
	assert(size + headerSize <= sndBuffSize);

	// TODO: Separate seq nrs for reliable and unreliable (only discarded on old)

	// Identifier
	*((unsigned int*)(sndBuff)) = magicID;
	// Reliability via sequence numbers (wrap around)
	*((unsigned int*)(sndBuff + 2)) = lastSndNr++;

	memcpy(sndBuff + headerSize, data, size);

	socket->send(url, sndPort, sndBuff, headerSize + size);
}

int Connection::receive(unsigned char* data) {
	unsigned int recAddr;
	unsigned int recPort;
	
	int size = socket->receive(recBuff, recBuffSize, recAddr, recPort);
	assert(size < recBuffSize);

	if (size >= 0) {
		// Check for prefix (stray packets)
		if (*((unsigned int*)(recBuff)) == magicID) {
			// TODO: Handle missing packets
			int recNr = *((unsigned int*)(recBuff + 2));

			// Prepare output
			int msgSize = size - headerSize;
			memcpy(data, recBuff + headerSize, msgSize);
			
			return msgSize;
		}
	}

	return 0;
}
