#include "hijack.h"
#include <fcntl.h>
#include <unistd.h>

int not_close(int fildes) {
	printf("I wanna say something\n");
	return 0;
}

int not_close_trampoline(int fildes) { TRAMPOLINE_CONTENTS }

int fake_close(int fildes) {
	printf("jk\n");
	not_close_trampoline(fildes);
	return 0;
}

int main() {
	int fd = open("cmalloc.sh", O_RDONLY);
	printf("Old close %p, new close %p\n", not_close, fake_close);
	// print_assembly(not_close, 100);
	sym_hook hook = hijack_start(not_close, fake_close);
	printf("Hijack size is %d\n", hook.hijack_size);
	if (hijack_make_trampoline(&hook, not_close_trampoline) != 0) {
		printf("Trampoline failed\n");
		exit(-1);
	}
	not_close(fd);
}
