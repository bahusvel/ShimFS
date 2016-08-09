.PHONY: clean

CFLAGS= -W -fPIC -Wall -Wextra -O -g -std=c99 -Iinclude -Idistorm/include
LIB_PATH= DYLD_LIBRARY_PATH=./

all: clean test

test: simple_test

run:
	./cmalloc.sh python3

clean:
	rm -f *.o *.so

dispatch.o:
	gcc -c $(CFLAGS) lib/dispatch.c -o dispatch.o

main.o:
	gcc -c $(CFLAGS) lib/main.c -o main.o

libdistorm:
	cp distorm/make/mac/libdistorm3.dylib ./

libShimFS: main.o dispatch.o libdistorm
	gcc -o libShimFS.dylib main.o dispatch.o -L. -ldl -ldistorm3 -shared

hellofs:
	make -C example/hellofs

simple_test: libShimFS hellofs
	gcc -c $(CFLAGS) test/helloworld.c -o helloworld.o
	gcc -o simple_test helloworld.o -L. -lShimFS
	$(LIB_PATH) SHIMFS_FSPATH=example/hellofs/libHelloFS.so ./simple_test
