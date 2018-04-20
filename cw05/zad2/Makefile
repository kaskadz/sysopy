CC=gcc
OPTS=-Wall -Wextra

.PHONY: clean test_setup test_slave test_teardown

all: master slave

master: master.o
	$(CC) $^ -o $@ $(OPTS)

master.o: master.c
	$(CC) -c $^ $(OPTS)

slave: slave.o
	$(CC) $^ -o $@ $(OPTS)

slave.o: slave.c
	$(CC) -c $^ $(OPTS)

test_setup:
	@echo Starting master..
	@echo 'Kill it by sending "ala123" to `pipe_one` (echo "ala123" > pipe_one)'
	@./master pipe_one

test_slave:
	@echo Starting slaves in the background to do their job..
	@./slave pipe_one 3 &
	@./slave pipe_one 1 &
	@./slave pipe_one 7 &
	@./slave pipe_one 5 &
	@./slave pipe_one 2 &

test_teardown:
	@echo Sending deadly message to master..
	@echo "ala123" > pipe_one
	@echo Deleting pipe_one
	@rm pipe_one

clean:
	rm -f *.o master slave
