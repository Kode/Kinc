#pragma once

#include <kinc/global.h>

/*! \file socket.h
    \brief Provides low-level network-communication via UDP or TCP-sockets.
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kinc_socket_protocol { KINC_SOCKET_PROTOCOL_UDP, KINC_SOCKET_PROTOCOL_TCP } kinc_socket_protocol_t;

#ifdef KORE_MICROSOFT
#if defined(_WIN64)
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;
#else
#if !defined _W64
#define _W64
#endif
typedef _W64 unsigned int UINT_PTR, *PUINT_PTR;
#endif
typedef UINT_PTR SOCKET;
#endif

typedef struct kinc_socket {
#ifdef KORE_MICROSOFT
	SOCKET handle;
#else
	int handle;
#endif
} kinc_socket_t;

typedef struct kinc_socket_options {
	bool non_blocking;
	bool broadcast;
	bool tcp_no_delay;
} kinc_socket_options_t;

/// <summary>
/// Initializes a socket-options-object to the default options
/// </summary>
/// <param name="options">The new default options</param>
KINC_FUNC void kinc_socket_options_set_defaults(kinc_socket_options_t *options);

/// <summary>
/// Initializes a socket-object.
/// </summary>
/// <param name="socket">The socket to initialize</param>
KINC_FUNC void kinc_socket_init(kinc_socket_t *socket);

/// <summary>
/// Destroys a socket-object.
/// </summary>
/// <param name="socket">The socket to destroy</param>
KINC_FUNC void kinc_socket_destroy(kinc_socket_t *socket);

/// <summary>
/// Opens a socket-connection.
/// </summary>
/// <param name="socket">The socket-object to use</param>
/// <param name="protocol">The protocol to use</param>
/// <param name="port">The port to use</param>
/// <param name="options">The options to use</param>
/// <returns>Whether the socket-connection could be opened</returns>
KINC_FUNC bool kinc_socket_open(kinc_socket_t *socket, kinc_socket_protocol_t protocol, int port, struct kinc_socket_options *options);

KINC_FUNC bool kinc_socket_listen(kinc_socket_t *socket, int backlog);
KINC_FUNC bool kinc_socket_accept(kinc_socket_t *socket, kinc_socket_t *new_socket, unsigned *remote_address, unsigned *remote_port);
KINC_FUNC bool kinc_socket_connect(kinc_socket_t *socket, unsigned address, int port);
KINC_FUNC int kinc_socket_send(kinc_socket_t *socket, unsigned address, int port, const unsigned char *data, int size);
KINC_FUNC int kinc_socket_send_url(kinc_socket_t *socket, const char *url, int port, const unsigned char *data, int size);
KINC_FUNC int kinc_socket_send_connected(kinc_socket_t *socket, const unsigned char *data, int size);
KINC_FUNC int kinc_socket_receive(kinc_socket_t *socket, unsigned char *data, int maxSize, unsigned *from_address, unsigned *from_port);
KINC_FUNC int kinc_socket_receive_connected(kinc_socket_t *socket, unsigned char *data, int max_size);

/// <summary>
/// Resolves a DNS-entry to an IP and returns its integer represenation.
/// </summary>
/// <param name="url"></param>
/// <param name="port"></param>
/// <returns></returns>
KINC_FUNC unsigned kinc_url_to_int(const char *url, int port);

#ifdef __cplusplus
}
#endif
