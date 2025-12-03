#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// 쓰레드가 수행할 함수: 인자를 받아서 작업 수행
void *thread_function(void *arg) {
    int task_id = *((int *)arg); // 메인에서 넘겨준 작업 번호를 받음
    pthread_t tid = pthread_self(); // 현재 실행 중인 쓰레드의 고유 ID를 가져옴

    printf("[Task %d] 시작! (System Thread ID: %lu)\n", task_id, (unsigned long)tid);
    
    // 작업을 시뮬레이션 (작업 번호만큼 쉽니다)
    sleep(task_id); 
    
    printf("[Task %d] 작업 완료!\n", task_id);
    
    return NULL; // pthread_exit(NULL)과 동일 효과
}

int main() {
    pthread_t t1, t2;
    int id1 = 1; // 첫 번째 쓰레드에게 줄 작업 번호
    int id2 = 2; // 두 번째 쓰레드에게 줄 작업 번호
    int status;

    printf("--- 메인 프로세스 시작 ---\n");

    // 1. 첫 번째 쓰레드 생성
    // arg 인자에 변수의 주소(&id1)를 넘깁니다.
    if (pthread_create(&t1, NULL, thread_function, (void *)&id1) != 0) {
        perror("Thread 1 생성 실패");
        exit(1);
    }

    // 2. 두 번째 쓰레드 생성
    if (pthread_create(&t2, NULL, thread_function, (void *)&id2) != 0) {
        perror("Thread 2 생성 실패");
        exit(1);
    }

    // 3. 쓰레드 종료 대기 (Join)
    // join을 하지 않으면 메인 함수가 먼저 끝나서 쓰레드가 죽을 수 있습니다.
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("--- 모든 쓰레드 작업 종료 ---\n");

    return 0;
}
