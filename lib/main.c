#include "guestfs.h"
#include "shimfs.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

GuestFS fs_list;

#define X(n) type_##n libc_##n;
OPLIST
#undef X

static int load_guestfs(const char *path) {
	GuestFS *guest = malloc(sizeof(GuestFS));
	guest->dlhandle = dlopen(path, RTLD_NOW);
	if ((guest->dlhandle) == NULL)
		goto fail_and_free;
	int (*init_func)(GuestFS * self) =
		dlsym(guest->dlhandle, nameof(guestfs_init));
	if (init_func == NULL)
		goto fail_and_free;
	if (init_func(guest) != 0)
		goto fail_and_free;

	LIST_INSERT(guest, fs_list);
	return 0;
fail_and_free:
	free(guest);
	return -1;
}

static void *libc_symbol_for(const char *symbol_name) {
	// locate libc funcions (potentially through libc file)
	void *symbol = dlsym(RTLD_NEXT, symbol_name);
	Dl_info info;
	if (dladdr(symbol, &info) == 0) {
		printf("dladdr failed\n");
		exit(-1);
	}
	printf("%s\n", info.dli_fname);
	return symbol;
}

__attribute__((constructor)) static void shimfs_constructor() {
#define X(n) libc_##n = libc_symbol_for(#n);
	OPLIST
#undef X
}

__attribute__((destructor)) static void shimfs_destructor() {}
