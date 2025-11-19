#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define SERVER_Q "/chat_server_q"
#define CLIENT_Q "/chat_client_q"
#define MSG_SIZE 256

int main() {
    mqd_t mq_server, mq_client;
    char buffer[MSG_SIZE];
    unsigned int prio;

    printf("=== [POSIX 채팅 클라이언트] ===\n");
    printf("서버에 연결을 시도합니다...\n");

    // 1. 큐 열기 (서버가 이미 생성했다고 가정)
    // 서버 큐: 쓰기 전용 (내 메시지를 보냄)
    while ((mq_server = mq_open(SERVER_Q, O_WRONLY)) == (mqd_t)-1) {
        // 서버가 켤 때까지 잠시 대기
        printf("서버 대기 중...\n");
        sleep(1);
    }
    // 클라이언트 큐: 읽기 전용 (서버 메시지를 받음)
    while ((mq_client = mq_open(CLIENT_Q, O_RDONLY)) == (mqd_t)-1) {
        sleep(1);
    }

    printf("연결되었습니다! (종료하려면 'quit' 입력)\n");

    while (1) {
        // 2. 송신 (Client -> Server)
        printf("[클라이언트]: ");
        memset(buffer, 0, MSG_SIZE);
        fgets(buffer, MSG_SIZE, stdin);

        if (mq_send(mq_server, buffer, MSG_SIZE, 1) == -1) {
            perror("송신 실패");
            break;
        }

        if (strncmp(buffer, "quit", 4) == 0) {
            break;
        }

        // 3. 수신 (Server -> Client)
        printf("(서버 응답 대기 중...)\n");
        memset(buffer, 0, MSG_SIZE);
        if (mq_receive(mq_client, buffer, MSG_SIZE, &prio) == -1) {
            perror("수신 실패");
            break;
        }

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("서버가 종료했습니다.\n");
            break;
        }

        printf("[서버]: %s", buffer);
    }

    // 4. 정리
    mq_close(mq_server);
    mq_close(mq_client);
    printf("클라이언트를 종료합니다.\n");

    return 0;
}
