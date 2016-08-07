#ifndef __GUESTFS__
#define __GUESTFS__
#include "shimfs.h"

#define nameof(symbol) #symbol

// Every GuestFS MUST define these two methods method
int guestfs_init(GuestFS *self);
int guestfs_fini(GuestFS *self);

#endif
