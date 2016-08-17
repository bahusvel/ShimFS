.PHONY: clean

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
	LIBNAME=libShimFS.so
	DISTORM_PATH=distorm/make/linux/libdistorm3.so
	LIB_PATH= LD_LIBRARY_PATH=./
endif
ifeq ($(UNAME_S),Darwin)
	LIBNAME=libShimFS.dylib
	DISTORM_PATH=distorm/make/mac/libdistorm3.dylib
	LIB_PATH= DYLD_LIBRARY_PATH=./
endif


CFLAGS= -W -fPIC -Wall -Wextra -O -g -std=c99 -Iinclude -Idistorm/include


all: clean test

test: simple_test

run:
	./cmalloc.sh python3

clean:
	rm -f *.o *.so core

dispatch.o:
	gcc -c $(CFLAGS) lib/dispatch.c -o dispatch.o

main.o:
	gcc -c $(CFLAGS) lib/main.c -o main.o

libdistorm:
	cp $(DISTORM_PATH) ./

libShimFS: main.o dispatch.o libdistorm
	gcc -o $(LIBNAME) main.o dispatch.o -L. -ldl -ldistorm3 -shared

hellofs:
	make -C example/hellofs

simple_test: libShimFS hellofs
	gcc -c $(CFLAGS) test/helloworld.c -o helloworld.o
	gcc -o simple_test helloworld.o -L. -ldistorm3 -lShimFS
	$(LIB_PATH) SHIMFS_FSPATH=example/hellofs/libHelloFS.so ./simple_test
