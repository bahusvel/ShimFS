# ShimFS
Make file systems completely in user space without cooperation from the kernel.

# How does it work?
ShimFS gets injected into your client process and executes file system code in the process itself (you dont have to implement file system there, this could simple be a client for a global FS server using your own custom API). ShimFS will in turn process app file system manipulations and call your file system APIs where you do whatever your filesystem needs to do.

# Why?
* For fun!
* To avoid using a kernel module
* Process private file systems
* Faster than FUSE even faster than native FS (in certain use cases)

# How fast is it?
Pretty fast, as it avoids context switching for custom file system completely (whatever context switching your file system does, i.e. networking, block device access is your problem not ShimFS problem). Additionally there is no locking or syncrhonization unless you actually need it.
Tests will follow...

# How to use it?
If you have the source code for the software that wants to use your custome file system, simply link the ShimFS like any other standard shared library and compile your code, and it should work as is. If you don't however you can use the following tricks to load it into your binary runtime:

```bash
# Mac OS X, It is not developed for OSX, but in theory it should work on any POSIX (may need minor modifications)
DYLD_FORCE_FLAT_NAMESPACE=1 DYLD_INSERT_LIBRARIES=libCryptoMalloc.dylib [application]
# Linux, Totally works :)
LD_PRELOAD=cryptomalloc.so [application]
# OR
./shimfs.sh [application]
```
ShimFS also provides a shell script that will do this for you! (shimfs.sh)

