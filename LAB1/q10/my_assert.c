// my_assert.c
#include "my_assert.h"
#include <stdio.h>
#include <stdlib.h>

void __my_assert_fail(const char *expr_str,
                      const char *file,
                      int line,
                      const char *func)
{
    /* glibc assert 메시지 형식에 가깝게 출력 */
    fprintf(stderr,
            "Assertion failed: (%s), function %s, file %s, line %d.\n",
            expr_str, func, file, line);

    /* 표준 assert는 abort()로 SIGABRT 발생 → 코어덤프 가능 */
    abort();
}

