CC = gcc
CFLAGS = -Wall -g
OBJS = mypthread.o scheduler.o test.o

all: test

mypthread.o: mypthread.c mypthread.h scheduler.h
	$(CC) $(CFLAGS) -c mypthread.c

scheduler.o: scheduler.c scheduler.h mypthread.h
	$(CC) $(CFLAGS) -c scheduler.c

test.o: test.c mypthread.h scheduler.h
	$(CC) $(CFLAGS) -c test.c

test: $(OBJS)
	$(CC) $(CFLAGS) -o test $(OBJS)

clean:
	rm -f *.o test