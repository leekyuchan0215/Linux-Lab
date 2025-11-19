#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// 시그널 핸들러 함수
void my_sig_handler(int signo) {
    if (signo == SIGUSR1) {
        printf("\t[핸들러] SIGUSR1 신호를 받았습니다!\n");
    } else if (signo == SIGALRM) {
        printf("\t[핸들러] 알람! 시간이 초과되었습니다.\n");
    }
}

int main() {
    pid_t pid;
    struct sigaction act;
    int status;

    // 1. sigaction 구조체 설정 (강의자료 47-48p 참고) 
    // signal() 함수보다 더 안전하고 정교한 sigaction() 사용
    act.sa_handler = my_sig_handler;
    sigemptyset(&act.sa_mask); // 시그널 집합 비우기 [cite: 7653]
    act.sa_flags = 0;

    // SIGUSR1과 SIGALRM에 대한 핸들러 등록
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGALRM, &act, NULL);

    printf("=== 프로세스 및 시그널 통신 예제 시작 ===\n");

    // 2. 프로세스 생성 (fork) [cite: 6862]
    pid = fork();

    if (pid < 0) {
        perror("fork 실패");
        exit(1);
    }
    else if (pid == 0) {
        // [자식 프로세스]
        printf("2. [자식] 생성됨 (PID: %d). 부모의 신호를 기다립니다.\n", getpid());
        
        // 3. 알람 설정 (5초 안에 신호가 안 오면 알람 발생) 
        alarm(5); 

        // 4. 시그널 대기 (pause) - 부모가 신호를 줄 때까지 잠듦 
        pause(); 
        
        // 부모로부터 SIGUSR1을 받고 핸들러가 실행된 후 여기로 복귀
        printf("3. [자식] 부모 신호 확인 완료. 알람을 끕니다.\n");
        alarm(0); // 알람 해제 [cite: 7893]

        printf("4. [자식] 부모에게 답장 신호(SIGUSR1)를 보냅니다.\n");
        sleep(1); // 전송 전 잠시 대기
        
        // 5. 부모에게 시그널 전송 (kill) 
        kill(getppid(), SIGUSR1);
        
        printf("5. [자식] 종료합니다.\n");
        exit(0);
    }
    else {
        // [부모 프로세스]
        printf("1. [부모] 자식 프로세스(PID: %d)를 생성했습니다.\n", pid);
        
        // 자식이 준비될 시간을 줌
        sleep(1); 

        printf("6. [부모] 자식에게 신호(SIGUSR1)를 보냅니다.\n");
        // 자식에게 시그널 전송 
        kill(pid, SIGUSR1);

        printf("7. [부모] 자식의 응답을 기다립니다 (pause)...\n");
        // 자식의 응답 대기 
        pause(); 

        printf("8. [부모] 자식의 응답을 확인했습니다.\n");

        // 자식 프로세스 종료 대기 (wait) [cite: 6994]
        wait(&status);
        printf("9. [부모] 자식 프로세스가 정상적으로 종료되었습니다.\n");
    }

    return 0;
}
