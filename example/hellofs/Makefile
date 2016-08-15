SHIMFS_DIR=-L../../

all:clean hellofs

clean:
	rm -rf *.so

hellofs:
	gcc -I../../include $(SHIMFS_DIR) -fPIC -shared hellofs.c -o libHelloFS.so -lShimFS
