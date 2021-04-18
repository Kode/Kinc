#include "pch.h"

#include "socket.h"

#include <kinc/log.h>

#include <stdbool.h>
#include <stdio.h>

#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
#include <Ws2tcpip.h>
#include <winsock2.h>
#elif defined(KORE_POSIX)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
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
static int resolveAddress(const char *url, int port, struct addrinfo **result) {
	struct addrinfo hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	char serv[6];
	sprintf(serv, "%u", port);

	return getaddrinfo(url, serv, &hints, result);
}
#endif

KINC_FUNC void kinc_socket_options_set_defaults(kinc_socket_options_t *options) {
	options->non_blocking = true;
	options->broadcast = false;
	options->tcp_no_delay = false;
}

void kinc_socket_init(kinc_socket_t *sock) {
	if (initialized) return;

	sock->handle = 0;
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);
#endif
	initialized = true;
}

bool kinc_socket_open(kinc_socket_t *sock, kinc_socket_protocol_t protocol, int port, struct kinc_socket_options *options) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	switch (protocol) {
	case KINC_SOCKET_PROTOCOL_UDP:
		sock->handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		break;
	case KINC_SOCKET_PROTOCOL_TCP:
		sock->handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		break;
	default:
		kinc_log(KINC_LOG_LEVEL_ERROR, "Unsupported socket protocol.");
		return false;
	}

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
		return false;
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((unsigned short)port);
	if (bind(sock->handle, (const struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not bind socket.");
		return false;
	}
#endif

	if (options) {
		if (options->non_blocking) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
			DWORD value = 1;
			if (ioctlsocket(sock->handle, FIONBIO, &value) != 0) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Could not set non-blocking mode.");
				return false;
			}
#elif defined(KORE_POSIX)
			int value = 1;
			if (fcntl(sock->handle, F_SETFL, O_NONBLOCK, value) == -1) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Could not set non-blocking mode.");
				return false;
			}
#endif
		}

		if (options->broadcast) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
			char value = 1;
			if (setsockopt(sock->handle, SOL_SOCKET, SO_BROADCAST, &value, sizeof(value)) < 0) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Could not set broadcast mode.");
				return false;
			}
#endif
		}

		if (options->tcp_no_delay) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
			int value = 1;
			if (setsockopt(sock->handle, IPPROTO_TCP, TCP_NODELAY, (const char *)&value, sizeof(value)) != 0) {
				kinc_log(KINC_LOG_LEVEL_ERROR, "Could not set no-delay mode.");
				return false;
			}
#endif
		}
	}

	return true;
}

void kinc_socket_destroy(kinc_socket_t *sock) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	closesocket(sock->handle);
#elif defined(KORE_POSIX)
	close(sock->handle);
#endif
	destroy();
	initialized = false;
}

unsigned kinc_url_to_int(const char *url, int port) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	struct addrinfo *address = NULL;
	int res = resolveAddress(url, port, &address);
	if (res != 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not resolve address.");
		return -1;
	}

	unsigned fromAddress = ntohl(((struct sockaddr_in *)address->ai_addr)->sin_addr.S_un.S_addr);
	freeaddrinfo(address);

	return fromAddress;
#else
	return 0;
#endif
}

bool kinc_socket_listen(kinc_socket_t *socket, int backlog) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	int res = listen(socket->handle, backlog);
	return (res == 0);
#endif
	return false;
}

bool kinc_socket_accept(kinc_socket_t *socket, kinc_socket_t *newSocket, unsigned *remoteAddress, unsigned *remotePort) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	typedef int socklen_t;
#endif
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	struct sockaddr_in addr;
	socklen_t addrLength = sizeof(addr);
	newSocket->handle = accept(socket->handle, (struct sockaddr *)&addr, &addrLength);
	if (newSocket->handle <= 0) {
		return false;
	}

	*remoteAddress = ntohl(addr.sin_addr.s_addr);
	*remotePort = ntohs(addr.sin_port);
	return true;
#else
	return false;
#endif
}

bool kinc_socket_connect(kinc_socket_t *socket, unsigned address, int port) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(address);
	addr.sin_port = htons(port);

	int res = connect(socket->handle, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	return (res == 0);
#endif
	return false;
}

int kinc_socket_send(kinc_socket_t *sock, unsigned address, int port, const unsigned char *data, int size) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(address);
	addr.sin_port = htons(port);

	size_t sent = sendto(sock->handle, (const char *)data, size, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (sent != size) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not send packet.");
	}
	return (int)sent;
#endif
	return 0;
}

int kinc_socket_send_url(kinc_socket_t *sock, const char *url, int port, const unsigned char *data, int size) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	struct addrinfo *address = NULL;
	int res = resolveAddress(url, port, &address);
	if (res != 0) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not resolve address.");
		return 0;
	}

	size_t sent = sendto(sock->handle, (const char *)data, size, 0, address->ai_addr, sizeof(struct sockaddr_in));
	if (sent != size) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not send packet.");
	}
	freeaddrinfo(address);
	return (int)sent;
#endif
	return 0;
}

int kinc_socket_send_connected(kinc_socket_t *sock, const unsigned char *data, int size) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	size_t sent = send(sock->handle, (const char *)data, size, 0);
	if (sent != size) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "Could not send packet.");
	}
	return (int)sent;
#endif
	return 0;
}

int kinc_socket_receive(kinc_socket_t *sock, unsigned char *data, int maxSize, unsigned *fromAddress, unsigned *fromPort) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	typedef int socklen_t;
	typedef int ssize_t;
#endif
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	struct sockaddr_in from;
	socklen_t fromLength = sizeof(from);
	ssize_t bytes = recvfrom(sock->handle, (char *)data, maxSize, 0, (struct sockaddr *)&from, &fromLength);
	if (bytes <= 0) {
		return (int)bytes;
	}
	*fromAddress = ntohl(from.sin_addr.s_addr);
	*fromPort = ntohs(from.sin_port);
	return (int)bytes;
#else
	return 0;
#endif
}

int kinc_socket_receive_connected(kinc_socket_t *sock, unsigned char *data, int maxSize) {
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP)
	typedef int ssize_t;
#endif
#if defined(KORE_WINDOWS) || defined(KORE_WINDOWSAPP) || defined(KORE_POSIX)
	ssize_t bytes = recv(sock->handle, (char *)data, maxSize, 0);
	return (int)bytes;
#else
	return 0;
#endif
}
