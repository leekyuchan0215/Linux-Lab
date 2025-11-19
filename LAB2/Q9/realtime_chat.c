#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#define BUF_SIZE 1024

// 색상 코드 정의
#define COLOR_RED     "\x1b[31m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_RESET   "\x1b[0m"

void chat_session(int read_pipe, int write_pipe, const char *name, const char *color) {
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    fd_set reads, temps;
    int max_fd;
    int result;

    FD_ZERO(&reads);
    FD_SET(0, &reads);         // 키보드
    FD_SET(read_pipe, &reads); // 상대방 메시지

    max_fd = (read_pipe > 0) ? read_pipe : 0;

    // 안내 문구도 색깔을 입혀서 출력
    printf("%s[%s] 채팅 시작! (종료: quit)%s\n", color, name, COLOR_RESET);
    
    while (1) {
        temps = reads;
        result = select(max_fd + 1, &temps, NULL, NULL, NULL);

        if (result == -1) {
            perror("select 실패");
            exit(1);
        }

        // 1. 내가 키보드를 입력했을 때
        if (FD_ISSET(0, &temps)) {
            memset(send_buf, 0, BUF_SIZE);
            if (fgets(send_buf, BUF_SIZE, stdin) == NULL) break;

            if (strncmp(send_buf, "quit", 4) == 0) {
                write(write_pipe, "quit", 5);
                break;
            }

            // 파이프로 전송
            write(write_pipe, send_buf, strlen(send_buf));
            
        }

        // 2. 상대방 메시지가 왔을 때
        if (FD_ISSET(read_pipe, &temps)) {
            memset(recv_buf, 0, BUF_SIZE);
            int str_len = read(read_pipe, recv_buf, BUF_SIZE);
            
            if (str_len <= 0 || strncmp(recv_buf, "quit", 4) == 0) {
                printf("\n%s상대방이 나갔습니다.%s\n", color, COLOR_RESET);
                break;
            }

            // 상대방 메시지 출력 (색상 적용)
            // \r은 커서를 맨 앞으로 돌려서, 입력 중인 내 글자가 밀리지 않게 보이게 하는 꼼수입니다.
            printf("\r%s[상대방]: %s%s", color, recv_buf, COLOR_RESET);
        }
    }
}

int main() {
    int p1[2], p2[2];
    pid_t pid;

    if (pipe(p1) == -1 || pipe(p2) == -1) {
        perror("Pipe Error");
        exit(1);
    }

    pid = fork();

    if (pid < 0) {
        perror("Fork Error");
        exit(1);
    }
    else if (pid == 0) { 
        // [자식 프로세스 - RED]
        close(p1[1]); close(p2[0]);
        chat_session(p1[0], p2[1], "Child", COLOR_RED);
        close(p1[0]); close(p2[1]);
    }
    else {
        // [부모 프로세스 - BLUE]
        close(p1[0]); close(p2[1]);
        chat_session(p2[0], p1[1], "Parent", COLOR_BLUE);
        close(p1[1]); close(p2[0]);
    }
    return 0;
}