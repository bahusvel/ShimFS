#ifndef __SHIMFS__
#define __SHIMFS__

#include <unistd.h>

/* Currently supported ops, refer to link below for explanation of the macro
  https://natecraun.net/articles/struct-iteration-through-abuse-of-the-c-preprocessor.html
 */
#define OPLIST                                                                 \
	X(open)                                                                    \
	X(read)                                                                    \
	X(write)                                                                   \
	X(lseek)                                                                   \
	X(close)

/* Libc functions */
typedef int (*type_open)(const char *path, int oflag, ...);
typedef ssize_t (*type_read)(int fildes, void *buf, size_t nbyte);
typedef ssize_t (*type_write)(int fildes, const void *buf, size_t nbyte);
typedef off_t (*type_lseek)(int fildes, off_t offset, int whence);
typedef int (*type_close)(int fildes);

#define X(n) extern type_##n libc_##n;
OPLIST
#undef X
/* end libc funcs */

struct shimfs_ops {
#define X(n) type_##n n;
	OPLIST
#undef X
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
