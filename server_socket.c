#include<WinSock2.h>
#include "socket.h"

struct ClientSocket;

struct ServerSocket {
	SOCKET sock;
	ClientSocketHandler onAccept;
	HANDLE iocp;
};

/**
 * Ŭ���̾�Ʈ ������ �ʱ�ȭ�մϴ�.
 * �Ϲ������δ� ���� ���� ���ο��� ����ؼ� ȣ���� �ʿ䰡 �����ϴ�
 */
int socket_tcp_client_socket_initialize(struct ClientSocket** c, void* sock, void* sockAddrIn);

/**
 * Ŭ���̾�Ʈ�� �񵿱�� ������ �޽��ϴ�
 * �Ϲ������δ� ���ο��� ����ؼ� ȣ���� �ʿ䰡 �����ϴ�
 * �ѹ��� �ִ� 60000�� ���̱��� ���� �� �ֽ��ϴ�
 * @return 0: ����, -1: �޸� �Ҵ� ����
 */
int socket_tcp_client_receive(struct ClientSocket* clientSocket);


int socket_iocp_start_complection_thread(void* iocp, int count);

int socket_wsa_initialize() {
	WSADATA w;
	return WSAStartup(MAKEWORD(2, 2), &w);
}

int socket_tcp_socket(SOCKET* sock) {
	SOCKET s;
	socket_wsa_initialize();

	s = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (s == INVALID_SOCKET) {
		return -1;
	}

	*sock = s;

	return 0;
}

/**
 * ���� ������ �ʱ�ȭ�մϴ�.
 */
int socket_tcp_server_socket_initialize(struct ServerSocket** s) {
	
	struct ServerSocket* serverSocket = (struct ServerSocket*)malloc(sizeof(struct ServerSocket));

	if (serverSocket == NULL) {
		return -1;
	}

	if (socket_tcp_socket(&serverSocket->sock)) {
		return -2;
	}
	
	serverSocket->iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (socket_iocp_start_complection_thread(serverSocket->iocp, 4)) {
		return -3;
	}

	*s = serverSocket;

	return 0;
}

/**
 * ���� ������ �����մϴ�.
 */
void socket_tcp_server_socket_release(struct ServerSocket* s) {
	closesocket(s->sock);
	free(s);
	WSACleanup();
}

int socket_tcp_serversocket_accept_client(struct ServerSocket* serverSocket) {

	SOCKADDR_IN addr;
	struct ClientSocket* clientSocket;
	while (1) {
		int addrLen = sizeof(SOCKADDR_IN);
		SOCKET sock = accept(serverSocket->sock, (SOCKADDR*)&addr, &addrLen);

		if (sock == INVALID_SOCKET) {
			continue;
		}

		if (!CreateIoCompletionPort((HANDLE)sock, serverSocket->iocp, 0, 0)) {
			closesocket(sock);
			continue;
		}
		
		socket_tcp_client_socket_initialize(&clientSocket, (void*)sock, &addr);
		
		if (serverSocket->onAccept != NULL) {
			serverSocket->onAccept(clientSocket);
		}
		
		socket_tcp_client_receive(clientSocket);
	}
}

/**
 * ���� ������ Ŭ���̾�Ʈ�κ��� ������ ���� �� �ֵ��� �غ��մϴ�
 * bind + listen + accept
 */
int socket_tcp_server_socket_listen(struct ServerSocket* s, short port) {
	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(SOCKADDR_IN));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(s->sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
		return -1;
	}

	if (listen(s->sock, 5)) {
		return -2;
	}

	socket_tcp_serversocket_accept_client(s);

	return 0;
}

/**
 * ���� ���Ͽ� client�� ���� �� ȣ���� �Լ��� �����մϴ�.
 */
void socket_tcp_server_socket_set_accept_handler(struct ServerSocket* s, AcceptHandler h) {
	s->onAccept = h;
}
