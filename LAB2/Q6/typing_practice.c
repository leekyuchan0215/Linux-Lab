#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>    // 터미널 제어 함수
#include <fcntl.h>
#include <time.h>

// 종료를 위한 ESC 키의 ASCII 코드
#define KEY_ESC 27

const char *sentences[] = {
    "System Programming is interesting",
    "Linux file system and process",
    "Practice makes perfect"
};
#define NUM_SENTENCES 3

int main() {
    int fd;
    int nread;
    char ch;
    struct termios init_attr, new_attr;
    
    int total_errors = 0;
    int total_chars = 0;
    int forced_exit = 0; // 강제 종료 플래그
    time_t start_time, end_time;

    // 1. 터미널 속성 변경 (비정규 모드)
    fd = fileno(stdin);
    tcgetattr(fd, &init_attr);
    new_attr = init_attr;
    new_attr.c_lflag &= ~ICANON; 
    new_attr.c_lflag &= ~ECHO;   
    new_attr.c_cc[VMIN] = 1;     
    new_attr.c_cc[VTIME] = 0;    
    
    if (tcsetattr(fd, TCSANOW, &new_attr) != 0) {
        perror("터미널 속성 설정 실패");
        exit(1);
    }

    printf("\n=== 리얼타임 타자 연습 (종료하려면 ESC 누르세요) ===\n\n");

    start_time = time(NULL);

    for (int i = 0; i < NUM_SENTENCES; i++) {
        const char *target = sentences[i];
        int len = strlen(target);
        int cur_idx = 0;
        
        // 줄 맞춤을 위해 공백 사용
        printf("문장 %d: %s\n", i + 1, target);
        printf("입력  : "); // '문장 1'과 길이를 맞추기 위해 공백 추가
        fflush(stdout);

        while (cur_idx < len) {
            nread = read(fd, &ch, 1); 
            
            if (nread > 0) {
                // ESC 키(27)를 누르면 종료 로직 실행
                if (ch == KEY_ESC) {
                    forced_exit = 1;
                    break; 
                }

                if (ch == target[cur_idx]) {
                    write(1, &ch, 1); 
                } else {
                    write(1, "*", 1); 
                    total_errors++;
                }
                cur_idx++;
                total_chars++;
            }
        }
        
        printf("\n\n"); // 문장 간 간격
        
        if (forced_exit) {
            printf("\n>>> ESC가 입력되어 연습을 중단합니다.\n");
            break; // for 반복문 탈출
        }
    }

    end_time = time(NULL);

    // 3. 터미널 속성 원상 복구 (필수!)
    tcsetattr(fd, TCSANOW, &init_attr);

    // 결과 출력 (ESC로 종료했어도 지금까지 한 만큼은 보여줌)
    double duration = difftime(end_time, start_time);
    double cpm = (duration > 0) ? (total_chars / duration) * 60.0 : 0.0;

    printf("=== 결과 ===\n");
    printf("총 입력 글자수: %d\n", total_chars);
    printf("총 오타 횟수 : %d\n", total_errors);
    printf("걸린 시간    : %.1f 초\n", duration);
    printf("평균 타자속도 : %.1f CPM\n", cpm);

    return 0;
}