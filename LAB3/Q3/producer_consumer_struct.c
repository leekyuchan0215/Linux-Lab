#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define MAX_PRODUCERS 2
#define MAX_CONSUMERS 2

// 관련 변수를 구조체 하나로 묶음
typedef struct {
    int item[BUFFER_SIZE]; // 실제 데이터가 저장될 버퍼
    int count;             // 현재 버퍼에 찬 개수
    int in;                // 데이터를 넣을 인덱스
    int out;               // 데이터를 뺄 인덱스
    pthread_mutex_t mutex; // 상호 배제를 위한 뮤텍스
    pthread_cond_t full;   // 버퍼가 꽉 찼음을 알리는 조건변수 (생산자 대기용)
    pthread_cond_t empty;  // 버퍼가 비었음을 알리는 조건변수 (소비자 대기용)
} BoundedBuffer;

// 전역 변수로 버퍼 구조체 선언
BoundedBuffer bb; 

// 생산자 쓰레드 함수
void *producer(void *arg) {
    int id = *((int *)arg);
    
    while (1) {
        int data = rand() % 100; // 0~99 사이 랜덤 데이터 생성
        
        // 임계 영역 진입 (Lock)
        pthread_mutex_lock(&bb.mutex);

        // 버퍼가 꽉 찼으면, 자리가 날 때까지 대기 (Wait)
        // 강의자료 38p: while문을 써야 깨어난 후 다시 조건을 검사함
        while (bb.count == BUFFER_SIZE) {
            printf("[생산자 %d] 버퍼가 가득 찼습니다. 대기 중...\n", id);
            pthread_cond_wait(&bb.full, &bb.mutex);
        }

        // 데이터 생산 (Write)
        bb.item[bb.in] = data;
        bb.in = (bb.in + 1) % BUFFER_SIZE;
        bb.count++;
        printf("[생산자 %d] 데이터 %d 생산 (현재 개수: %d)\n", id, data, bb.count);

        // 소비자가 가져갈 수 있도록 '비어있지 않음' 신호 전송 (Signal)
        pthread_cond_signal(&bb.empty);
        
        // 임계 영역 탈출 (Unlock)
        pthread_mutex_unlock(&bb.mutex);

        // 작업 속도 조절 (잠시 대기)
        usleep(rand() % 1000000); 
    }
    return NULL;
}

// 소비자 쓰레드 함수
void *consumer(void *arg) {
    int id = *((int *)arg);
    
    while (1) {
        // 임계 영역 진입 (Lock)
        pthread_mutex_lock(&bb.mutex);

        // 버퍼가 비었으면, 데이터가 찰 때까지 대기 (Wait)
        while (bb.count == 0) {
            printf("    [소비자 %d] 버퍼가 비었습니다. 대기 중...\n", id);
            pthread_cond_wait(&bb.empty, &bb.mutex);
        }

        // 데이터 소비 (Read)
        int data = bb.item[bb.out];
        bb.out = (bb.out + 1) % BUFFER_SIZE;
        bb.count--;
        printf("    [소비자 %d] 데이터 %d 소비 (현재 개수: %d)\n", id, data, bb.count);

        // 생산자가 데이터를 넣을 수 있도록 '가득 차지 않음' 신호 전송 (Signal)
        pthread_cond_signal(&bb.full);

        // 임계 영역 탈출 (Unlock)
        pthread_mutex_unlock(&bb.mutex);

        // 소비 속도 조절 (생산보다 약간 느리게 설정하여 버퍼가 차는 상황 연출)
        usleep(rand() % 1500000);
    }
    return NULL;
}

int main() {
    pthread_t p_threads[MAX_PRODUCERS];
    pthread_t c_threads[MAX_CONSUMERS];
    int p_ids[MAX_PRODUCERS];
    int c_ids[MAX_CONSUMERS];

    // 1. 구조체 내부 동기화 도구 초기화
    bb.count = 0;
    bb.in = 0;
    bb.out = 0;
    pthread_mutex_init(&bb.mutex, NULL);
    pthread_cond_init(&bb.full, NULL);
    pthread_cond_init(&bb.empty, NULL);

    printf("--- 생산자-소비자 시뮬레이션 시작 ---\n");

    // 2. 생산자 쓰레드 생성
    for (int i = 0; i < MAX_PRODUCERS; i++) {
        p_ids[i] = i + 1;
        pthread_create(&p_threads[i], NULL, producer, (void *)&p_ids[i]);
    }

    // 3. 소비자 쓰레드 생성
    for (int i = 0; i < MAX_CONSUMERS; i++) {
        c_ids[i] = i + 1;
        pthread_create(&c_threads[i], NULL, consumer, (void *)&c_ids[i]);
    }

    // 4. 종료 대기 (무한 루프라 도달하지 않음)
    for (int i = 0; i < MAX_PRODUCERS; i++) {
        pthread_join(p_threads[i], NULL);
    }
    for (int i = 0; i < MAX_CONSUMERS; i++) {
        pthread_join(c_threads[i], NULL);
    }

    // 자원 해제
    pthread_mutex_destroy(&bb.mutex);
    pthread_cond_destroy(&bb.full);
    pthread_cond_destroy(&bb.empty);

    return 0;
}
