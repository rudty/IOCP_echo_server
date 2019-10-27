#include <stdio.h>
#include "socket.h"
#pragma comment(lib, "ws2_32.lib")

// windows only
// cmd 
// telnet 127.0.0.1 8087 
//
// ��������
// 127.0.0.1:8087

void onDataReceived(ClientSocket* c, const char* buf, int len) {
	printf("%.*s\n", len, buf);
	socket_tcp_client_send(c, buf, len);
}

void onConnectionClosed(ClientSocket* c) {
	printf("disconnected\n");
}

void onSendComplection(ClientSocket* c) {
	printf("send fin\n");
}

void onClientAccept(ClientSocket* c) {
	printf("new client\n");

	//���� �Ϸ� ��
	socket_tcp_client_socket_set_send_complection_handler(c, onSendComplection);

	//���� ���� ��
	socket_tcp_client_socket_set_connection_close_handler(c, onConnectionClosed);

	//������ �Է� ��
	socket_tcp_client_socket_set_data_receive_handler(c, onDataReceived);
}

int main() {
	ServerSocket* s;
	socket_tcp_server_socket_initialize(&s);

	//Ŭ���̾�Ʈ �Է½� ȣ���� �Լ�
	socket_tcp_server_socket_set_accept_handler(s, onClientAccept);

	if (socket_tcp_server_socket_listen(s, 8087)) {
		printf("8087 is already use");
	}

}