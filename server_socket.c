#include<WinSock2.h>
#include "socket.h"

struct ClientSocket;

struct ServerSocket {
	SOCKET sock;
	ClientSocketHandler onAccept;
	HANDLE iocp;
};

/**
 * 클라이언트 소켓을 초기화합니다.
 * 일반적으로는 서버 소켓 내부에서 사용해서 호출할 필요가 없습니다
 */
int socket_tcp_client_socket_initialize(struct ClientSocket** c, void* sock, void* sockAddrIn);

/**
 * 클라이언트에 비동기로 내용을 받습니다
 * 일반적으로는 내부에서 사용해서 호출할 필요가 없습니다
 * 한번에 최대 60000의 길이까지 받을 수 있습니다
 * @return 0: 정상, -1: 메모리 할당 실패
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
 * 서버 소켓을 초기화합니다.
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
 * 서버 소켓을 해제합니다.
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
 * 서버 소켓을 클라이언트로부터 응답을 받을 수 있도록 준비합니다
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
 * 서버 소켓에 client가 들어올 시 호출할 함수를 지정합니다.
 */
void socket_tcp_server_socket_set_accept_handler(struct ServerSocket* s, AcceptHandler h) {
	s->onAccept = h;
}
