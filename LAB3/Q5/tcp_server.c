#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9000
#define BUF_SIZE 1024

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    char buffer[BUF_SIZE];
    int str_len;

    // 1. 소켓 생성 (PF_INET: IPv4, SOCK_STREAM: TCP)
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket error");
        exit(1);
    }

    // 2. 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 내 IP 주소 자동 할당
    server_addr.sin_port = htons(PORT);

    // 3. 소켓에 주소 할당 (Bind)
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind error");
        exit(1);
    }

    // 4. 연결 대기 상태 진입 (Listen)
    if (listen(server_sock, 5) == -1) {
        perror("listen error");
        exit(1);
    }

    printf("--- TCP Server Started (Port: %d) ---\n", PORT);

    // 5. 클라이언트 연결 수락 (Accept) - 연결될 때까지 대기함
    client_addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
    if (client_sock == -1) {
        perror("accept error");
        exit(1);
    } else {
        printf("Client Connected!\n");
    }

    // 6. 데이터 송수신 (Echo)
    while ((str_len = read(client_sock, buffer, BUF_SIZE)) != 0) {
        write(client_sock, buffer, str_len); // 받은 데이터를 그대로 다시 보냄
        buffer[str_len] = 0; // 문자열 끝 처리
        printf("Received & Echoed: %s", buffer);
    }

    // 7. 연결 종료
    close(client_sock);
    close(server_sock);
    return 0;
}