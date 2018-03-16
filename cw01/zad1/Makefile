CC=gcc
OPTS=-Wall -Wextra

libchararray.so: blockarray.c
	$(CC) -fPIC -c $^ $(OPTS)
	$(CC) -shared -FPIC -o $@ blockarray.o

libchararray.a: blockarray.c
	$(CC) -c $^
	ar cr $@ blockarray.o

.PHONY: clean

clean:
	rm *.o *.so
