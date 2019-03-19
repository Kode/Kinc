#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef KORE_WINDOWS
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

typedef struct {
#ifdef KORE_WINDOWS
	SOCKET handle;
#else
	int handle;
#endif
} Kinc_Socket;

void Kinc_Socket_Init(Kinc_Socket *socket);
void Kinc_Socket_Destroy(Kinc_Socket *socket);
void Kinc_Socket_Open(Kinc_Socket *socket, int port);
void Kinc_Socket_Send(Kinc_Socket *socket, unsigned address, int port, const unsigned char *data, int size);
void Kinc_Socket_Send_URL(Kinc_Socket *socket, const char *url, int port, const unsigned char *data, int size);
int Kinc_Socket_Receive(Kinc_Socket *socket, unsigned char *data, int maxSize, unsigned *fromAddress, unsigned *fromPort);
unsigned Kinc_urlToInt(const char *url, int port);

#ifdef __cplusplus
}
#endif
