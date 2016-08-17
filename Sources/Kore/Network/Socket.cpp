#include "pch.h"
#include "Socket.h"
#include <stdio.h>
#include <Kore/Log.h>

#if defined(SYS_WINDOWS) || defined(SYS_WINDOWSAPP)
#include <winsock2.h>
#include <Ws2tcpip.h>
#elif defined(SYS_UNIXOID)
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#endif

using namespace Kore;

namespace {
	bool initialized = false;

	void init() {
		if (initialized) return;
#if defined(SYS_WINDOWS) || defined(SYS_WINDOWSAPP)
		WSADATA WsaData;
		WSAStartup(MAKEWORD(2, 2), &WsaData);
#endif
		initialized = true;
	}

	void destroy() {
#if defined(SYS_WINDOWS) || defined(SYS_WINDOWSAPP)
		WSACleanup();
#endif
	}
}

Socket::Socket() : handle(0) {
	init();
}

void Socket::open(int port) {
#if defined(SYS_WINDOWS) || defined(SYS_WINDOWSAPP) || defined(SYS_UNIXOID)
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
#endif

#if defined(SYS_WINDOWS) || defined(SYS_WINDOWSAPP)
	DWORD nonBlocking = 1;
	if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0) {
		log(Kore::Error, "Could not set non-blocking mode.");
		return;
	}
#elif defined(SYS_UNIXOID)
	int nonBlocking = 1;
	if (fcntl(handle, F_SETFL, O_NONBLOCK, nonBlocking ) == -1) {
		log(Kore::Error, "Could not set non-blocking mode.");
		return;
	}
#endif
}

Socket::~Socket() {
#if defined(SYS_WINDOWS) || defined(SYS_WINDOWSAPP)
	closesocket(handle);
#elif defined(SYS_UNIXOID)
	close(handle);
#endif
}

void Socket::send(const char* url, int port, const unsigned char* data, int size) {
#if defined(SYS_WINDOWS) || defined(SYS_WINDOWSAPP) || defined(SYS_UNIXOID)
	addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	addrinfo* address = NULL;
	char serv[5];
	sprintf(serv, "%u", port);
	
	int res = getaddrinfo(url, serv, &hints, &address);
	if (res != 0) {
		log(Kore::Error, "Could not resolve address.");
		return;
	}
	
	int sent = sendto(handle, (const char*)data, size, 0, address->ai_addr, sizeof(sockaddr_in));
	if (sent != size) {
		log(Kore::Error, "Could not send packet.");
		return;
	}
#endif
}

int Socket::receive(unsigned char* data, int maxSize, unsigned& fromAddress, unsigned& fromPort) {
#if defined(SYS_WINDOWS) || defined(SYS_WINDOWSAPP)
	typedef int socklen_t;
#endif
#if defined(SYS_WINDOWS) || defined(SYS_WINDOWSAPP) || defined(SYS_UNIXOID)
	sockaddr_in from;
	socklen_t fromLength = sizeof(from);
	int bytes = recvfrom(handle, (char*)data, maxSize, 0, (sockaddr*)&from, &fromLength);
	if (bytes <= 0) return bytes;
	fromAddress = ntohl(from.sin_addr.s_addr);
	fromPort = ntohs(from.sin_port);
	return bytes;
#else
	return 0;
#endif
}
