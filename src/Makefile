CC = gcc
CFLAGS = -Wall -g
OBJS = main.o procesos.o config.o log.o
EXEC = MatComGuard

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

main.o: main.c procesos.h config.h log.h
	$(CC) $(CFLAGS) -c main.c

procesos.o: procesos.c procesos.h config.h log.h
	$(CC) $(CFLAGS) -c procesos.c

config.o: config.c config.h
	$(CC) $(CFLAGS) -c config.c

log.o: log.c log.h
	$(CC) $(CFLAGS) -c log.c

clean:
	rm -f *.o $(EXEC)
