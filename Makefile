CC = gcc
CFLAGS = -Wall -g
OBJS = mypthread.o scheduler.o scheduler_test.o

all: scheduler_test

mypthread.o: mypthread.c mypthread.h
	$(CC) $(CFLAGS) -c mypthread.c

scheduler.o: scheduler.c mypthread.h
	$(CC) $(CFLAGS) -c scheduler.c

scheduler_test.o: scheduler_test.c mypthread.h
	$(CC) $(CFLAGS) -c scheduler_test.c

scheduler_test: $(OBJS)
	$(CC) $(CFLAGS) -o scheduler_test $(OBJS)

clean:
	rm -f *.o scheduler_test