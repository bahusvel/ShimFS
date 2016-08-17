#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	int fd = open("/hello", O_RDONLY);
	if (fd < 0) {
		// printf("Open failed\n");
		// exit(-1);
	}
	char hellobuf[6];
	if (read(fd, hellobuf, 6) != 6) {
		// printf("Read returned less than expected\n");
	}
	write(1, hellobuf, 6);
	// printf("%s\n", hellobuf);
}
