#include "pch.h"
#include "Connection.h"
#include "Socket.h"

#include <cassert>
#include <cstring>

#include <Kore/System.h>

using namespace Kore;

Connection::Connection(const char* url, int sendPort, int receivePort, double timeout, double pngInterv, int buffSize, int cacheCount) :
		url(url),
		sndPort(sendPort),
		recPort(receivePort),
		timeout(timeout),
		pngInterv(pngInterv),
		buffSize(buffSize),
		cacheCount(cacheCount) {

	socket.init();
	socket.open(receivePort);

	sndBuff = new u8[buffSize];
	sndCache = new u8[buffSize * cacheCount];
	recBuff = new u8[buffSize];
	recCache = new u8[buffSize * cacheCount];
	lastSndNrRel = 0;
	lastSndNrURel = 0;
	lastRecNrRel = 0;
	lastRecNrURel = 0;

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
	send(data, size, reliable);
}

void Connection::send(const u8* data, int size, bool reliable, bool control) {
	assert(size + headerSize <= buffSize);

	memcpy(sndBuff + headerSize, data, size);
	// Identifier
	*((u32*)(sndBuff)) = (magicID & 0xFFFFFFF0) + reliable + 2 * control;
	// Reliability via sequence numbers (wrap around via overflow)
	if (reliable) {
		*((u32*)(sndBuff + 4)) = ++lastSndNrRel;
		// Cache message for potential resend
		memcpy(sndCache + (lastSndNrRel % cacheCount) * buffSize, sndBuff, headerSize + size);
	}
	else {
		*((u32*)(sndBuff + 4)) = lastSndNrURel++;
	}

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
		send(data, 13, false, true);

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

			bool reliable = (header & 1);
			bool control = (header & 2);
			int recNr = *((u32*)(recBuff + 4));
			if (reliable) {
				if (recNr == lastRecNrURel + 1) {
					lastRecNrURel = recNr;

					// Process message
					if (control) {
						processControlMessage();
					}
					else {
						// Leave loop and return to caller
						return processMessage(size, data);
					}
				}
				else {
					// TODO: Store new packets, request resend on missing ones, process pending if resend on old
				}
			}
			else {
				// Ignore old packets, no resend
				if (recNr > lastRecNrURel) {
					lastRecNrURel = recNr;

					// Process message
					if (control) {
						processControlMessage();
					}
					else {
						// Leave loop and return to caller
						return processMessage(size, data);
					}
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

void Connection::processControlMessage() {
	ControlType controlType = (ControlType)recBuff[headerSize];
	switch (controlType) {
	case Ping: {
		// Send back as pong
		u8 data[13];
		data[0] = Pong;
		*((double*)(data + 1)) = *((double*)(recBuff + headerSize + 1));
		int recNr = *((u32*)(recBuff + headerSize + 9));
		// TODO: Trigger resend if last paket is overdue (e.g. time > 1.5f * ping)
		send(data, 13, false, true);
		break;
	}
	case Pong:
		// Measure ping
		ping = System::time() - *((double*)(recBuff + headerSize + 1));
		break;
	}
}

int Connection::processMessage(int size, u8* returnBuffer) {
	// Prepare output
	int msgSize = size - headerSize;
	memcpy(returnBuffer, recBuff + headerSize, msgSize);

	return msgSize;
}
