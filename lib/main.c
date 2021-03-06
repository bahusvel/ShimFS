#define _GNU_SOURCE

#include "guestfs.h"
#include "hijack.h"
#include "shimfs.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

GuestFS fs_list;

// allocate libc handles
#define X(n) type_##n libc_##n;
OPLIST
#undef X

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

int delete_fd_node(GuestFS *self, int fd) {
	struct fd_node *node;
	struct fd_node *delete = NULL;
	LIST_FOREACH(node, self->fds) {
		if (node->fd == fd)
			delete = node;
	}
	if (delete == NULL)
		return -1;
	LIST_DELETE(node, self->fds);
	free(delete);
	return 0;
}

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
	GuestFS *guest = calloc(sizeof(GuestFS), 1);
	guest->dlhandle = dlopen(path, RTLD_NOW);
	if ((guest->dlhandle) == NULL)
		goto fail_and_free;
	int (*init_func)(GuestFS * self) =
		dlsym(guest->dlhandle, nameof(guestfs_init));
	if (init_func == NULL)
		goto fail_and_free;

// load guestfs fs symbols
#define X(n) guest->ops.n = dlsym(guest->dlhandle, "shim_" #n);
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
// its just gonna be one, located at SHIMFS_FSPATH
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

/* This is for future where syscalls are overriden
static inline long execute_syscall(ucontext_t *context) {
	long output;
	mcontext_t mcontext = context->uc_mcontext;
	__asm__ volatile("syscall" : "=a"(output) : "d"(mcontext.gregs));
	return output;
}
*/

#ifdef __linux__
// LINUX version of GLIBC patching routine
static inline void patch_glibc() {
#define X(n) type_##n orig_##n = symbol_from_lib(RTLD_NEXT, #n);
	OPLIST
#undef X

	Dl_info libc_info;
	lib_for_symbol(orig_open, &libc_info);
	void *handle =
		dlmopen(LM_ID_NEWLM, libc_info.dli_fname, RTLD_NOW | RTLD_LOCAL);
	if (handle == NULL) {
		printf("Failed loading glibc %s\n", dlerror());
		exit(-1);
	}
	if (orig_open == symbol_from_lib(handle, "open")) {
		printf("Your OS doesnt support dlmopen properly, new glibc was not "
			   "loaded\n");
		exit(-1);
	}
	printf("Libc %s loaded successfully\n", libc_info.dli_fname);

#define X(n)                                                                   \
	libc_##n = symbol_from_lib(handle, #n);                                    \
	hijack_start(orig_##n, n);
	OPLIST
#undef X
	safe_printf = symbol_from_lib(handle, "printf");
}

#elif __APPLE__

static inline void patch_glibc() {
#define X(n) libc_##n = symbol_from_lib(RTLD_NEXT, #n);
	OPLIST
#undef X
	printf("Old open %p, new open %p\n", libc_open,
		   symbol_from_lib(RTLD_DEFAULT, "open"));
	/* The only thing that can be done now is to find the end of the current
	 * address space, and map something at the end of it, hoping that it is
	 * close enough to the libc, alternatively one could find libc itself and
	 * look for gaps around it, of the sufficient size required to allocate
	 * trampolines, from there on I can simply mmap it so theoretical algorithm
	 is as follows:
	 1) Locate libc start via lib_for_symbol it should be info.dli_saddr
	 2) Do something similar to what vmmap does on osx
	 3) Locate a gap big enought to fit N trampolines
	 4) mmap(gap_addr, sizeof(n_trampolines))
	 5) set libc_n to corresponding trampoline address
	 */
	safe_printf = symbol_from_lib(RTLD_DEFAULT, "printf");
}

#endif

__attribute__((constructor)) static void shimfs_constructor() {
	// load libc symbols
	patch_glibc();
	// print_assembly(libc_open, 100);
	load_filesystems();
	printf("Successfuly Loaded ShimFS\n\n\n\n");
}

__attribute__((destructor)) static void shimfs_destructor() {}
