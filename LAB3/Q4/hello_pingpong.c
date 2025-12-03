#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define LOOP_COUNT 5

// 문제의 핵심: 공유 자원 구조체
typedef struct {
    pthread_mutex_t mutex; // 상호 배제
    pthread_cond_t cond;   // 동기화 (순서 제어)
    int flag;              // 이진 플래그 (0: 부모 차례, 1: 자식 차례)
} SharedData;

SharedData shared;

// 자식 쓰레드 함수
void *child_func(void *arg) {
    for (int i = 0; i < LOOP_COUNT; i++) {
        pthread_mutex_lock(&shared.mutex);

        // 자식 차례(flag == 1)가 아니면 대기
        // [조건변수 사용]
        while (shared.flag != 1) {
            pthread_cond_wait(&shared.cond, &shared.mutex);
        }

        // 작업 수행
        printf("hello child\n");
        
        // 이진 플래그 변경 (자식 -> 부모)
        shared.flag = 0;

        // 대기 중인 부모 쓰레드 깨우기
        pthread_cond_signal(&shared.cond);

        pthread_mutex_unlock(&shared.mutex);
    }
    return NULL;
}

int main() {
    pthread_t tid;

    // 초기화
    pthread_mutex_init(&shared.mutex, NULL);
    pthread_cond_init(&shared.cond, NULL);
    shared.flag = 0; // 부모가 먼저 시작하도록 설정 (0: 부모)

    // 자식 쓰레드 생성
    if (pthread_create(&tid, NULL, child_func, NULL) != 0) {
        perror("pthread_create error");
        exit(1);
    }

    // 부모 쓰레드 작업
    for (int i = 0; i < LOOP_COUNT; i++) {
        pthread_mutex_lock(&shared.mutex);

        // 부모 차례(flag == 0)가 아니면 대기
        while (shared.flag != 0) {
            pthread_cond_wait(&shared.cond, &shared.mutex);
        }

        // 작업 수행
        printf("hello parent\n");

        // 이진 플래그 변경 (부모 -> 자식)
        shared.flag = 1;

        // 대기 중인 자식 쓰레드 깨우기
        pthread_cond_signal(&shared.cond);

        pthread_mutex_unlock(&shared.mutex);
    }

    // 자식 쓰레드 종료 대기
    pthread_join(tid, NULL);

    // 자원 해제
    pthread_mutex_destroy(&shared.mutex);
    pthread_cond_destroy(&shared.cond);

    return 0;
}