.PHONY: clean

CFLAGS= -W -fPIC -Wall -Wextra -O -g -std=c99 -Iinclude

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

libShimFS: main.o dispatch.o
	gcc -o libShimFS.dylib main.o dispatch.o -ldl -shared

hellofs:
	make -C example/hellofs

simple_test: libShimFS hellofs
	gcc -c $(CFLAGS) test/helloworld.c -o helloworld.o
	gcc -o simple_test helloworld.o -L. -lShimFS
	SHIMFS_FSPATH=example/hellofs/libHelloFS.so ./simple_test

segments_run: clean libsegments test
	LD_PRELOAD=./CryptoSegments.so python2

malloc_hook: clean
	gcc -I distorm/include Experiments/malloc_rewrite.c -o malloc_rewrite -ldistorm3 -ldl
	./malloc_rewrite
