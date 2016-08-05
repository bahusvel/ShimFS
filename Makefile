.PHONY: clean

all: clean cryptomalloc test

test:
	gcc -std=c99 CryptoMallocTest/main.c -o test

clean:
	rm -f *.o *.so monitor test malloc_rewrite

libsegments: aes.o
	gcc -W -fPIC -Wall -Wextra -O2 -g -std=c99 -c -I./CryptoMalloc/ CryptoSegments/segments.c
	gcc $(LDFLAGS) -o CryptoSegments.so segments.o aes.o -lrt -lpthread -lelf

cryptomalloc: aes.o
	gcc $(CFLAGS) -c CryptoMalloc/main.c
	gcc $(LDFLAGS) -o CryptoMalloc.so main.o aes.o -lrt

<<<<<<< HEAD
segment_test:
	gcc -std=c99 -I./CryptoSegments/ CryptoMallocTest/segment_test.c -o segment_test
	./segment_test

segments_run: clean libsegments test
	LD_PRELOAD=./CryptoSegments.so python2
=======
monitor:
	gcc CryptoMalloc/monitor.c -o monitor

run_monitor: clean monitor
	./monitor

malloc_hook: clean
	gcc -I distorm/include Experiments/malloc_rewrite.c -o malloc_rewrite -ldistorm3 -ldl
	./malloc_rewrite
>>>>>>> malloc_rewrite

run:
	./cmalloc.sh python3
