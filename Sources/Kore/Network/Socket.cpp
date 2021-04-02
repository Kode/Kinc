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
	kinc_socket_init(&sock);
}

bool Socket::open(kinc_socket_protocol_t protocol, int port, bool blocking) {
	return kinc_socket_open(&sock, protocol, port, blocking);
}

Socket::~Socket() {
	kinc_socket_destroy(&sock);
}

unsigned Socket::urlToInt(const char* url, int port) {
	return kinc_url_to_int(url, port);
}

bool Socket::listen(int backlog) {
	return kinc_socket_listen(&sock, backlog);
}

bool Socket::accept(Socket* newSocket, unsigned* remoteAddress, unsigned* remotePort) {
	return kinc_socket_accept(&sock, &(newSocket->sock), remoteAddress, remotePort);
}

bool Socket::connect(unsigned address, int port) {
	return kinc_socket_connect(&sock, address, port);
}

void Socket::send(unsigned address, int port, const u8 *data, int size) {
	kinc_socket_send(&sock, address, port, data, size);

}

void Socket::setBroadcastEnabled(bool enabled) {
	kinc_socket_set_broadcast_enabled(&sock, enabled);
}

void Socket::send(const char *url, int port, const u8 *data, int size) {
	kinc_socket_send_url(&sock, url, port, data, size);
}

void Socket::send(const u8 *data, int size) {
	kinc_socket_send_connected(&sock, data, size);
}

int Socket::receive(u8* data, int maxSize, unsigned& fromAddress, unsigned& fromPort) {
	return kinc_socket_receive(&sock, data, maxSize, &fromAddress, &fromPort);
}

int Socket::receive(u8* data, int maxSize) {
	return kinc_socket_receive_connected(&sock, data, maxSize);
}
