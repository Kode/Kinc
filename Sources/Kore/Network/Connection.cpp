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

Connection::Connection(int receivePort, int maxConns, double timeout, double pngInterv, double resndInterv, double congestPing, float congestShare,
                       int buffSize, int cacheCount)
    : recPort(receivePort), maxConns(maxConns), timeout(timeout), pngInterv(pngInterv), resndInterv(resndInterv), congestPing(congestPing),
      congestShare(congestShare), buffSize(buffSize), cacheCount(cacheCount), activeConns(0), acceptConns(false) {

	socket.init();
	socket.open(receivePort);

	sndBuff = new u8[buffSize];
	sndCache = new u8[(buffSize + 12) * cacheCount];
	recBuff = new u8[buffSize];

	states = new State[maxConns];
	pings = new double[maxConns];
	congests = new bool[maxConns];

	connAdds = new unsigned[maxConns];
	connPorts = new int[maxConns];
	lastRecs = new double[maxConns];
	lastSndNrsRel = new u32[maxConns];
	lastSndNrsURel = new u32[maxConns];
	lastAckNrsRel = new u32[maxConns];
	lastRecNrsRel = new u32[maxConns];
	lastRecNrsURel = new u32[maxConns];
	congestBits = new u32[maxConns];
	recCaches = new u8[(buffSize + 12) * cacheCount * maxConns];

	for (int id = 0; id < maxConns; ++id) {
		reset(id, false);
	}
	// TODO: There is a synchronization issue if a new client connects before the last connection has timed out
}

Connection::~Connection() {
	delete[] sndBuff;
	delete[] sndCache;
	delete[] recBuff;
	delete[] recCaches;

	delete[] states;
	delete[] pings;
	delete[] congests;
	delete[] connAdds;
	delete[] connPorts;
	delete[] lastSndNrsRel;
	delete[] lastSndNrsURel;
	delete[] lastAckNrsRel;
	delete[] lastRecNrsRel;
	delete[] lastRecNrsURel;
	delete[] congestBits;
	delete[] lastRecs;
}

int Connection::getID(unsigned int recAddr, unsigned int recPort) {
	for (int id = 0; id < maxConns; ++id) {
		if (connAdds[id] == recAddr && connPorts[id] == recPort) {
			return id;
		}
	}
	return -1;
}

inline bool Connection::checkSeqNr(u32 next, u32 last) {
	return ((next > last || next < REC_NR_WINDOW) && next < last + REC_NR_WINDOW); // Wrap around handled by overflow
}

void Connection::listen() {
	acceptConns = true;
}

void Connection::connect(unsigned address, int port) {
	for (int id = 0; id < maxConns; ++id) {
		if (states[id] == Disconnected) {

			states[id] = Connecting;
			connAdds[id] = address;
			connPorts[id] = port;

			lastRecs[id] = System::time(); // Prevent premature timeout
			lastPng = 0;                   // Force ping immediately
			activeConns++;

			return;
		}
	}

	// All connection slots used?
	// Just returning a bool value could be seen as misleading since connect == true would not mean that an end point has been reached
	assert(false);
}

void Connection::connect(const char* url, int port) {
	connect(socket.urlToInt(url, port), port);
}

void Connection::send(const u8* data, int size, int connId, bool reliable) {
	sendPacket(data, size, connId, reliable, false);
}

void Connection::sendPacket(const u8* data, int size, int connId, bool reliable, bool control) {
	assert(size + HEADER_SIZE <= buffSize);

	memcpy(sndBuff + HEADER_SIZE, data, size);

	// Identifier
	*((u32*)(sndBuff)) = (PROTOCOL_ID & 0xFFFFFFF0) + reliable + 2 * control;

	if (connId >= 0) {
		sendPreparedBuffer(size, reliable, connId);
	}
	else {
		for (int id = 0; id < maxConns; ++id) {
			if (states[id] == Disconnected) continue;

			sendPreparedBuffer(size, reliable, id);
		}
	}
}

void Connection::sendPreparedBuffer(int size, bool reliable, int id) {
	// Reliable ack
	*((u32*)(sndBuff + 4)) = lastRecNrsRel[id];
	// Reliability via sequence numbers (wrap around via overflow)
	if (reliable) {
		*((u32*)(sndBuff + 8)) = ++lastSndNrsRel[id];
		// Cache message for potential resend
		*((double*)(sndCache + (lastSndNrsRel[id] % cacheCount) * buffSize)) = System::time();
		*((int*)(sndCache + (lastSndNrsRel[id] % cacheCount) * buffSize + 8)) = HEADER_SIZE + size;
		memcpy(sndCache + (lastSndNrsRel[id] % cacheCount) * buffSize + 12, sndBuff, HEADER_SIZE + size);
	}
	else {
		*((u32*)(sndBuff + 8)) = ++lastSndNrsURel[id];
	}

	// DEBUG ONLY: Introduce packet drop
	// if (!reliable || lastSndNrRel % 2)
	socket.send(connAdds[id], connPorts[id], sndBuff, HEADER_SIZE + size);
}

// Must be called regularily as it also keeps the connection alive
int Connection::receive(u8* data, int& id) {
	unsigned int recAddr;
	unsigned int recPort;

	// Regularily send a ping / keep-alive
	{
		if ((System::time() - lastPng) > pngInterv) {
			u8 data[9];
			data[0] = Ping;
			*((double*)(data + 1)) = System::time();
			sendPacket(data, 9, -1, false, true);

			lastPng = System::time();
		}
	}

	// Receive pending packets
	{
		int size = 0;
		while ((size = socket.receive(recBuff, buffSize, recAddr, recPort)) > 0) {
			assert(size < buffSize);

			id = getID(recAddr, recPort);
			// Unknown sender?
			if (id < 0) {
				if (acceptConns && activeConns < maxConns)
					connect(recAddr, recPort);
				else
					continue;
			}

			u32 header = *((u32*)(recBuff));
			// Check for prefix (stray packets)
			if ((header & 0xFFFFFFF0) != (PROTOCOL_ID & 0xFFFFFFF0)) continue;

			states[id] = Connected;
			lastRecs[id] = System::time();

			bool reliable = (header & 1) != 0;
			bool control = (header & 2) != 0;

			u32 ackNrRel = *((u32*)(recBuff + 4));
			if (checkSeqNr(ackNrRel, lastAckNrsRel[id])) { // Usage of range function is intentional as multiple packets can be acknowledged at the same time,
				                                           // stepwise increment handled by client
				lastAckNrsRel[id] = ackNrRel;
			}

			u32 recNr = *((u32*)(recBuff + 8));
			if (reliable) {
				if (recNr == lastRecNrsRel[id] + 1) { // Wrap around handled by overflow
					lastRecNrsRel[id] = recNr;

					// Process message
					if (control) {
						processControlMessage(id);
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
				if (checkSeqNr(recNr, lastRecNrsURel[id])) {
					lastRecNrsURel[id] = recNr;

					// Process message
					if (control) {
						processControlMessage(id);
					}
					else {
						// Leave loop and return to caller
						return processMessage(size, data);
					}
				}
			}
		}
	}

	// Connection maintenance
	{
		for (int id = 0; id < maxConns; ++id) {
			if (states[id] == Disconnected) continue;

			// Connection timeout?
			if ((System::time() - lastRecs[id]) > timeout) {
				reset(id, true);
			}
			// Trigger resend if last paket is overdue
			else if (lastSndNrsRel[id] != lastAckNrsRel[id]) {
				u8* cachedPacket = sndCache + ((lastAckNrsRel[id] + 1) % cacheCount) * buffSize;
				double* sndTime = ((double*)cachedPacket);
				if (System::time() - *sndTime > resndInterv) {
					int size = *((int*)(cachedPacket + 8));
					memcpy(sndBuff, cachedPacket + 12, size);
					socket.send(connAdds[id], connPorts[id], sndBuff, size);
					*sndTime += resndInterv;
				}
			}
		}
	}

	return 0;
}

void Connection::processControlMessage(int id) {
	ControlType controlType = (ControlType)recBuff[HEADER_SIZE];
	switch (controlType) {
	case Ping: {
		// Send back as pong
		u8 data[9];
		data[0] = Pong;
		*((double*)(data + 1)) = *((double*)(recBuff + HEADER_SIZE + 1));

		sendPacket(data, 9, id, false, true);
		break;
	}
	case Pong:
		// Measure ping
		double recPing = System::time() - *((double*)(recBuff + HEADER_SIZE + 1));
		// Don't smooth first ping
		if (pings[id] == -1)
			pings[id] = recPing;
		else
			pings[id] = (PNG_SMOOTHING * pings[id]) + (1 - PNG_SMOOTHING) * recPing;

		// Congestion check
		bool nowCongest = pings[id] > congestPing;
		congestBits[id] = (congestBits[id] << 1) + nowCongest;

		// Method by Brian Kernighan
		unsigned int set = congestBits[id];
		unsigned int all;
		for (all = 0; set; all++) {
			set &= set - 1;
		}

		congests[id] = ((float)set) / all > congestShare;

		break;
	}
}

int Connection::processMessage(int size, u8* returnBuffer) {
	// Prepare output
	int msgSize = size - HEADER_SIZE;
	memcpy(returnBuffer, recBuff + HEADER_SIZE, msgSize);

	return msgSize;
}

void Connection::reset(int id, bool decCount) {
	lastSndNrsRel[id] = 0;
	lastSndNrsURel[id] = 0;
	lastAckNrsRel[id] = 0;
	lastRecNrsRel[id] = 0;
	lastRecNrsURel[id] = 0;
	congestBits[id] = 0;

	states[id] = Disconnected;
	pings[id] = -1;
	lastRecs[id] = 0;
	congests[id] = false;

	if (decCount) {
		--activeConns;
	}
}
