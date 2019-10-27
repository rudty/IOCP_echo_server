# IOCP_echo_server

iocp 활용해서 멀티 스레드 echo 서버

서버 소켓과 클라이언트 소켓을 따로 작성

### 서버소켓
accept 만 담당

- 예제용으로 스레드 개수는 일단 4개로 작성ㄴ

---

### 클라이언트 소켓
send/receive 담당

- send: 함수로 직접 호출
- receive: callback 형태로 callback 함수를 작성

