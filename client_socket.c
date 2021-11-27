#include<WinSock2.h>
#include "socket.h"
#define DATA_SIZE 60000

struct ClientSocket {
	SOCKET sock;
	SOCKADDR_IN addr;

	DataReceiveHandler onDataReceiveHandler;
	ClientSocketHandler onConnectionCloseHandler;
	ClientSocketHandler onSendComplectionHandler;
};

int socket_iocp_op_initialize(struct Poll** p, SOCKET socket);
int socket_iocp_send(struct Poll* op, SOCKET sock, const char* buf, int len);
int socket_iocp_receive(struct Poll* op, SOCKET sock, char* buf, int len);
void socket_iocp_op_release(struct Poll* p);
void socket_iocp_op_set_complection_callback(struct Poll* p, IOComplectionCallaback onComplectionCallback, void* userArg);

int socket_tcp_client_socket_initialize(struct ClientSocket** c, void* sock, void* sockAddrIn) {
	struct ClientSocket* clientSocket = (struct ClientSocket*)malloc(sizeof(struct ClientSocket));
	if (clientSocket == NULL) {
		return -1;
	}

	memset(clientSocket, 0, sizeof(struct ClientSocket));
	clientSocket->sock = (SOCKET)sock;
	memcpy(&clientSocket->addr, sockAddrIn, sizeof(SOCKADDR_IN));

	*c = clientSocket;

	return 0;
}

/**
 * Ŭ���̾�Ʈ�� send �۾��� ���� �� ȣ���� handler�� ����մϴ�
 */
void socket_tcp_client_socket_set_send_complection_handler(struct ClientSocket* c, ClientSocketHandler h) {
	c->onSendComplectionHandler = h;
}

/**
 * Ŭ���̾�Ʈ�κ��� receive �۾��� ���� �� ȣ���� handler�� ����մϴ�
 */
void socket_tcp_client_socket_set_data_receive_handler(struct ClientSocket* c, DataReceiveHandler h) {
	c->onDataReceiveHandler = h;
}

/**
 * Ŭ���̾�Ʈ�� ������ �������� ȣ���� handler�� ����մϴ�
 */
void socket_tcp_client_socket_set_connection_close_handler(struct ClientSocket* c, ClientSocketHandler h) {
	c->onConnectionCloseHandler = h;
}

static void socket_tcp_client_send_callback(struct Poll* op, char* buf, int len, void* arg) {
	struct ClientSocket* clientSocket = (struct ClientSocket*)arg;
	ClientSocketHandler sendHandler = clientSocket->onSendComplectionHandler;
	if (sendHandler != NULL) {
		sendHandler(clientSocket);
	}
	free(buf);
	socket_iocp_op_release(op);
}

int socket_tcp_client_send(struct ClientSocket* clientSocket, const char* buf, int len) {
 	char* cpyBuf = (char*)malloc(len);
	struct Poll* op;

	if (cpyBuf == NULL) {
		return -1;
	}

	memcpy(cpyBuf, buf, len);

	if (socket_iocp_op_initialize(&op, clientSocket->sock)) {
		return -1;
	}
	
	socket_iocp_op_set_complection_callback(op, socket_tcp_client_send_callback, clientSocket);
	return socket_iocp_send(op, clientSocket->sock, cpyBuf, len);
}

/**
 * Ŭ���̾�Ʈ ������ �����մϴ�.
 * �Ϲ������δ� ���ο��� ����ؼ� ȣ���� �ʿ䰡 �����ϴ�
 */
static void socket_tcp_client_socket_release(struct ClientSocket* c) {
	closesocket(c->sock);
	free(c);
}

static void socket_tcp_client_receive_callback(struct Poll* op, char* buf, int len, void* arg) {
	struct ClientSocket* clientSocket = (struct ClientSocket*)arg;
	if (len <= 0) {
		//������ ����
		ClientSocketHandler closeHandler = clientSocket->onConnectionCloseHandler;
		if (closeHandler) {
			closeHandler(clientSocket);
		}
		socket_iocp_op_release(op);
		socket_tcp_client_socket_release(clientSocket);
		free(buf);
	} else {
		DataReceiveHandler h = clientSocket->onDataReceiveHandler;
		if (h != NULL) {
			h(clientSocket, buf, len);
		}
		socket_iocp_receive(op, clientSocket->sock, buf, DATA_SIZE);
	}
}

/**
 * Ŭ���̾�Ʈ ���Ͽ��� ������ �޽��ϴ�.
 * �� �Լ��� �ڵ����� ȣ��˴ϴ�
 */
int socket_tcp_client_receive(struct ClientSocket* clientSocket) {
	struct Poll* op;
	char* receiveBuf;

	socket_iocp_op_initialize(&op, clientSocket->sock);

	receiveBuf = (char*)malloc(DATA_SIZE);
	if (receiveBuf == NULL) {
		return -1;
	}

	socket_iocp_op_set_complection_callback(op, socket_tcp_client_receive_callback, clientSocket);

	return socket_iocp_receive(op, clientSocket->sock, receiveBuf, DATA_SIZE);
}