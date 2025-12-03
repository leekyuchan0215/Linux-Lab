#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9001
#define BUF_SIZE 1024

int main() {
    int server_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    char buffer[BUF_SIZE];
    int str_len;

    // 1. 소켓 생성 (SOCK_DGRAM: UDP)
    server_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (server_sock == -1) {
        perror("socket error");
        exit(1);
    }

    // 2. 주소 설정 및 Bind
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind error");
        exit(1);
    }

    printf("--- UDP Server Started (Port: %d) ---\n", PORT);

    // 3. 데이터 송수신 (UDP는 listen/accept 과정이 없음)
    while (1) {
        client_addr_size = sizeof(client_addr);
        // recvfrom: 데이터를 받으면서 보낸 사람의 주소(client_addr)도 같이 받음
        str_len = recvfrom(server_sock, buffer, BUF_SIZE, 0, 
                           (struct sockaddr*)&client_addr, &client_addr_size);
        
        buffer[str_len] = 0;
        printf("Received from Client: %s", buffer);

        // sendto: 받은 주소로 그대로 다시 전송
        sendto(server_sock, buffer, str_len, 0, 
               (struct sockaddr*)&client_addr, client_addr_size);
    }

    close(server_sock);
    return 0;
}