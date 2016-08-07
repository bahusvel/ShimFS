#include "guestfs.h"
#include "shimfs.h"
#include <dlfcn.h>
#include <stdlib.h>

GuestFS fs_list;

int load_guestfs(const char *path) {
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

__attribute__((constructor)) static void shimfs_constructor() {}

__attribute__((destructor)) static void shimfs_destructor() {}
