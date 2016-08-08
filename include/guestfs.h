#ifndef __GUESTFS__
#define __GUESTFS__
#include "shimfs.h"
#include <stdlib.h>

#define nameof(symbol) #symbol

// Every GuestFS MUST define these two methods method
extern int guestfs_init(GuestFS *self);
extern int guestfs_fini(GuestFS *self);

// TODO THIS NEEDS TO BE IMPLEMENTED WITHIN SHIMFS
struct path_node *new_path_node(GuestFS *self) {
	struct path_node *node = malloc(sizeof(struct path_node));
	LIST_INSERT(node, self->paths);
	return node;
}

/* NOTE You may NEVER execute standard libc functions on this fd, as it is a
 * dupe or fd of some random file, designed to be used as a dummy, to avoid
 * collusions between in-kernel fs and ShimFS fds.
HACK this dupes stdout, which may not be connected, open could be more reliable
TODO THIS NEEDS TO BE IMPLEMENTED WITHIN SHIMFS*/
int allocate_global_fd() { return dup(1); }

struct fd_node *new_fd_node(GuestFS *self) {
	struct fd_node *node = malloc(sizeof(struct fd_node));
	LIST_INSERT(node, self->fds);
	return node;
}

#endif
