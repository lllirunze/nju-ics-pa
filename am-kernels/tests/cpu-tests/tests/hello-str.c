#include "trap.h"

char buf[128];

int main() {

	sprintf(buf, "%s", "Hello world!\n");
	check(strcmp(buf, "Hello world!\n") == 0);

	sprintf(buf, "%d + %d = %d\n", -1, 1, 0);
	check(strcmp(buf, "-1 + 1 = 0\n") == 0);

	sprintf(buf, "%d + %d = %d\n", -2, -125, -127);
	check(strcmp(buf, "-2 + -125 = -127\n") == 0);

	printf(buf, "%d + %d = %d\n", -2, -125, -127);

	return 0;
}
