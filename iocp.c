
#include <WinSock2.h>
#include <Windows.h>
#include "socket.h"
struct ClientSocket;

struct IocpOperator {
	/**
	 * overlapped는 항상 맨 위에 위치하게 할 것.
	 * WSA 함수에 overapped로 전달 시 
	 * 포인터만 변환해서 사용 가능함
	 */
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
	SOCKET socket;
	void* userArg;
	IOComplectionCallaback onComplectionCallback;
};


int socket_iocp_op_initialize(struct IocpOperator** p, SOCKET socket) {
	struct IocpOperator* iocpOp = (struct IocpOperator*) malloc(sizeof(struct IocpOperator));

	if (iocpOp == NULL) {
		return -1;
	}

	memset(iocpOp, 0, sizeof(struct IocpOperator));
	iocpOp->socket = socket;
	*p = iocpOp;
	return 0;
}

void socket_iocp_op_set_complection_callback(struct IocpOperator* p, IOComplectionCallaback onComplectionCallback, void* userArg) {
	p->onComplectionCallback = onComplectionCallback;
	p->userArg = userArg;
}

void socket_iocp_op_release(struct IocpOperator* p) {
	free(p);
}

int socket_iocp_send(struct IocpOperator* op, SOCKET sock, const char* buf, int len) {

	int sendBytes = 0;
	WSABUF* wsaBuf = &op->wsaBuf;
	wsaBuf->buf = (char*)buf;
	wsaBuf->len = len;

	if (WSASend(
		sock,
		wsaBuf,
		1,
		&sendBytes,
		0,
		(OVERLAPPED*)op,
		NULL) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING) {
			return err;
		}
	}

	return 0;
}

int socket_iocp_receive(struct IocpOperator* op, SOCKET sock, char* buf, int len) {
	DWORD flags = 0;

	WSABUF* wsaBuf = &op->wsaBuf;
	wsaBuf->buf = buf;
	wsaBuf->len = len;

	if (WSARecv(sock,
		&op->wsaBuf,
		1,
		NULL,
		&flags,
		(OVERLAPPED*)op,
		NULL) == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING) {
			return err;
		}
	}
	return 0;
}

void iocp_call_complection_callback(struct IocpOperator* op, int receiveBytes) {
	IOComplectionCallaback cb = op->onComplectionCallback;
	if (cb) {
		cb(op,
			op->wsaBuf.buf,
			receiveBytes,
			op->userArg);
	}
}

static DWORD WINAPI socket_tcp_client_socket_iocp_thread(LPVOID arg) {
	HANDLE iocp = (HANDLE)arg;
	OVERLAPPED_ENTRY entries[64];
	ULONG entriesCount;
	int i;
	while (1) {
		if (GetQueuedCompletionStatusEx(iocp, &entries[0], 64, &entriesCount, 0, 0)) {
			for (i = 0; i < entriesCount; ++i) {
				struct IocpOperator* op = (struct IocpOperator*)entries[i].lpOverlapped;
				iocp_call_complection_callback(op, entries[i].dwNumberOfBytesTransferred);
			}
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