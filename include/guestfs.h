#ifndef __GUESTFS__
#define __GUESTFS__
#include "shimfs.h"
#include <stdlib.h>

#define nameof(symbol) #symbol

// Every GuestFS MUST define these two methods method
int guestfs_init(GuestFS *self);
int guestfs_fini(GuestFS *self);

// these are defined within ShimFS
struct path_node *new_path_node(GuestFS *self);
struct fd_node *new_fd_node(GuestFS *self);

/* You may NEVER execute standard libc functions on this fd, as it is designed
to be used as a dummy, to avoid collisions between in-kernel fs and ShimFS fds.
HACK this dupes stdout, which may not be connected, open could be more reliable
*/
int allocate_global_fd() { return dup(1); }

#endif
