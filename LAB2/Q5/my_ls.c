#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

void my_ls_recursive(const char *base_path) {
    DIR *dir_ptr;
    struct dirent *entry;
    struct stat statbuf;
    char path[1024];

    // 1. 디렉토리 열기
    if ((dir_ptr = opendir(base_path)) == NULL) {
        // 접근 권한 등으로 열지 못하는 경우 에러 메시지 없이 리턴 (선배 출력처럼 깔끔하게 하기 위해)
        return; 
    }

    // 2. 현재 디렉토리 이름 출력
    printf("\n%s:\n", base_path);

    // 3. [Phase 1] 현재 디렉토리 내의 파일 목록 출력
    while ((entry = readdir(dir_ptr)) != NULL) {
        // ★ 핵심 수정 부분 ★
        // 이름의 첫 글자가 '.'이면 숨김 파일이므로 건너뜁니다. (. 과 .. 도 포함됨)
        if (entry->d_name[0] == '.')
            continue;

        printf("%s  ", entry->d_name);
    }
    printf("\n");

    // 4. 디렉토리 포인터 초기화
    rewinddir(dir_ptr);

    // 5. [Phase 2] 하위 디렉토리 재귀 탐색
    while ((entry = readdir(dir_ptr)) != NULL) {
        // ★ 핵심 수정 부분 ★
        // 마찬가지로 숨김 파일/디렉토리는 재귀 탐색에서도 제외합니다.
        if (entry->d_name[0] == '.')
            continue;

        // 전체 경로 생성
        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        if (lstat(path, &statbuf) == -1) {
            continue;
        }

        // 디렉토리인 경우 재귀 호출
        if (S_ISDIR(statbuf.st_mode)) {
            my_ls_recursive(path);
        }
    }

    // 6. 디렉토리 닫기
    closedir(dir_ptr);
}

int main(int argc, char *argv[]) {
    const char *target_dir;

    if (argc > 1) {
        target_dir = argv[1];
    } else {
        target_dir = ".";
    }

    my_ls_recursive(target_dir);

    return 0;
}