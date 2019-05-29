#include "pch.h"

#include "Socket.h"

#include <Kore/Log.h>

#include <stdio.h>

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
#include <Ws2tcpip.h>
#include <winsock2.h>
#elif defined(KORE_POSIX)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

using namespace Kore;

Socket::Socket() {}

void Socket::init() {
	Kinc_Socket_Init(&sock);
}

void Socket::open(int port) {
	Kinc_Socket_Open(&sock, port);
}

Socket::~Socket() {
	Kinc_Socket_Destroy(&sock);
}

unsigned Socket::urlToInt(const char* url, int port) {
	return Kinc_urlToInt(url, port);
}

void Socket::send(unsigned address, int port, const u8 *data, int size) {
	Kinc_Socket_Send(&sock, address, port, data, size);

}

void Socket::setBroadcastEnabled(bool enabled) {
	Kinc_Socket_SetBroadcastEnabled(&sock, enabled);
}

void Socket::send(const char *url, int port, const u8 *data, int size) {
	Kinc_Socket_Send_URL(&sock, url, port, data, size);
}

int Socket::receive(u8* data, int maxSize, unsigned& fromAddress, unsigned& fromPort) {
	return Kinc_Socket_Receive(&sock, data, maxSize, &fromAddress, &fromPort);
}
