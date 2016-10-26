#include "pch.h"
#include "Connection.h"
#include "Socket.h"

#include <cassert>
#include <cstring>

#include <Kore/System.h>

using namespace Kore;

Connection::Connection(const char* url, int sendPort, int receivePort, double timeout, double pngInterv, int buffSize) :
		url(url),
		sndPort(sendPort),
		recPort(receivePort),
		timeout(timeout),
		pngInterv(pngInterv),
		buffSize(buffSize) {

	socket.init();
	socket.open(receivePort);

	sndBuff = new u8[buffSize];
	recBuff = new u8[buffSize];
	lastSndNr = 0;

	state = Disconnected;
	ping = -1;
	lastRec = 0;
	lastPng = 0;
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
	*((u32*)(sndBuff)) = (magicID & 0xFFFFFFF0) + type;
	// Reliability via sequence numbers (wrap around)
	*((u32*)(sndBuff + 4)) = lastSndNr++;

	memcpy(sndBuff + headerSize, data, size);

	socket.send(url, sndPort, sndBuff, headerSize + size);
}

// Must be called regularily as it also keeps the connection alive
int Connection::receive(u8* data) {
	unsigned int recAddr;
	unsigned int recPort;

	// Regularily send a ping / keep-alive
	if ((System::time() - lastPng) > pngInterv) {
		u8 data[9];
		data[0] = Ping;
		*((double*)(data + 1)) = System::time();
		send(data, 9, Control);

		lastPng = System::time();
	}

	int size = 0;
	while ((size = socket.receive(recBuff, buffSize, recAddr, recPort)) > 0) {
		assert(size < buffSize);

		// Check for prefix (stray packets)
		u32 header = *((u32*)(recBuff));
		if ((header & 0xFFFFFFF0) == (magicID & 0xFFFFFFF0)) {
			state = Connected;
			lastRec = System::time();

			PaketType type = (PaketType)(header & 0x0000000F);
			if (type == Control) {
				
				ControlType controlType = (ControlType)recBuff[headerSize];
				switch (controlType) {
				case Ping:
					// Send back as pong
					u8 data[9];
					data[0] = Pong;
					*((double*)(data + 1)) = *((double*)(recBuff + headerSize + 1));
					send(data, 9, Control);
					break;
				case Pong:
					// Measure ping
					ping = System::time() - *((double*)(recBuff + headerSize + 1));
					break;
				}
			}
			else {
				// TODO: Handle missing packets
				int recNr = *((u32*)(recBuff + 4));

				// Prepare output
				int msgSize = size - headerSize;
				memcpy(data, recBuff + headerSize, msgSize);

				// Leave loop and return to caller
				return msgSize;
			}
		}
	}

	// Connection timeout?
	if ((System::time() - lastRec) > timeout) {
		ping = -1;
		state = Disconnected;
	}

	return 0;
}
