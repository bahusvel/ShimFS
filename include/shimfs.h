#ifndef __SHIMFS__
#define __SHIMFS__

#include <unistd.h>

/* Libc functions */
int (*libc_open)(const char *path, int oflag, ...);
ssize_t (*libc_read)(int fildes, void *buf, size_t nbyte);
ssize_t (*libc_write)(int fildes, const void *buf, size_t nbyte);
off_t (*libc_lseek)(int fildes, off_t offset, int whence);
int (*libc_close)(int fildes);
/* end libc funcs */

struct shimfs_ops {
	int (*open)(const char *path, int oflag, ...);
	int (*read)(int fildes, void *buf, size_t nbyte);
	int (*write)(int fildes, const void *buf, size_t nbyte);
	off_t (*lseek)(int fildes, off_t offset, int whence);
	int (*close)(int fildes);
};

#define LIST_INSERT(item_ptr, list)                                            \
	item_ptr->next = list.next;                                                \
	list.next = item_ptr

#define LIST_FOREACH(iter_ptr, list)                                           \
	for (iter_ptr = list.next; iter_ptr != NULL; iter_ptr = iter_ptr->next)

struct path_node {
	const char *path;
	struct path_node *next;
};

struct fd_node {
	const int fd;
	struct fd_node *next;
};

typedef struct GuestFS {
	const char *name;
	void *dlhandle;
	struct path_node paths;
	struct fd_node fds;
	struct shimfs_ops ops;
	struct GuestFS *next;
} GuestFS;

extern GuestFS fs_list;

#endif
