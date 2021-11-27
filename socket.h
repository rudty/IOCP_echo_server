#pragma once
#ifdef __cplusplus
extern "C" {
#endif

struct Poll;
typedef void(*IOComplectionCallaback)(struct Poll* p, char* buf, int len, void* arg);

struct ServerSocket;
struct ClientSocket;

typedef void(*AcceptHandler)(struct ClientSocket*);


/****************************************************************************
 * server socket function
 ****************************************************************************/

/**
 * 서버 소켓을 초기화합니다.
 * @return 0: 정상, -1: 메모리 할당 에러, -2: 소켓 초기화 에러, -3: 스레드 생성 에러
 */
int socket_tcp_server_socket_initialize(struct ServerSocket** s);

/**
 * 서버 소켓을 해제합니다.
 */
void socket_tcp_server_socket_release(struct ServerSocket* s);

/**
 * 서버 소켓을 클라이언트로부터 응답을 받을 수 있도록 준비합니다
 * 이 함수는 무한루프입니다
 * bind + listen + accept
 * @return -1: bind 에러, -2: listen 에러
 */
int socket_tcp_server_socket_listen(struct ServerSocket* s, short port);

/****************************************************************************
 * server socket handler
 ****************************************************************************/

/**
 * 클라이언트와 연결될때마다 호출할 함수를 등록합니다
 */
void socket_tcp_server_socket_set_accept_handler(struct ServerSocket* s, AcceptHandler h);


typedef void(*ClientSocketHandler)(struct ClientSocket*);
typedef void(*DataReceiveHandler)(struct ClientSocket*, const char* buf, int len);

/****************************************************************************
 * client socket function
 ****************************************************************************/

/**
 * 클라이언트에 비동기로 내용을 보냅니다
 * 내부적으로 복사하여 한번 return 하였다면 인자로 넘긴 buf를 유지할 필요는 없습니다
 * @return 0: 정상, -1: 메모리 할당 실패, 그외: WSAGetLastError
 */
int socket_tcp_client_send(struct ClientSocket* clientSocket, const char* buf, int len);

/****************************************************************************
 * client socket handler
 ****************************************************************************/

/**
 * 클라이언트로부터 데이터를 받을때마다 호출할 함수를 등록합니다
 */
void socket_tcp_client_socket_set_data_receive_handler(struct ClientSocket* c, DataReceiveHandler h);

/**
 * 클라이언트와 연결이 끊어졌을때 호출할 함수를 등록합니다
 */
void socket_tcp_client_socket_set_connection_close_handler(struct ClientSocket* c, ClientSocketHandler h);

/**
 * 클라이언트에 메세지를 보냈을때 호출할 함수를 등록합니다
 */
void socket_tcp_client_socket_set_send_complection_handler(struct ClientSocket* c, ClientSocketHandler h);

#ifdef __cplusplus
}
#endif