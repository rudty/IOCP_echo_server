#include <stdio.h>
#include "socket.h"
#pragma comment(lib, "ws2_32.lib")

// windows only
// cmd 
// telnet 127.0.0.1 8087 
//
// 웹브라우저
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

	//전송 완료 시
	socket_tcp_client_socket_set_send_complection_handler(c, onSendComplection);

	//연결 종료 시
	socket_tcp_client_socket_set_connection_close_handler(c, onConnectionClosed);

	//데이터 입력 시
	socket_tcp_client_socket_set_data_receive_handler(c, onDataReceived);
}

int main() {
	ServerSocket* s;
	socket_tcp_server_socket_initialize(&s);

	//클라이언트 입력시 호출할 함수
	socket_tcp_server_socket_set_accept_handler(s, onClientAccept);

	if (socket_tcp_server_socket_listen(s, 8087)) {
		printf("8087 is already use");
	}

}