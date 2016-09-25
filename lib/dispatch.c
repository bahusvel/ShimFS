#include "shimfs.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define startsWith(str, prefix) strncmp(prefix, str, strlen(prefix)) == 0

int not_implemented_stub(const char *fs_name, char *op_name) {
	safe_printf("Operation %s not implemented for %s\n", op_name, fs_name);
	return 0;
}

#define PATH_WRAPPER(opname, path, ...)                                        \
	GuestFS *fs;                                                               \
	if ((fs = path_filter(path))) {                                            \
		if (fs->ops.opname == NULL) {                                          \
			return not_implemented_stub(fs->name, #opname);                    \
		} else {                                                               \
			return fs->ops.opname(path, ##__VA_ARGS__);                        \
		}                                                                      \
	}                                                                          \
	return libc_##opname(path, ##__VA_ARGS__);

#define FD_WRAPPER(opname, fildes, ...)                                        \
	GuestFS *fs;                                                               \
	if ((fs = fd_filter(fildes))) {                                            \
		if (fs->ops.opname == NULL) {                                          \
			return not_implemented_stub(fs->name, #opname);                    \
		} else {                                                               \
			return fs->ops.opname(fildes, ##__VA_ARGS__);                      \
		}                                                                      \
	}                                                                          \
	return libc_##opname(fildes, ##__VA_ARGS__);

GuestFS *path_filter(const char *path) {
	GuestFS *fs_iter;
	// HACK lookups can be faster than just brute force search, for now its ok
	LIST_FOREACH(fs_iter, fs_list) {
		struct path_node *path_iter;
		LIST_FOREACH(path_iter, fs_iter->paths) {
			if (startsWith(path, path_iter->path)) {
				return fs_iter;
			}
		}
	}
	return NULL;
}

GuestFS *fd_filter(int fildes) {
	GuestFS *fs_iter;
	// HACK lookups can be faster than just brute force search, for now its ok
	LIST_FOREACH(fs_iter, fs_list) {
		struct fd_node *fd_iter;
		LIST_FOREACH(fd_iter, fs_iter->fds) {
			if (fildes == fd_iter->fd) {
				return fs_iter;
			}
		}
	}
	return NULL;
}

/* Intercept functions */
int open(const char *path, int oflag, ...) {
	GuestFS *fs;
	if ((fs = path_filter(path)))
		return fs->ops.open(path, oflag);
	return libc_open(path, oflag);
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
	FD_WRAPPER(read, fildes, buf, nbyte);
}
ssize_t write(int fildes, const void *buf, size_t nbyte) {
	FD_WRAPPER(write, fildes, buf, nbyte);
}
off_t lseek(int fildes, off_t offset, int whence) {
	FD_WRAPPER(lseek, fildes, offset, whence);
}
int close(int fildes) { FD_WRAPPER(close, fildes); }
int creat(const char *path, mode_t mode) { PATH_WRAPPER(creat, path, mode); }
int unlink(const char *path) { PATH_WRAPPER(unlink, path); }
int truncate(const char *path, off_t length) {
	PATH_WRAPPER(truncate, path, length);
}
int ftruncate(int fildes, off_t length) {
	FD_WRAPPER(ftruncate, fildes, length);
}
int rename(const char *old, const char *new); // TODO needs special treatment
int access(const char *path, int amode) { PATH_WRAPPER(access, path, amode); }

// int fstat(int fildes, struct stat *buf) { FD_WRAPPER(fstat, fildes, buf); }

// int stat(const char *path, struct stat *buf) { PATH_WRAPPER(stat, path, buf);
// }
int mkdir(const char *path, mode_t mode) { PATH_WRAPPER(mkdir, path, mode); }
int rmdir(const char *path) { PATH_WRAPPER(rmdir, path); }

/* End Intercept functions */
