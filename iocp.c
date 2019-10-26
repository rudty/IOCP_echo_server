
#include <WinSock2.h>
#include <Windows.h>
#include "socket.h"
struct ClientSocket;

struct IocpPacket {
	/**
	 * overlapped는 항상 맨 위에 위치하게 할 것.
	 * WSA 함수에 overapped로 전달 시 
	 * 포인터만 변환해서 사용 가능함
	 */
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	void* userArg;
	IOComplectionCallaback onComplectionCallback;
};


int socket_iocp_packet_initialize(struct IocpPacket** p) {
	struct IocpPacket* iocpPacket = (struct IocpPacket*) malloc(sizeof(struct IocpPacket));

	if (iocpPacket == NULL) {
		return -1;
	}

	memset(iocpPacket, 0, sizeof(struct IocpPacket));
	*p = iocpPacket;
	return 0;
}

void socket_iocp_packet_set_complection_callback(struct IocpPacket* p, IOComplectionCallaback onComplectionCallback, void* userArg) {
	p->onComplectionCallback = onComplectionCallback;
	p->userArg = userArg;
}

void socket_iocp_packet_release(struct IocpPacket* p) {
	free(p);
}

int socket_iocp_send(struct IocpPacket* packet, SOCKET sock, const char* buf, int len) {

	int sendBytes = 0;
	WSABUF* wsaBuf = &packet->wsaBuf;
	wsaBuf->buf = (char*)buf;
	wsaBuf->len = len;

	if (WSASend(
		sock,
		wsaBuf,
		1,
		&sendBytes,
		0,
		(OVERLAPPED*)packet,
		NULL) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING) {
			return err;
		}
	}

	return 0;
}

int socket_iocp_receive(struct IocpPacket* packet, SOCKET sock, char* buf, int len) {
	DWORD flags = 0;

	WSABUF* wsaBuf = &packet->wsaBuf;
	wsaBuf->buf = buf;
	wsaBuf->len = len;

	if (WSARecv(sock,
		&packet->wsaBuf,
		1,
		NULL,
		&flags,
		(OVERLAPPED*)packet,
		NULL) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING) {
			return err;
		}
	}
	return 0;
}

void iocp_call_complection_callback(struct IocpPacket* packet, int receiveBytes) {
	IOComplectionCallaback cb = packet->onComplectionCallback;
	if (cb) {
		cb(packet,
			packet->wsaBuf.buf,
			receiveBytes,
			packet->userArg);
	}
}

static DWORD WINAPI socket_tcp_client_socket_iocp_thread(LPVOID arg) {
	HANDLE iocp = (HANDLE)arg;
	DWORD receiveBytes;
	struct IocpPacket* packet;
	ULONG_PTR context;
	BOOL ret;
	while (1) {
		packet = NULL;
		ret = GetQueuedCompletionStatus(iocp, &receiveBytes, &context, (OVERLAPPED**)&packet, INFINITE);
		if (ret) {
			if (packet == NULL) {
				continue;
			}

			iocp_call_complection_callback(packet, receiveBytes);
		}
		
	}
	return 0;
}

int socket_iocp_start_complection_thread(void* iocp, int count) {
	int i;
	HANDLE h;
	for (i = 0; i < count; ++i) {
		h = CreateThread(
			NULL,
			0,
			socket_tcp_client_socket_iocp_thread,
			iocp,
			0,
			NULL);

		if (h == NULL) {
			return -1;
		}

		CloseHandle(h);
	}
	return 0;
}