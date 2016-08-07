#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	printf("Hello World\n");
	write(1, "Hello\n", 6);
}
