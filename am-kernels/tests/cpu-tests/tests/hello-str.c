#include "trap.h"

char buf[128];
int ret;

int main() {

	// char *str = "1234";
	ret = printf("size: 1234\n");
	printf("size: %d\n", ret);

	ret = sprintf(buf, "%s", "Hello world!\n");
	printf("size: %d\n", ret);
	check(strcmp(buf, "Hello world!\n") == 0);

	sprintf(buf, "%d + %d = %d\n", -1, 1, 0);
	check(strcmp(buf, "-1 + 1 = 0\n") == 0);

	sprintf(buf, "%d + %d = %d\n", -2, -125, -127);
	check(strcmp(buf, "-2 + -125 = -127\n") == 0);

	printf("%8d + %08d = %08d\n", -2, -125, -127);
	

	return 0;
}
