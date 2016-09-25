#include "guestfs.h"
#include <stdio.h>
#include <string.h>

GuestFS *self_global = NULL;
int fpos = 0;

int open(const char *path, int oflag, ...) {
	if (strcmp(path, "/hello") != 0) {
		return 0;
	}
	fpos = 0;
	struct fd_node *node = new_fd_node(self_global);
	node->fd = allocate_global_fd();
	return node->fd;
}

int close(int fildes) {
	delete_fd_node(self_global, fildes);
	return 0;
}

/*
ssize_t read(int fildes, void *buf, size_t nbyte) {
	if (fpos == 6) {
		return EOF;
	}
	int toread = nbyte < 6 ? nbyte : 6;
	memcpy(buf, "world\n", toread);
	fpos += toread;
	return toread;
}
*/
int guestfs_init(GuestFS *self) {
	self_global = self;
	self->name = "hellofs";
	/* Optional, ShimFS will automatically load symbols that are named
	correctly, but this can be used as an overide
	self->ops.open = open;
	self->ops.read = read;
	*/
	struct path_node *node = new_path_node(self);
	node->path = "/hello";
	return 0;
}
