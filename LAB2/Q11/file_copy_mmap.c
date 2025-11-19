/*
 * file_copy_mmap.c
 *
 * 기능: 공유 메모리(System V)와 mmap을 사용하여 파일을 복사하는 프로그램
 * 차별점: 
 * 1. 원본 파일을 메모리에 매핑(mmap)하여 읽기 속도 향상
 * 2. 공유 메모리 내부에 POSIX Unnamed Semaphore를 배치하여 동기화 (파일 I/O 불필요)
 * 3. 부모(Producer) -> 공유메모리 -> 자식(Consumer) 구조
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h>

#define SHM_KEY 9876       // 공유 메모리 키 (임의 설정)
#define BUF_SIZE 4096      // 버퍼 크기

// 공유 메모리 내부에 위치할 구조체
typedef struct {
    sem_t sem_empty;       // 버퍼가 비었는지 확인 (Producer용)
    sem_t sem_full;        // 버퍼가 찼는지 확인 (Consumer용)
    int data_size;         // 현재 버퍼에 담긴 데이터 크기
    int eof;               // 파일 끝 도달 여부
    char buffer[BUF_SIZE]; // 실제 데이터 버퍼
} shm_data_t;

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <원본파일> <대상파일>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *src_file = argv[1];
    const char *dst_file = argv[2];

    // 1. 원본 파일 열기 및 크기 확인
    int src_fd = open(src_file, O_RDONLY);
    if (src_fd == -1) error_exit("원본 파일 열기 실패");

    struct stat sb;
    if (fstat(src_fd, &sb) == -1) error_exit("파일 정보 얻기 실패");
    off_t file_size = sb.st_size;

    // 2. 원본 파일 메모리 매핑 (mmap) - 읽기 성능 최적화
    // 파일 전체를 메모리 주소처럼 접근 가능하게 만듦
    char *src_map = NULL;
    if (file_size > 0) {
        src_map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
        if (src_map == MAP_FAILED) error_exit("원본 파일 mmap 실패");
    }

    // 3. 공유 메모리 생성 (System V IPC)
    int shmid = shmget(SHM_KEY, sizeof(shm_data_t), IPC_CREAT | 0666);
    if (shmid == -1) error_exit("공유 메모리 생성 실패");

    // 4. 공유 메모리 연결 (Attach)
    shm_data_t *shm_ptr = (shm_data_t *)shmat(shmid, NULL, 0);
    if (shm_ptr == (void *)-1) error_exit("공유 메모리 연결 실패");

    // 5. 세마포어 초기화 (공유 메모리 내부 변수 사용)
    // pshared = 1 (프로세스 간 공유), sem_empty = 1 (쓰기 가능), sem_full = 0 (읽기 대기)
    if (sem_init(&shm_ptr->sem_empty, 1, 1) == -1) error_exit("세마포어(empty) 초기화 실패");
    if (sem_init(&shm_ptr->sem_full, 1, 0) == -1) error_exit("세마포어(full) 초기화 실패");
    
    shm_ptr->eof = 0;

    // 6. 프로세스 분기
    pid_t pid = fork();

    if (pid < 0) {
        error_exit("fork 실패");
    } else if (pid > 0) {
        /* --------------------------------------
         * 부모 프로세스 (Producer)
         * 역할: mmap된 원본 데이터를 읽어 공유 메모리에 씀
         * -------------------------------------- */
        close(src_fd); // 부모는 fd 필요 없음 (이미 mmap 함)
        
        off_t offset = 0;
        while (offset < file_size) {
            // 빈 공간이 생길 때까지 대기
            sem_wait(&shm_ptr->sem_empty);

            // 남은 데이터 계산
            int chunk_size = BUF_SIZE;
            if (file_size - offset < BUF_SIZE) {
                chunk_size = file_size - offset;
            }

            // 메모리 복사 (mmap 영역 -> 공유 메모리)
            memcpy(shm_ptr->buffer, src_map + offset, chunk_size);
            shm_ptr->data_size = chunk_size;
            
            offset += chunk_size;

            // 데이터가 찼음을 자식에게 알림
            sem_post(&shm_ptr->sem_full);
        }

        // EOF 처리
        sem_wait(&shm_ptr->sem_empty);
        shm_ptr->eof = 1; // 종료 플래그 설정
        sem_post(&shm_ptr->sem_full);

        // 자원 정리 (부모)
        if (src_map) munmap(src_map, file_size);
        wait(NULL); // 자식 종료 대기

        // 공유 메모리 및 세마포어 제거
        sem_destroy(&shm_ptr->sem_empty);
        sem_destroy(&shm_ptr->sem_full);
        shmdt(shm_ptr);
        shmctl(shmid, IPC_RMID, NULL);
        
        printf("파일 복사 완료 (부모 프로세스 종료)\n");

    } else {
        /* --------------------------------------
         * 자식 프로세스 (Consumer)
         * 역할: 공유 메모리 데이터를 읽어 대상 파일에 씀
         * -------------------------------------- */
        close(src_fd); // 자식은 원본 fd 필요 없음
        if (src_map) munmap(src_map, file_size); // 자식은 매핑 필요 없음

        // 대상 파일 열기 (쓰기 전용, 생성, 내용 삭제)
        int dst_fd = open(dst_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (dst_fd == -1) error_exit("대상 파일 열기 실패");

        while (1) {
            // 데이터가 찰 때까지 대기
            sem_wait(&shm_ptr->sem_full);

            if (shm_ptr->eof) {
                break; // 종료 플래그 확인 시 루프 탈출
            }

            // 파일 쓰기
            int written = write(dst_fd, shm_ptr->buffer, shm_ptr->data_size);
            if (written != shm_ptr->data_size) {
                perror("파일 쓰기 오류");
                break;
            }

            // 빈 공간이 생겼음을 부모에게 알림
            sem_post(&shm_ptr->sem_empty);
        }

        close(dst_fd);
        shmdt(shm_ptr); // 자식은 공유 메모리 연결 해제만 수행
        exit(EXIT_SUCCESS);
    }

    return 0;
}
