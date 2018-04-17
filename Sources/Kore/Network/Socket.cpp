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

namespace {
	bool initialized = false;

	void destroy() {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
		WSACleanup();
#endif
	}

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	// Important: Must be cleaned with freeaddrinfo(address) later if the result is 0 in order to prevent memory leaks
	int resolveAddress(const char* url, int port, addrinfo** result) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
		addrinfo hints = {};
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		char serv[6];
		sprintf(serv, "%u", port);

		return getaddrinfo(url, serv, &hints, result);
#endif
	}
#endif
}

Socket::Socket() {}

void Socket::init() {
	if (initialized) return;

	handle = 0;
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);
#endif
	initialized = true;
}

void Socket::open(int port) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (handle <= 0) {
		log(Kore::Error, "Could not create socket.");
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
		int errorCode = WSAGetLastError();
		switch (errorCode) {
		case (WSANOTINITIALISED):
			Kore::log(Error, "A successful WSAStartup call must occur before using this function.");
			break;
		case (WSAENETDOWN):
			Kore::log(Error, "The network subsystem or the associated service provider has failed.");
			break;
		case (WSAEAFNOSUPPORT):
			Kore::log(Error, "The specified address family is not supported.For example, an application tried to create a socket for the AF_IRDA address "
			                 "family but an infrared adapter and device driver is not installed on the local computer.");
			break;
		case (WSAEINPROGRESS):
			Kore::log(Error, "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
			break;
		case (WSAEMFILE):
			Kore::log(Error, "No more socket descriptors are available.");
			break;
		case (WSAEINVAL):
			Kore::log(Error, "An invalid argument was supplied.This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol "
			                 "parameter are unspecified.");
			break;
		case (WSAENOBUFS):
			Kore::log(Error, "No buffer space is available.The socket cannot be created.");
			break;
		case (WSAEPROTONOSUPPORT):
			Kore::log(Error, "The specified protocol is not supported.");
			break;
		case (WSAEPROTOTYPE):
			Kore::log(Error, "The specified protocol is the wrong type for this socket.");
			break;
		case (WSAEPROVIDERFAILEDINIT):
			Kore::log(Error, "The service provider failed to initialize.This error is returned if a layered service provider(LSP) or namespace provider was "
			                 "improperly installed or the provider fails to operate correctly.");
			break;
		case (WSAESOCKTNOSUPPORT):
			Kore::log(Error, "The specified socket type is not supported in this address family.");
			break;
		case (WSAEINVALIDPROVIDER):
			Kore::log(Error, "The service provider returned a version other than 2.2.");
			break;
		case (WSAEINVALIDPROCTABLE):
			Kore::log(Error, "The service provider returned an invalid or incomplete procedure table to the WSPStartup.");
			break;
		default:
			Kore::log(Error, "Unknown error.");
		}
#endif
		return;
	}

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((unsigned short)port);
	if (bind(handle, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0) {
		log(Kore::Error, "Could not bind socket.");
		return;
	}
#endif

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	DWORD nonBlocking = 1;
	if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0) {
		log(Kore::Error, "Could not set non-blocking mode.");
		return;
	}
#elif defined(KORE_POSIX)
	int nonBlocking = 1;
	if (fcntl(handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1) {
		log(Kore::Error, "Could not set non-blocking mode.");
		return;
	}
#endif
}

Socket::~Socket() {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	closesocket(handle);
#elif defined(KORE_POSIX)
	close(handle);
#endif
	destroy();
}

unsigned Socket::urlToInt(const char* url, int port) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	addrinfo* address = nullptr;
	int res = resolveAddress(url, port, &address);
	if (res != 0) {
		log(Kore::Error, "Could not resolve address.");
		return -1;
	}

	unsigned fromAddress = ntohl(((sockaddr_in*)address->ai_addr)->sin_addr.S_un.S_addr);
	freeaddrinfo(address);

	return fromAddress;
#else
	return 0;
#endif
}

void Socket::send(unsigned address, int port, const u8* data, int size) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(address);
	addr.sin_port = htons(port);

	size_t sent = sendto(handle, (const char*)data, size, 0, (sockaddr*)&addr, sizeof(sockaddr_in));
	if (sent != size) {
		log(Kore::Error, "Could not send packet.");
	}
#endif
}

void Socket::send(const char* url, int port, const u8* data, int size) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	addrinfo* address = nullptr;
	int res = resolveAddress(url, port, &address);
	if (res != 0) {
		log(Kore::Error, "Could not resolve address.");
		return;
	}

	size_t sent = sendto(handle, (const char*)data, size, 0, address->ai_addr, sizeof(sockaddr_in));
	if (sent != size) {
		log(Kore::Error, "Could not send packet.");
	}
	freeaddrinfo(address);
#endif
}

int Socket::receive(u8* data, int maxSize, unsigned& fromAddress, unsigned& fromPort) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	typedef int socklen_t;
	typedef int ssize_t;
#endif
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	sockaddr_in from;
	socklen_t fromLength = sizeof(from);
	ssize_t bytes = recvfrom(handle, (char*)data, maxSize, 0, (sockaddr*)&from, &fromLength);
	if (bytes <= 0) return static_cast<int>(bytes);
	fromAddress = ntohl(from.sin_addr.s_addr);
	fromPort = ntohs(from.sin_port);
	return static_cast<int>(bytes);
#else
	return 0;
#endif
}
