#ifndef __PATH_TO_DES__
#define __PATH_TO_DES__
#include <sys/stat.h>
#include <sys/types.h>

// TODO can I avoid dispatch for open and close by calling the shim directly,
// the f vesrion of the function can go through dispatch, but it would be nice
// if it could also be avoided

int shim_stat(const char *path, struct stat *buf) {
	int fd = open(path);
	if (fd < 0) {
		return -1;
	}
	int ret = fstat(fd, buf);
	close(fd);
	return ret;
}
int shim_truncate(const char *path, off_t length) {
	int fd = open(path);
	if (fd < 0) {
		return -1;
	}
	int ret = ftruncate(fd, length);
	close(fd);
	return ret;
}

#endif
