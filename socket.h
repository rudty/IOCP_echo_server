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
 * ���� ������ �ʱ�ȭ�մϴ�.
 * @return 0: ����, -1: �޸� �Ҵ� ����, -2: ���� �ʱ�ȭ ����, -3: ������ ���� ����
 */
int socket_tcp_server_socket_initialize(struct ServerSocket** s);

/**
 * ���� ������ �����մϴ�.
 */
void socket_tcp_server_socket_release(struct ServerSocket* s);

/**
 * ���� ������ Ŭ���̾�Ʈ�κ��� ������ ���� �� �ֵ��� �غ��մϴ�
 * �� �Լ��� ���ѷ����Դϴ�
 * bind + listen + accept
 * @return -1: bind ����, -2: listen ����
 */
int socket_tcp_server_socket_listen(struct ServerSocket* s, short port);

/****************************************************************************
 * server socket handler
 ****************************************************************************/

/**
 * Ŭ���̾�Ʈ�� ����ɶ����� ȣ���� �Լ��� ����մϴ�
 */
void socket_tcp_server_socket_set_accept_handler(struct ServerSocket* s, AcceptHandler h);


typedef void(*ClientSocketHandler)(struct ClientSocket*);
typedef void(*DataReceiveHandler)(struct ClientSocket*, const char* buf, int len);

/****************************************************************************
 * client socket function
 ****************************************************************************/

/**
 * Ŭ���̾�Ʈ�� �񵿱�� ������ �����ϴ�
 * ���������� �����Ͽ� �ѹ� return �Ͽ��ٸ� ���ڷ� �ѱ� buf�� ������ �ʿ�� �����ϴ�
 * @return 0: ����, -1: �޸� �Ҵ� ����, �׿�: WSAGetLastError
 */
int socket_tcp_client_send(struct ClientSocket* clientSocket, const char* buf, int len);

/****************************************************************************
 * client socket handler
 ****************************************************************************/

/**
 * Ŭ���̾�Ʈ�κ��� �����͸� ���������� ȣ���� �Լ��� ����մϴ�
 */
void socket_tcp_client_socket_set_data_receive_handler(struct ClientSocket* c, DataReceiveHandler h);

/**
 * Ŭ���̾�Ʈ�� ������ ���������� ȣ���� �Լ��� ����մϴ�
 */
void socket_tcp_client_socket_set_connection_close_handler(struct ClientSocket* c, ClientSocketHandler h);

/**
 * Ŭ���̾�Ʈ�� �޼����� �������� ȣ���� �Լ��� ����մϴ�
 */
void socket_tcp_client_socket_set_send_complection_handler(struct ClientSocket* c, ClientSocketHandler h);

#ifdef __cplusplus
}
#endif