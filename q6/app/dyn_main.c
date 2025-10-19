// dyn_main.c : 동적 로딩을 이용한 계산기 예제
#include <stdio.h>
#include <dlfcn.h>

int main(void) {
    int a, b;
    printf("두 정수를 입력하세요: ");
    if (scanf("%d %d", &a, &b) != 2) return 1;

    // 동적 라이브러리 열기
    void *lib = dlopen("./lib/liboperations.so", RTLD_LAZY);
    if (!lib) {
        fprintf(stderr, "라이브러리 로드 실패: %s\n", dlerror());
        return 1;
    }

    // 함수 포인터 가져오기
    int   (*add)(int,int) = dlsym(lib, "add");
    int   (*sub)(int,int) = dlsym(lib, "sub");
    int   (*mul)(int,int) = dlsym(lib, "mul");
    float (*div)(int,int) = dlsym(lib, "div");

    char *err = dlerror();
    if (err) {
        fprintf(stderr, "심볼 로딩 실패: %s\n", err);
        dlclose(lib);
        return 1;
    }

    // 출력 형식을 main.c와 동일하게
    printf("덧셈 결과 : %d\n", add(a, b));
    printf("뺄셈 결과 : %d\n", sub(a, b));
    printf("곱셈 결과 : %d\n", mul(a, b));
    printf("나눗셈 결과 : %.2f\n", div(a, b));

    dlclose(lib);
    return 0;
}

