#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9000
#define IP "127.0.0.1"
#define BUF_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char message[BUF_SIZE];
    int str_len;

    // 1. 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket error");
        exit(1);
    }

    // 2. 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port = htons(PORT);

    // 3. 서버로 연결 요청 (Connect)
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect error");
        exit(1);
    }
    printf("Connected to Server! (Type 'q' to quit)\n");

    // 4. 데이터 송수신
    while (1) {
        printf("Input message: ");
        fgets(message, BUF_SIZE, stdin);

        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
            break;

        write(sock, message, strlen(message)); // 데이터 전송
        str_len = read(sock, message, BUF_SIZE - 1); // 에코 데이터 수신
        message[str_len] = 0;
        printf("Message from Server: %s", message);
    }

    // 5. 종료
    close(sock);
    return 0;
}