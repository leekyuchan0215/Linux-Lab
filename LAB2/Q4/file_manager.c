#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // open, close, read, write, lseek, getcwd, chdir
#include <fcntl.h>      // O_RDWR, O_CREAT, O_TRUNC 등
#include <sys/types.h>
#include <sys/stat.h>   // mkdir, stat
#include <dirent.h>     // opendir, readdir

#define DIR_NAME "my_logs"
#define FILE_NAME "system.log"
#define NEW_FILE_NAME "system_backup.log"

void error_handling(const char *message) {
    perror(message);
    exit(1);
}

int main() {
    int fd;
    char buf[1024];
    char cwd[1024];

    printf("=== [Q4. 파일/디렉토리 심화 실습 시작] ===\n\n");

    // 1. 현재 작업 디렉토리 확인 (getcwd)
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("1. 현재 작업 경로: %s\n", cwd);
    }

    // 2. 새 디렉토리 생성 (mkdir)
    // 권한 0755: 소유자(rwx), 그룹(rx), 기타(rx)
    if (mkdir(DIR_NAME, 0755) == -1) {
        printf("   (알림: '%s' 디렉토리가 이미 존재하거나 생성 실패)\n", DIR_NAME);
    } else {
        printf("2. '%s' 디렉토리 생성 완료\n", DIR_NAME);
    }

    // 3. 작업 디렉토리 변경 (chdir)
    if (chdir(DIR_NAME) == -1) {
        error_handling("디렉토리 이동 실패");
    }
    printf("3. '%s' 디렉토리로 이동 완료\n", DIR_NAME);

    // 4. 파일 생성 및 내용 쓰기 (open, write)
    fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) error_handling("파일 생성 실패");

    const char *msg1 = "[INFO] System started.\n";
    const char *msg2 = "[WARN] Low memory.\n";
    const char *msg3 = "[INFO] Task completed.\n";

    write(fd, msg1, strlen(msg1));
    write(fd, msg2, strlen(msg2));
    write(fd, msg3, strlen(msg3));
    printf("4. 로그 파일('%s') 생성 및 데이터 기록 완료\n", FILE_NAME);

    // 5. 파일 오프셋 이동 (lseek)
    printf("5. lseek를 사용하여 파일 포인터 조작\n");
    
    // SEEK_SET: 파일의 시작점 기준으로 0만큼 이동 (처음으로 되감기)
    lseek(fd, 0, SEEK_SET); 
    
    int read_cnt = read(fd, buf, sizeof(buf) - 1);
    if (read_cnt > 0) {
        buf[read_cnt] = '\0';
        printf("   [파일 전체 내용 읽기]:\n%s", buf);
    }

    // SEEK_END를 사용하여 파일 크기 구하기
    off_t file_size = lseek(fd, 0, SEEK_END);
    printf("   -> 파일 크기 확인 (lseek 활용): %ld bytes\n", file_size);

    close(fd); // 파일 닫기

    // 6. 파일 이름 변경 (rename)
    if (rename(FILE_NAME, NEW_FILE_NAME) == 0) {
        printf("6. 파일명 변경 완료: %s -> %s\n", FILE_NAME, NEW_FILE_NAME);
    } else {
        perror("파일명 변경 실패");
    }

    // 7. 디렉토리 내부 검사 (opendir, readdir)
    printf("7. 현재 디렉토리('%s') 파일 목록 스캔:\n", DIR_NAME);
    DIR *dp = opendir("."); // 현재 디렉토리('.') 열기
    struct dirent *entry;
    struct stat statbuf;

    if (dp == NULL) error_handling("디렉토리 열기 실패");

    while ((entry = readdir(dp)) != NULL) {
        // stat 함수로 파일의 상세 정보 얻기
        stat(entry->d_name, &statbuf);

        // 파일 타입 확인 매크로 사용 (S_ISDIR, S_ISREG)
        if (S_ISDIR(statbuf.st_mode))
            printf("   [DIR]  %s\n", entry->d_name);
        else if (S_ISREG(statbuf.st_mode))
            printf("   [FILE] %s (Size: %ld bytes)\n", entry->d_name, statbuf.st_size);
        else
            printf("   [ETC]  %s\n", entry->d_name);
    }
    closedir(dp);

    printf("\n=== 프로그램 종료 ===\n");
    return 0;
}
