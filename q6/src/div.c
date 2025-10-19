//div.c
#include "cal.h"
#include <stdio.h>

float div(int a, int b) {
	if (b == 0) {
		printf("Error\n");
		return 0;
	}
	return (float)a / b;
}


