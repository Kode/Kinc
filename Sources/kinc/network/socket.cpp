#include "pch.h"

#include "Socket.h"

#include <Kinc/Log.h>

#include <stdbool.h>
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

static bool initialized = false;

static void destroy() {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	WSACleanup();
#endif
}

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
// Important: Must be cleaned with freeaddrinfo(address) later if the result is 0 in order to prevent memory leaks
static int resolveAddress(const char* url, int port, addrinfo** result) {
	addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	char serv[6];
	sprintf(serv, "%u", port);

	return getaddrinfo(url, serv, &hints, result);
}
#endif

void Kinc_Socket_Init(Kinc_Socket *sock) {
	if (initialized) return;

	sock->handle = 0;
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);
#endif
	initialized = true;
}

void Kinc_Socket_Open(Kinc_Socket *sock, int port) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	sock->handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock->handle <= 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not create socket.");
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
		int errorCode = WSAGetLastError();
		switch (errorCode) {
		case (WSANOTINITIALISED):
			kinc_log(KINC_LOG_LEVEL_ERROR, "A successful WSAStartup call must occur before using this function.");
			break;
		case (WSAENETDOWN):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The network subsystem or the associated service provider has failed.");
			break;
		case (WSAEAFNOSUPPORT):
			kinc_log(KINC_LOG_LEVEL_ERROR,
			          "The specified address family is not supported.For example, an application tried to create a socket for the AF_IRDA address "
			                 "family but an infrared adapter and device driver is not installed on the local computer.");
			break;
		case (WSAEINPROGRESS):
			kinc_log(KINC_LOG_LEVEL_ERROR,
			          "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
			break;
		case (WSAEMFILE):
			kinc_log(KINC_LOG_LEVEL_ERROR, "No more socket descriptors are available.");
			break;
		case (WSAEINVAL):
			kinc_log(KINC_LOG_LEVEL_ERROR,
			          "An invalid argument was supplied.This error is returned if the af parameter is set to AF_UNSPEC and the type and protocol "
			                 "parameter are unspecified.");
			break;
		case (WSAENOBUFS):
			kinc_log(KINC_LOG_LEVEL_ERROR, "No buffer space is available.The socket cannot be created.");
			break;
		case (WSAEPROTONOSUPPORT):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The specified protocol is not supported.");
			break;
		case (WSAEPROTOTYPE):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The specified protocol is the wrong type for this socket.");
			break;
		case (WSAEPROVIDERFAILEDINIT):
			kinc_log(KINC_LOG_LEVEL_ERROR,
			          "The service provider failed to initialize.This error is returned if a layered service provider(LSP) or namespace provider was "
			                 "improperly installed or the provider fails to operate correctly.");
			break;
		case (WSAESOCKTNOSUPPORT):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The specified socket type is not supported in this address family.");
			break;
		case (WSAEINVALIDPROVIDER):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The service provider returned a version other than 2.2.");
			break;
		case (WSAEINVALIDPROCTABLE):
			kinc_log(KINC_LOG_LEVEL_ERROR, "The service provider returned an invalid or incomplete procedure table to the WSPStartup.");
			break;
		default:
			kinc_log(KINC_LOG_LEVEL_ERROR, "Unknown error.");
		}
#endif
		return;
	}

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((unsigned short)port);
	if (bind(sock->handle, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not bind socket.");
		return;
	}
#endif

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	DWORD nonBlocking = 1;
	if (ioctlsocket(sock->handle, FIONBIO, &nonBlocking) != 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not set non-blocking mode.");
		return;
	}
#elif defined(KORE_POSIX)
	int nonBlocking = 1;
	if (fcntl(sock->handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1) {
		Kinc_Log(KINC_LOG_LEVEL_ERROR, "Could not set non-blocking mode.");
		return;
	}
#endif
}

void Kinc_Socket_Destroy(Kinc_Socket *sock) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	closesocket(sock->handle);
#elif defined(KORE_POSIX)
	close(sock->handle);
#endif
	destroy();
}

unsigned Kinc_urlToInt(const char *url, int port) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	addrinfo* address = nullptr;
	int res = resolveAddress(url, port, &address);
	if (res != 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not resolve address.");
		return -1;
	}

	unsigned fromAddress = ntohl(((sockaddr_in*)address->ai_addr)->sin_addr.S_un.S_addr);
	freeaddrinfo(address);

	return fromAddress;
#else
	return 0;
#endif
}

void Kinc_Socket_Send(Kinc_Socket *sock, unsigned address, int port, const unsigned char *data, int size) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(address);
	addr.sin_port = htons(port);

	size_t sent = sendto(sock->handle, (const char*)data, size, 0, (sockaddr*)&addr, sizeof(sockaddr_in));
	if (sent != size) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not send packet.");
	}
#endif
}

void Kinc_Socket_Send_URL(Kinc_Socket *sock, const char *url, int port, const unsigned char *data, int size) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	addrinfo* address = nullptr;
	int res = resolveAddress(url, port, &address);
	if (res != 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not resolve address.");
		return;
	}

	size_t sent = sendto(sock->handle, (const char*)data, size, 0, address->ai_addr, sizeof(sockaddr_in));
	if (sent != size) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not send packet.");
	}
	freeaddrinfo(address);
#endif
}

void Kinc_Socket_SetBroadcastEnabled(Kinc_Socket *sock, bool enabled) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	char broadcast = enabled ? 1 : 0;
	if (setsockopt(sock->handle, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not set broadcast mode.");
		return;
	}
#endif
}

int Kinc_Socket_Receive(Kinc_Socket *sock, unsigned char *data, int maxSize, unsigned *fromAddress, unsigned *fromPort) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	typedef int socklen_t;
	typedef int ssize_t;
#endif
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	sockaddr_in from;
	socklen_t fromLength = sizeof(from);
	ssize_t bytes = recvfrom(sock->handle, (char*)data, maxSize, 0, (sockaddr*)&from, &fromLength);
	if (bytes <= 0) return static_cast<int>(bytes);
	*fromAddress = ntohl(from.sin_addr.s_addr);
	*fromPort = ntohs(from.sin_port);
	return static_cast<int>(bytes);
#else
	return 0;
#endif
}
