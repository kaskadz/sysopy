.PHONY: clean test
CC=gcc
RESULTS=raport2.txt
OPTS=-Wall -Wextra
EXEC=main

main: main.c libblockarray.a
	$(CC) $< -o $@ -L . -l blockarray $(OPTS)

libblockarray.so: blockarray.c
	$(CC) -fPIC -c $^ $(OPTS)
	$(CC) -shared -FPIC -o $@ blockarray.o

libblockarray.a: blockarray.c
	$(CC) -c $^
	ar cr $@ blockarray.o

test: main
	./$(EXEC) 1000 100 dynamic arr_create 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 static arr_create 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 dynamic blk_find 500 >> $(RESULTS)
	./$(EXEC) 1000 100 static blk_find 500 >> $(RESULTS)
	./$(EXEC) 1000 100 dynamic blk_add 1000 blk_delete 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 static blk_add 1000 blk_delete 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 dynamic blk_add_and_delete 1000 >> $(RESULTS)
	./$(EXEC) 1000 100 static blk_add_and_delete 1000 >> $(RESULTS)


clean:
	-rm *.o *.so *.a main

