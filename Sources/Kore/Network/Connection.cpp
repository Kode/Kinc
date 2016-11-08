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
	lastSndNrRel = 0;
	lastSndNrURel = 0;

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

	// Identifier
	*((u32*)(sndBuff)) = (magicID & 0xFFFFFFF0) + type;
	// Reliability via sequence numbers (wrap around)
	if (type == Unreliable)
		*((u32*)(sndBuff + 4)) = lastSndNrURel++;
	else
		*((u32*)(sndBuff + 4)) = lastSndNrRel++;

	memcpy(sndBuff + headerSize, data, size);

	socket.send(url, sndPort, sndBuff, headerSize + size);
}

// Must be called regularily as it also keeps the connection alive
int Connection::receive(u8* data) {
	unsigned int recAddr;
	unsigned int recPort;

	// Regularily send a ping / keep-alive
	if ((System::time() - lastPng) > pngInterv) {
		u8 data[13];
		data[0] = Ping;
		*((double*)(data + 1)) = System::time();
		*((u32*)(data + 9)) = lastRecNrRel;
		send(data, 13, Control);

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

			int recNr = *((u32*)(recBuff + 4));

			PaketType type = (PaketType)(header & 0x0000000F);
			switch (type) {
			case Control: {
				// TODO: Handle missing packets (same as with reliable
				ControlType controlType = (ControlType)recBuff[headerSize];
				switch (controlType) {
				case Ping: {
					// Send back as pong
					u8 data[13];
					data[0] = Pong;
					*((double*)(data + 1)) = *((double*)(recBuff + headerSize + 1));
					int recNr = *((u32*)(recBuff + headerSize + 9));
					// TODO: Trigger resend if last paket is overdue (e.g. time > 1.5f * ping)
					send(data, 13, Control);
					break;
				}
				case Pong:
					// Measure ping
					ping = System::time() - *((double*)(recBuff + headerSize + 1));
					break;
				}
				break;
			}
			case Reliable:
				// TODO: Store new packets, request resend on missing ones
				if (recNr == lastRecNrURel + 1) {
					lastRecNrURel = recNr;

					// Prepare output
					int msgSize = size - headerSize;
					memcpy(data, recBuff + headerSize, msgSize);

					// Leave loop and return to caller
					return msgSize;
				}
				break;
			case Unreliable:
				// Ignore old packets, no resend
				if (recNr > lastRecNrURel) {
					lastRecNrURel = recNr;

					// Prepare output
					int msgSize = size - headerSize;
					memcpy(data, recBuff + headerSize, msgSize);

					// Leave loop and return to caller
					return msgSize;
				}
				break;
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
