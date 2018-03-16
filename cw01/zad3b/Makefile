.PHONY: all clean test test_dynamic test_shared test_static test_all

CC=gcc
RESULTS=results3b.txt
OLEVEL=0
#OLEVEL=1
#OLEVEL=2
#OLEVEL=3
#OLEVEL=s
OPTS=-Wall -Wextra -O$(OLEVEL)

all: static shared dynamic 

static: main.c libblockarray.a
	$(CC) $^ -o $@ 

shared: main.c libblockarray.so
	$(CC) $< -o $@ -L . -l blockarray

dynamic: main.c libblockarray.so
	$(CC) $< -o $@ -ldl -DDLL

libblockarray.a: blockarray.c
	$(CC) -c $^
	$(AR) cr $@ blockarray.o

libblockarray.so: blockarray.c
	$(CC) -fPIC -c $^ $(OPTS)
	$(CC) -shared -FPIC -o $@ blockarray.o

test_all:
	$(MAKE) test_static
	$(MAKE) test_shared
	$(MAKE) test_dynamic

test_dynamic: EXEC=dynamic
test_dynamic: dynamic test

test_shared: EXEC=shared
test_shared: shared test

test_static: EXEC=static
test_static: static test

test:
	echo Library: $(EXEC) >> $(RESULTS)
	echo O-level: $(OLEVEL) >> $(RESULTS)
	./$(EXEC) 1000 100 dynamic arr_create 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 static arr_create 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 dynamic blk_add 1000 blk_delete 1000 blk_add_and_delete 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 static blk_add 1000 blk_delete 1000 blk_add_and_delete 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 dynamic blk_find 500 >> $(RESULTS)
	./$(EXEC) 1000 100 static blk_find 500 >> $(RESULTS)

clean:
	-rm *.o *.so *.a static dynamic shared

