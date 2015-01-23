#include "pch.h"
#include "Socket.h"
#include <Kore/Log.h>

#ifdef SYS_WINDOWS
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h> 
#endif

using namespace Kore;

namespace {
	bool initialized = false;

	void init() {
		if (initialized) return;
#ifdef SYS_WINDOWS
		WSADATA WsaData;
		WSAStartup(MAKEWORD(2, 2), &WsaData);
#endif
		initialized = true;
	}

	void destroy() {
#ifdef SYS_WINDOWS
		WSACleanup();
#endif
	}
}

Socket::Socket() : handle(0) {
	init();
}

void Socket::open(int port) {
	handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (handle <= 0) {
		log(Kore::Error, "Could not create socket.");
		return;
	}

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((unsigned short) port);
	if (bind(handle, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0) {
		log(Kore::Error, "Could not bind socket.");
		return;
	}

#ifdef SYS_WINDOWS
	DWORD nonBlocking = 1;
	if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0) {
		log(Kore::Error, "Could not set non-blocking mode.");
		return;
	}
#else
	int nonBlocking = 1;
	if (fcntl(handle, F_SETFL, O_NONBLOCK, nonBlocking ) == -1) {
		log(Kore::Error, "Could not set non-blocking mode.");
		return;
	}
#endif
}

Socket::~Socket() {
#ifdef SYS_WINDOWS
	closesocket(handle);
#else
	close(handle);
#endif
}

void Socket::send(unsigned addr1, unsigned addr2, unsigned addr3, unsigned addr4, unsigned short port, const unsigned char* data, int size) {
	unsigned int address = (addr1 << 24) | (addr2 << 16) | (addr3 << 8) | addr4;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(address);
	addr.sin_port = htons(port);

	int sent = sendto(handle, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in));
	if (sent != size) {
		log(Kore::Error, "Could not send packet.");
		return;
	}
}

int Socket::receive(unsigned char* data, int maxSize, unsigned& fromAddress, unsigned& fromPort) {
	while (true) {
#ifdef SYS_WINDOWS
		typedef int socklen_t;
#endif
		sockaddr_in from;
		socklen_t fromLength = sizeof(from);
		int bytes = recvfrom(handle, (char*)data, maxSize, 0, (sockaddr*)&from, &fromLength);
		if (bytes <= 0) return bytes;
		fromAddress = ntohl(from.sin_addr.s_addr);
		fromAddress = ntohs(from.sin_port);
		return bytes;
	}
}
