#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main() {
	int fd = open("/hello", O_RDONLY);
	if (fd < 0) {
		printf("Open failed\n");
		exit(-1);
	}
	char hellobuf[6];
	if (read(fd, hellobuf, 6) != 6) {
		printf("Read returned less than expected\n");
	}
	printf("%s\n", hellobuf);
	while (1)
		sleep(1);
}
