#include <stdlib.h>

__attribute__((constructor)) static void shimfs_constructor() {}

__attribute__((destructor)) static void shimfs_destructor() {}
