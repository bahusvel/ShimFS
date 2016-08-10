#include "guestfs.h"
#include "hijack.h"
#include "shimfs.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

GuestFS fs_list;

struct path_node *new_path_node(GuestFS *self) {
	struct path_node *node = malloc(sizeof(struct path_node));
	LIST_INSERT(node, self->paths);
	return node;
}

struct fd_node *new_fd_node(GuestFS *self) {
	struct fd_node *node = malloc(sizeof(struct fd_node));
	LIST_INSERT(node, self->fds);
	return node;
}

// allocate libc handles
#define X(n) type_##n libc_##n;
OPLIST
#undef X

static inline void *symbol_from_lib(void *dlhandle, const char *symbol_name) {
	// locate libc funcions (potentially through libc file)
	void *symbol = dlsym(dlhandle, symbol_name);
	if (symbol == NULL) {
		printf("Failed to fetch %s\n", symbol_name);
		exit(-1);
	}
	return symbol;
}

static inline void lib_for_symbol(void *symbol, Dl_info *info) {
	if (dladdr(symbol, info) == 0 || info->dli_fname == NULL) {
		printf("Dladdr failed for %p\n", symbol);
		exit(-1);
	}
}

static int load_guestfs(const char *path) {
	GuestFS *guest = malloc(sizeof(GuestFS));
	guest->dlhandle = dlopen(path, RTLD_NOW);
	if ((guest->dlhandle) == NULL)
		goto fail_and_free;
	int (*init_func)(GuestFS * self) =
		dlsym(guest->dlhandle, nameof(guestfs_init));
	if (init_func == NULL)
		goto fail_and_free;

// load guestfs fs symbols
#define X(n) guest->ops.n = dlsym(guest->dlhandle, #n);
	OPLIST
#undef X

	// init func has the opportunity to overwrite symbols loaded by dlsym
	if (init_func(guest) != 0)
		goto fail_and_free;

	LIST_INSERT(guest, fs_list);
	return 0;
fail_and_free:
	free(guest);
	return -1;
}

// TODO in future this is to load all filesystems in a directory, for now
// its
// just gonna be one, located at SHIMFS_FSPATH
static void load_filesystems() {
	char *fspath = getenv(SHIMFS_FSPATH);
	if (fspath == NULL) {
		printf(SHIMFS_FSPATH " not defined, cannot load file systems\n");
		exit(-1);
	}
	if (load_guestfs(fspath)) {
		printf("Failed to load %s\n", fspath);
		exit(-1);
	}
}

__attribute__((constructor)) static void shimfs_constructor() {
// load libc symbols
#define X(n) libc_##n = symbol_from_lib(RTLD_NEXT, #n);
	OPLIST
#undef X

	/* A second copy of libc is fetched and loaded, functions of the original
	 * libc will be rewritten and hijacked, the second copy will remain
	 * untouched and its functions will be called instead when libc
	 * functionality is wanted*/
	Dl_info libc_info;
	lib_for_symbol(libc_open, &libc_info);
	void *dlhandle = dlopen(libc_info.dli_fname, RTLD_LOCAL);
	if (dlhandle == NULL) {
		printf("Dlopen failed: %s\n", dlerror());
		exit(-1);
	}
	void *new_open = symbol_from_lib(dlhandle, "open");
	Dl_info new_libc_info;
	lib_for_symbol(new_open, &new_libc_info);
	printf("Old open %p-%p, new open %p-%p\n", libc_open, libc_info.dli_fbase,
		   new_open, new_libc_info.dli_fbase);

	// printf("Assembly for open():\n");
	// print_assembly(libc_open, 100);
	load_filesystems();
	printf("Successfuly Loaded ShimFS\n\n\n\n");
}

__attribute__((destructor)) static void shimfs_destructor() {}
