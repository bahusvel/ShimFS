#include "shimfs.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define startsWith(str, prefix) strncmp(prefix, str, strlen(prefix)) == 0

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
	GuestFS *fs;
	if ((fs = fd_filter(fildes)))
		return fs->ops.read(fildes, buf, nbyte);
	return libc_read(fildes, buf, nbyte);
}

ssize_t write(int fildes, const void *buf, size_t nbyte) {
	printf("Hi\n");
	GuestFS *fs;
	if ((fs = fd_filter(fildes)))
		return fs->ops.write(fildes, buf, nbyte);
	return libc_write(fildes, buf, nbyte);
}

off_t lseek(int fildes, off_t offset, int whence) {
	GuestFS *fs;
	if ((fs = fd_filter(fildes)))
		return fs->ops.lseek(fildes, offset, whence);
	return libc_lseek(fildes, offset, whence);
}

int close(int fildes) {
	GuestFS *fs;
	if ((fs = fd_filter(fildes)))
		return fs->ops.close(fildes);
	return libc_close(fildes);
}
/* End Intercept functions */
