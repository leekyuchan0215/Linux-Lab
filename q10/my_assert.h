// my_assert.h
#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#ifdef NDEBUG
  #define MY_ASSERT(expr) ((void)0)
#else
  void __my_assert_fail(const char *expr_str,
                        const char *file,
                        int line,
                        const char *func);

  #define MY_ASSERT(expr) \
    ((expr) ? (void)0 : __my_assert_fail(#expr, __FILE__, __LINE__, __func__))
#endif

#endif // MY_ASSERT_H

