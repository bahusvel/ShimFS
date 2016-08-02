# CryptoMalloc


# How does it work?


# Why?
* For fun!
* To avoid using a kernel module
* Process private file systems
* Faster than FUSE even faster than native FS (in certain use cases)

# How fast is it?

# How to use it?
If you have the source code for the software whose ram you want encrypted, simply link the CryptoMalloc like any other standard shared library and compile your code, and it should work as is. If you don't however you can use the following tricks to load it into your binary runtime:

```bash
# Mac OS X, It is not developed for OSX, but in theory it should work on any POSIX (may need minor modifications)
DYLD_FORCE_FLAT_NAMESPACE=1 DYLD_INSERT_LIBRARIES=libCryptoMalloc.dylib [application]
# Linux, Totally works :)
LD_PRELOAD=cryptomalloc.so [application]
# OR
./cmalloc.sh [application]
```
CryptoMalloc also provides a shell script that will do this for you! (cmalloc.sh)

