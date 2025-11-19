#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>      // O_CREAT, O_RDWR
#include <sys/stat.h>   // mode constants
#include <mqueue.h>     // POSIX MQ 헤더

#define SERVER_Q "/chat_server_q"
#define CLIENT_Q "/chat_client_q"
#define MSG_SIZE 256

int main() {
    mqd_t mq_server, mq_client;
    struct mq_attr attr;
    char buffer[MSG_SIZE];
    unsigned int prio;
    int n;

    // 1. 큐 속성 설정
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_curmsgs = 0;

    // 기존 큐 삭제 (초기화)
    mq_unlink(SERVER_Q);
    mq_unlink(CLIENT_Q);

    // 2. 큐 생성 및 열기
    // 서버 큐: 읽기 전용 (Client -> Server 메시지 수신용)
    mq_server = mq_open(SERVER_Q, O_CREAT | O_RDONLY, 0644, &attr);
    // 클라이언트 큐: 쓰기 전용 (Server -> Client 메시지 송신용)
    mq_client = mq_open(CLIENT_Q, O_CREAT | O_WRONLY, 0644, &attr);

    if (mq_server == (mqd_t)-1 || mq_client == (mqd_t)-1) {
        perror("메시지 큐 오픈 실패");
        exit(1);
    }

    printf("=== [POSIX 채팅 서버] 시작 ===\n");
    printf("클라이언트의 접속 및 메시지를 기다립니다...\n");

    while (1) {
        // 3. 수신 (Client -> Server)
        memset(buffer, 0, MSG_SIZE);
        if (mq_receive(mq_server, buffer, MSG_SIZE, &prio) == -1) {
            perror("수신 실패");
            exit(1);
        }

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("클라이언트가 종료했습니다.\n");
            break;
        }

        printf("[클라이언트]: %s", buffer);

        // 4. 송신 (Server -> Client)
        printf("[서버]: ");
        memset(buffer, 0, MSG_SIZE);
        fgets(buffer, MSG_SIZE, stdin);

        // 'quit' 입력 시 종료 처리
        if (strncmp(buffer, "quit", 4) == 0) {
            mq_send(mq_client, buffer, MSG_SIZE, 1);
            break;
        }

        if (mq_send(mq_client, buffer, MSG_SIZE, 1) == -1) {
            perror("송신 실패");
            exit(1);
        }
    }

    // 5. 종료 및 정리
    mq_close(mq_server);
    mq_close(mq_client);
    mq_unlink(SERVER_Q);
    mq_unlink(CLIENT_Q);
    printf("서버를 종료합니다.\n");

    return 0;
}
