#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define PORT 9002
#define MAX_CLIENTS 10
#define BUF_SIZE 1024

// 서버 상태를 관리하는 구조체
typedef struct {
    int server_sock;
    int client_sockets[MAX_CLIENTS]; // 접속한 클라이언트 소켓 목록
    int max_fd;                      // select 감시용 최대 파일 디스크립터 번호
    fd_set read_fds;                 // 읽기 감시 대상 소켓 집합
} ChatServer;

ChatServer server;

void error_handling(char *message) {
    perror(message);
    exit(1);
}

// 모든 클라이언트에게 메시지 전송 (나 자신 제외 옵션 가능)
void broadcast_message(char *msg, int len, int sender_sock) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int client = server.client_sockets[i];
        // 연결된 클라이언트이고, 보낸 사람이 아니라면 전송
        if (client != 0 && client != sender_sock) {
            write(client, msg, len);
        }
    }
}

int main() {
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t adr_sz;
    int str_len, i;
    char buf[BUF_SIZE];

    // 1. 초기화
    for (i = 0; i < MAX_CLIENTS; i++) server.client_sockets[i] = 0;
    
    server.server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (server.server_sock == -1) error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(PORT);

    if (bind(server.server_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    
    if (listen(server.server_sock, 5) == -1)
        error_handling("listen() error");

    printf("--- Chat Server Started on Port %d ---\n", PORT);

    // 2. I/O 멀티플렉싱 루프
    while (1) {
        FD_ZERO(&server.read_fds);            // 감시 목록 초기화
        FD_SET(server.server_sock, &server.read_fds); // 서버 소켓(접속 요청) 감시 등록
        server.max_fd = server.server_sock;

        // 연결된 클라이언트 소켓들도 감시 목록에 추가
        for (i = 0; i < MAX_CLIENTS; i++) {
            int clnt_sock = server.client_sockets[i];
            if (clnt_sock > 0) {
                FD_SET(clnt_sock, &server.read_fds);
            }
            if (clnt_sock > server.max_fd) {
                server.max_fd = clnt_sock;
            }
        }

        // 3. 변화 감지 (Select)
        // 변화가 생길 때까지 대기 (블로킹)
        if (select(server.max_fd + 1, &server.read_fds, NULL, NULL, NULL) == -1)
            break;

        // 4. 변화 처리
        
        // Case A: 새로운 접속 요청이 들어온 경우
        if (FD_ISSET(server.server_sock, &server.read_fds)) {
            adr_sz = sizeof(clnt_adr);
            int new_sock = accept(server.server_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
            
            // 빈 자리에 클라이언트 추가
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (server.client_sockets[i] == 0) {
                    server.client_sockets[i] = new_sock;
                    printf("New Client Connected: %d\n", new_sock);
                    break;
                }
            }
        }

        // Case B: 기존 클라이언트가 메시지를 보낸 경우
        for (i = 0; i < MAX_CLIENTS; i++) {
            int clnt_sock = server.client_sockets[i];
            if (clnt_sock > 0 && FD_ISSET(clnt_sock, &server.read_fds)) {
                str_len = read(clnt_sock, buf, BUF_SIZE);
                
                if (str_len == 0) { // 연결 종료 요청 (EOF)
                    printf("Client Disconnected: %d\n", clnt_sock);
                    close(clnt_sock);
                    server.client_sockets[i] = 0;
                } else {
                    // 메시지 방송 (Broadcasting)
                    // 보낸 사람 포함 모든 사람에게 전송하려면 sender_sock에 0을 넣으세요.
                    broadcast_message(buf, str_len, 0); 
                }
            }
        }
    }

    close(server.server_sock);
    return 0;
}