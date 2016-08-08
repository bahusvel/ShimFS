#include "guestfs.h"
#include <string.h>

GuestFS *self_global = NULL;

int open(const char *path, int oflag, ...) {
	if (strcmp(path, "/hello") != 0) {
		return 0;
	}
	struct fd_node *node = new_fd_node(self_global);
	node->fd = allocate_global_fd();
	return node->fd;
}

ssize_t read(int fildes, void *buf, size_t nbyte) {
	memcpy(buf, "world", nbyte < 6 ? nbyte : 6);
	return nbyte < 6 ? nbyte : 6;
}

int guestfs_init(GuestFS *self) {
	self_global = self;
	/* Optional, ShimFS will automatically load symbols that are named
	correctly, but this can be used as an overide
	self->ops.open = open;
	self->ops.read = read;
	*/
	struct path_node *node = new_path_node(self);
	node->path = "/hello";
	return 0;
}
