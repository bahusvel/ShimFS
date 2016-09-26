#ifndef __SHIMFS__
#define __SHIMFS__

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SHIMFS_FSPATH "SHIMFS_FSPATH"

/* Currently supported ops, refer to link below for explanation of the macro
  https://natecraun.net/articles/struct-iteration-through-abuse-of-the-c-preprocessor.html
 */
#define OPLIST                                                                 \
	X(open)                                                                    \
	X(read)                                                                    \
	X(write)                                                                   \
	X(lseek)                                                                   \
	X(close)                                                                   \
	X(creat)                                                                   \
	X(unlink)                                                                  \
	X(truncate)                                                                \
	X(ftruncate)                                                               \
	X(rename)                                                                  \
	X(access)                                                                  \
	X(mkdir)                                                                   \
	X(rmdir)

typedef int (*type_open)(const char *path, int oflag, ...);
typedef ssize_t (*type_read)(int fildes, void *buf, size_t nbyte);
typedef ssize_t (*type_write)(int fildes, const void *buf, size_t nbyte);
typedef off_t (*type_lseek)(int fildes, off_t offset, int whence);
typedef int (*type_close)(int fildes);
typedef int (*type_creat)(const char *path, mode_t mode);
typedef int (*type_unlink)(const char *path);
typedef int (*type_truncate)(const char *path, off_t length);
typedef int (*type_ftruncate)(int fildes, off_t length);
typedef int (*type_rename)(const char *old, const char *new);
typedef int (*type_access)(const char *path, int amode);
typedef int (*type_fstat)(int fildes, struct stat *buf);
typedef int (*type_stat)(const char *path, struct stat *buf);
typedef int (*type_mkdir)(const char *path, mode_t mode);
typedef int (*type_rmdir)(const char *path);
typedef struct dirent *(*type_readdir)(DIR *dirp);

int (*safe_printf)(const char *format, ...);

// declare libc functions
#define X(n) type_##n libc_##n;
OPLIST
#undef X

/*  TODO it might be useful to provide some way for guestfs to communicate to
 * shimfs in response to requests (i.e.) tell shimfs to use libc, or something
 * else */
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

#define LIST_DELETE(item_ptr, list)                                            \
	if (list.next == item_ptr)                                                 \
		list.next = list.next->next;                                           \
	__typeof__(item_ptr) iter_ptr;                                             \
	for (iter_ptr = list.next; iter_ptr->next != NULL;                         \
		 iter_ptr = iter_ptr->next) {                                          \
		if (iter_ptr->next == item_ptr)                                        \
			iter_ptr->next = iter_ptr->next->next;                             \
	}

struct path_node {
	const char *path;
	struct path_node *next;
};

struct fd_node {
	int fd;
	struct fd_node *next;
};

typedef struct GuestFS {
	const char *name;
	void *dlhandle;
	// think of paths as a mount point, in normal cases you should only have a
	// few here, if you want to do something like overlayFS just specify "/" and
	// handle the rest yourself
	struct path_node paths;
	// this is where you should store every open file fd, lookups for these will
	// be made efficient.
	struct fd_node fds;
	struct shimfs_ops ops;
	struct GuestFS *next;
} GuestFS;

extern GuestFS fs_list;

#endif
