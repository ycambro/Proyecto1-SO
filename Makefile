CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread
OBJS = mypthreads.o scheduler.o prueba.o
TARGET = prueba

all: $(TARGET)

mypthreads.o: mypthreads.c mypthreads.h scheduler.h
	$(CC) $(CFLAGS) -c mypthreads.c

scheduler.o: scheduler.c scheduler.h mypthreads.h
	$(CC) $(CFLAGS) -c scheduler.c

prueba.o: prueba.c mypthreads.h scheduler.h
	$(CC) $(CFLAGS) -c prueba.c

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run