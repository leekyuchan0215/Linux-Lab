#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>      // O_CREAT, O_RDWR 등
#include <sys/stat.h>   // mode constants
#include <mqueue.h>     // POSIX 메시지 큐 헤더

#define QUEUE_NAME "/lab_mq_q8"
#define MAX_SIZE 1024
#define MSG_STOP "exit"

int main() {
    pid_t pid;
    mqd_t mq;
    struct mq_attr attr;
    char buffer[MAX_SIZE + 1];
    int must_stop = 0;

    // 1. 메시지 큐 속성 설정 (강의자료 73p 참고)
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;      // 최대 메시지 개수
    attr.mq_msgsize = MAX_SIZE; // 최대 메시지 크기
    attr.mq_curmsgs = 0;

    // 혹시 이전에 찌꺼기가 남아있을 수 있으니 삭제 후 시작
    mq_unlink(QUEUE_NAME);

    // 2. 메시지 큐 생성 및 열기 (강의자료 72p mq_open)
    // O_CREAT: 없으면 생성, O_RDWR: 읽기/쓰기 모드
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("메시지 큐 오픈 실패");
        exit(1);
    }

    printf("=== POSIX 메시지 큐 통신 시작 (이름: %s) ===\n\n", QUEUE_NAME);

    pid = fork();

    if (pid < 0) {
        perror("fork 실패");
        exit(1);
    }
    else if (pid == 0) { 
        // [자식 프로세스] - 메시지를 받아서 출력하고 답장 보냄
        printf("2. [자식] 준비 완료. 부모의 메시지를 기다립니다.\n");

        // 우선순위를 저장할 변수
        unsigned int prio;
        
        // 메시지 수신 대기 (강의자료 72p mq_receive)
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_SIZE, &prio);
        
        if (bytes_read >= 0) {
            buffer[bytes_read] = '\0';
            printf("3. [자식] 수신된 메시지: %s (우선순위: %d)\n", buffer, prio);
        }

        // 부모에게 답장 전송 (우선순위 2로 설정하여 중요도 높임)
        const char *reply = "네, 메시지 잘 받았습니다! (From Child)";
        printf("4. [자식] 답장 전송 중...\n");
        mq_send(mq, reply, strlen(reply) + 1, 2); 
        
        // 자식용 큐 닫기
        mq_close(mq);
        exit(0);
    }
    else {
        // [부모 프로세스] - 먼저 메시지를 보내고 답장을 기다림
        const char *msg = "안녕? 나는 부모 프로세스야. (From Parent)";
        printf("1. [부모] 메시지 전송: %s\n", msg);
        
        // 메시지 전송 (우선순위 1) (강의자료 72p mq_send)
        if (mq_send(mq, msg, strlen(msg) + 1, 1) == -1) {
            perror("전송 실패");
        }

        // 자식이 처리할 시간을 주기 위해 잠시 대기 (동기화)
        sleep(1);
        
        // 자식의 답장 수신
        unsigned int prio;
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_SIZE, &prio);
        
        if (bytes_read >= 0) {
            buffer[bytes_read] = '\0';
            printf("5. [부모] 자식의 답장: %s (우선순위: %d)\n", buffer, prio);
        }

        // 자식 종료 대기
        wait(NULL);
        
        // 3. 정리 작업 (강의자료 72p mq_close, mq_unlink)
        mq_close(mq);
        mq_unlink(QUEUE_NAME); // 큐 삭제 (중요!)
        printf("6. [부모] 통신 종료 및 큐 삭제 완료.\n");
    }

    return 0;
}
