/* 5장 디버깅과 오류 처리
 * 파일 이름: assert_test.c (MY_ASSERT 버전)
 */
#include <stdio.h>
#include <stdlib.h>
#include "my_assert.h"

void foo(int num)
{
    MY_ASSERT( (num >= 0) && (num <= 100) );
    printf("foo: num = %d\n", num);
}

int main(int argc, char *argv[])
{
    int num;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s aNumber\n(0 <= aNumber <= 100)\n", argv[0]);
        return 1;
    }

    num = atoi(argv[1]);
    foo(num);
    return 0;
}

