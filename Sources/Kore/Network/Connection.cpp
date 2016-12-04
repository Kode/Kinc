#include "pch.h"
#include "Connection.h"
#include "Socket.h"

#include <cassert>
#include <cstring>

#include <Kore/System.h>

using namespace Kore;

namespace {
	const u32 PROTOCOL_ID = 1346655563;
	const u32 REC_NR_WINDOW = ((u32)-1) / 4;
	const int HEADER_SIZE = 12;
	const double PNG_SMOOTHING = 0.1; // png = (value * old) + (1 - value) * new
}

Connection::Connection(const char* url, int sendPort, int receivePort, double timeout, double pngInterv, double resndInterv, double congestPing, float congestShare, int buffSize, int cacheCount) :
		url(url),
		sndPort(sendPort),
		recPort(receivePort),
		timeout(timeout),
		pngInterv(pngInterv),
		resndInterv(resndInterv),
		congestPing(congestPing),
		congestShare(congestShare),
		buffSize(buffSize),
		cacheCount(cacheCount) {

	socket.init();
	socket.open(receivePort);

	sndBuff = new u8[buffSize];
	sndCache = new u8[(buffSize + 12) * cacheCount];
	recBuff = new u8[buffSize];
	recCache = new u8[(buffSize + 12) * cacheCount];
	
	reset();
	// TODO: (Dis-)connection handling, especially for the server (broadcasting, control messages - client hello / ping) -> maybe split into two classes
	// TODO: There is a synchronization issue if a new client connects before the last connection has timed out
}

Connection::~Connection() {
	delete sndBuff;
	delete sndCache;
	delete recBuff;
	delete recCache;
}

void Connection::send(const u8* data, int size, bool reliable) {
	send(data, size, reliable, false);
}

inline bool Connection::checkSeqNr(u32 next, u32 last) {
	return ((next > last || next < REC_NR_WINDOW) && next < last + REC_NR_WINDOW); // Wrap around handled by overflow
}

void Connection::send(const u8* data, int size, bool reliable, bool control) {
	assert(size + HEADER_SIZE <= buffSize);

	memcpy(sndBuff + HEADER_SIZE, data, size);
	// Identifier
	*((u32*)(sndBuff)) = (PROTOCOL_ID & 0xFFFFFFF0) + reliable + 2 * control;
	// Reliable ack
	*((u32*)(sndBuff + 4)) = lastRecNrRel;
	// Reliability via sequence numbers (wrap around via overflow)
	if (reliable) {
		*((u32*)(sndBuff + 8)) = ++lastSndNrRel;
		// Cache message for potential resend
		*((double*)(sndCache + (lastSndNrRel % cacheCount) * buffSize)) = System::time();
		*((int*)(sndCache + (lastSndNrRel % cacheCount) * buffSize + 8)) = HEADER_SIZE + size;
		memcpy(sndCache + (lastSndNrRel % cacheCount) * buffSize + 12, sndBuff, HEADER_SIZE + size);
	}
	else {
		*((u32*)(sndBuff + 8)) = ++lastSndNrURel;
	}

	// DEBUG ONLY: Introduce packet drop
	//if (!reliable || lastSndNrRel % 2)
	socket.send(url, sndPort, sndBuff, HEADER_SIZE + size);
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
		send(data, 9, false, true);

		lastPng = System::time();
	}

	int size = 0;
	while ((size = socket.receive(recBuff, buffSize, recAddr, recPort)) > 0) {
		assert(size < buffSize);

		// Check for prefix (stray packets)
		u32 header = *((u32*)(recBuff));
		if ((header & 0xFFFFFFF0) == (PROTOCOL_ID & 0xFFFFFFF0)) {
			state = Connected;
			lastRec = System::time();

			bool reliable = (header & 1);
			bool control = (header & 2);
			
			u32 ackNrRel = *((u32*)(recBuff + 4));
			if (checkSeqNr(ackNrRel, lastAckNrRel)) { // Usage of range function is intentional as multiple packets can be acknowledged at the same time, stepwise increment handled by client
				lastAckNrRel = ackNrRel;
			}

			u32 recNr = *((u32*)(recBuff + 8));
			if (reliable) {
				if (recNr == lastRecNrRel + 1) { // Wrap around handled by overflow
					lastRecNrRel = recNr;

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
					// TODO (Currently naive resend of everything): Store new packets, request resend on missing ones, process pending if resend on old
				}
			}
			else {
				// Ignore old packets, no resend
				if (checkSeqNr(recNr, lastRecNrURel)) {
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
			}
		}
	}

	// Connection timeout?
	if ((System::time() - lastRec) > timeout) {
		reset();
	}
	// Trigger resend if last paket is overdue
	else if (lastSndNrRel != lastAckNrRel) {
		u8* cachedPacket = sndCache + ((lastAckNrRel + 1) % cacheCount) * buffSize;
		double* sndTime = ((double*)cachedPacket);
		if (System::time() - *sndTime > resndInterv) {
			int size = *((int*)(cachedPacket + 8));
			memcpy(sndBuff, cachedPacket + 12, size);
			socket.send(url, sndPort, sndBuff, size);
			*sndTime += resndInterv;
		}
	}

	return 0;
}

void Connection::processControlMessage() {
	ControlType controlType = (ControlType)recBuff[HEADER_SIZE];
	switch (controlType) {
	case Ping: {
		// Send back as pong
		u8 data[9];
		data[0] = Pong;
		*((double*)(data + 1)) = *((double*)(recBuff + HEADER_SIZE + 1));

		send(data, 9, false, true);
		break;
	}
	case Pong:
		// Measure ping
		double recPing = System::time() - *((double*)(recBuff + HEADER_SIZE + 1));
		// Don't smooth first ping
		if (ping == -1) ping = recPing;
		else ping = (PNG_SMOOTHING * ping) + (1 - PNG_SMOOTHING) * recPing;

		// Congestion check
		bool nowCongest = ping > congestPing;
		congestBits = (congestBits << 1) + nowCongest;

		// Method by Brian Kernighan
		unsigned int set, all;
		for (all = 0; set; all++) {
			set &= set - 1;
		}
		congested = ((float)set) / all > congestShare;

		break;
	}
}

int Connection::processMessage(int size, u8* returnBuffer) {
	// Prepare output
	int msgSize = size - HEADER_SIZE;
	memcpy(returnBuffer, recBuff + HEADER_SIZE, msgSize);

	return msgSize;
}

void Connection::reset() {
	lastSndNrRel = 0;
	lastSndNrURel = 0;
	lastAckNrRel = 0;
	lastRecNrRel = 0;
	lastRecNrURel = 0;
	congestBits = 0;

	state = Disconnected;
	ping = -1;
	lastRec = 0;
	lastPng = 0;
	congested = false;
}
