#include "pch.h"
#include "Connection.h"
#include "Socket.h"

#include <cstring>

using namespace Kore;

Connection::Connection(const char* url, int sendPort, int receivePort) : url(url), sendPort(sendPort), receivePort(receivePort) {
	socket = new Socket();
	socket->open(receivePort);

	bufferUsage = 0;
}

Connection::~Connection() {
	delete socket;
}

void Connection::send(const unsigned char* data, int size) {
	socket->send(url, sendPort, data, size);
}

int Connection::receive(unsigned char* data) {
	unsigned int recAddr;
	unsigned int recPort;
	int size = socket->receive(buffer + bufferUsage, connBuffSize - bufferUsage, recAddr, recPort);
	if (size >= 0) {
		bufferUsage += size;

		// TODO: Check for completeness etc.
		bool complete = true;
		if (complete) {
			// Prepare output
			int msgSize = bufferUsage;
			memcpy(data, buffer, bufferUsage);

			// Clear buffer
			// TODO: There might be a part of the next message
			bufferUsage = 0;

			return msgSize;
		}
	}
	return 0;
}
