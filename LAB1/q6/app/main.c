//main.c
#include <stdio.h>
#include "cal.h"

int main(void) {
	int a,b;
	printf("두 정수를 입력하세요: ");
	if (scanf("%d %d", &a, &b) != 2) return 1;

	printf("덧셈 결과: %d\n", add(a,b));
	printf("뺄셈 결과: %d\n", sub(a,b));
	printf("곱셈 결과: %d\n", mul(a,b));
	printf("나눗셈 결과: %.2f\n", div(a,b));

	return 0;
}


