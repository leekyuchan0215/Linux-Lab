#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9001
#define IP "127.0.0.1"
#define BUF_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr, from_addr;
    socklen_t addr_size;
    char message[BUF_SIZE];
    int str_len;

    // 1. 소켓 생성 (UDP)
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket error");
        exit(1);
    }

    // 2. 서버 주소 설정 (UDP는 connect 과정 불필요)
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port = htons(PORT);

    printf("UDP Client Started (Type 'q' to quit)\n");

    while (1) {
        printf("Input message: ");
        fgets(message, BUF_SIZE, stdin);

        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
            break;

        // 3. 데이터 전송 (sendto)
        sendto(sock, message, strlen(message), 0, 
               (struct sockaddr*)&server_addr, sizeof(server_addr));

        // 4. 데이터 수신 (recvfrom)
        addr_size = sizeof(from_addr);
        str_len = recvfrom(sock, message, BUF_SIZE, 0, 
                           (struct sockaddr*)&from_addr, &addr_size);
        
        message[str_len] = 0;
        printf("Message from Server: %s", message);
    }

    close(sock);
    return 0;
}