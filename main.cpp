#include <stdio.h>
#include <string>
#include <iostream>
#include "socket.h"
#pragma comment(lib, "ws2_32.lib")

// windows only
// cmd 
// telnet 127.0.0.1 8087 
//
// 웹브라우저
// 127.0.0.1:8087

int main() {
	ServerSocket* s;
	socket_tcp_server_socket_initialize(&s);
	socket_tcp_server_socket_set_accept_handler(s, [](ClientSocket* c) {
		printf("new client\n");
		
		socket_tcp_client_socket_set_send_complection_handler(c, [](ClientSocket* c) {
			printf("send finish\n");
		});

		socket_tcp_client_socket_set_connection_close_handler(c, [](ClientSocket* c) {
			printf("disconnected\n");
		});

		socket_tcp_client_socket_set_data_receive_handler(c, [](ClientSocket* c, const char* buf, int len) {
			std::string str(buf, len);
			std::cout << str << std::endl;
			socket_tcp_client_send(c, buf, len);
		});
	});

	if (socket_tcp_server_socket_listen(s, 8087)) {
		printf("8087 is already use");
	}

}